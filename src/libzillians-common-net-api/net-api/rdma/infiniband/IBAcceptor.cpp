/**
 * Zillians MMO
 * Copyright (C) 2007-2009 Zillians.com, Inc.
 * For more information see http://www.zillians.com
 *
 * Zillians MMO is the library and runtime for massive multiplayer online game
 * development in utility computing model, which runs as a service for every
 * developer to build their virtual world running on our GPU-assisted machines.
 *
 * This is a close source library intended to be used solely within Zillians.com
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "net-api/rdma/infiniband/IBAcceptor.h"
#include "net-api/rdma/infiniband/IBNetEngine.h"

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr IBAcceptor::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.rdma.infiniband.IBAcceptor"));

//////////////////////////////////////////////////////////////////////////
IBAcceptor::IBAcceptor(IBNetEngine* engine) : mEngine(engine)
{
	IB_DEBUG("CTOR");
	mStatus = IDLE;
}

IBAcceptor::~IBAcceptor()
{
	IB_DEBUG("DTOR");
	cleanup();
	IB_DEBUG("DTOR_COMPLETE");
}

//////////////////////////////////////////////////////////////////////////
bool IBAcceptor::accept(SharedPtr<Poller> poller, SharedPtr<InetSocketAddress> address, AcceptorCallback callback)
{
	int err = 0;

	if(mStatus != IDLE)
		return false;

	mAcceptorCallback = callback;

	// create listener rdma_event_channel
	mAcceptInfo.rchannel = IBFactory::createEventChannel();

	// create listener cm_id
	mAcceptInfo.listen_id = IBFactory::createManagementId(mAcceptInfo.rchannel.get(), NULL, RDMA_PS_TCP);;

	// save poller reference
	mAcceptInfo.poller = poller;

	// create rdma_event_channel watcher
	mAcceptInfo.watcher.set(mAcceptInfo.rchannel->fd, ev::READ);
	mAcceptInfo.watcher.set<IBAcceptor, &IBAcceptor::handleChannelEvent>(this);
	mAcceptInfo.poller->start(&mAcceptInfo.watcher);

	// create timeout watcher
	mAcceptInfo.timeout.set(0.0, 0.5);
	mAcceptInfo.timeout.set<IBAcceptor, &IBAcceptor::handleTimeoutEvent>(this);
	mAcceptInfo.poller->start(&mAcceptInfo.timeout);

	// bind on the local address
	IB_DEBUG("trying to bind on listening address");
	err = rdma_bind_addr(mAcceptInfo.listen_id.get(), address->getSocketAddress());
	if(err)
	{
		IB_ERROR("fail to bind listening address, error = " << err);
		cleanup();
		return false;
	}

	// start listening on the address
	IB_DEBUG("trying to listen on binded address");
	err = rdma_listen(mAcceptInfo.listen_id.get(), 1);
	if(err)
	{
		IB_ERROR("fail to listen on binded address, error = " << err);
		cleanup();
		return false;
	}

	mStatus = ACCEPTING;

	return true;
}

void IBAcceptor::cancel()
{
	if(mStatus == ACCEPTING)
		mStatus = CANCELING;
}

void IBAcceptor::handleChannelEvent(ev::io& w, int revent)
{
	int err = 0;
	rdma_cm_event* event = NULL;

	// get rmda_cm_event
	err = rdma_get_cm_event(mAcceptInfo.rchannel.get(), &event);
	if(err)
	{
		IB_ERROR("fail to get cm event!");
		mAcceptorCallback(SharedPtr<IBConnection>(), err);
		return;
	}
	IB_DEBUG("capture cm event = " << rdma_event_str(event->event));

	switch(event->event)
	{
		case RDMA_CM_EVENT_CONNECT_REQUEST:
		{
			IB_DEBUG("connection requested, cm_id = " << event->id);

			// save the rdma_cm_id (management id)
			SharedPtr<rdma_cm_id> id(event->id, IBFactory::destroyManagementId);

			// acknowledge rmda_cm_event
			err = rdma_ack_cm_event(event);

			// create new rdma_event_channel
			//SharedPtr<rdma_event_channel> rchannel = IBFactory::createEventChannel();

			// create the IBConnection object
			SharedPtr<IBConnection> connection = IBConnection::create(mEngine, id/*, rchannel*/);

			// start poller
			connection->start(mAcceptInfo.poller);

			// add the connection object to NetEngine
			mEngine->addConnection(connection);

			// try to accept connection
			IB_DEBUG("trying to accept from remote through Infiniband");
			const rdma_conn_param DEFAULT_CONNECT_PARAM =
			{
			    0,    // .private_data
			    0,    // .private_data_len
			    4,    // .responder_resources
			    4,    // .initiator_depth
			    0,    // .flow_control
			    5,    // .retry_count
			    7     // .rnr_retry_count
			};
			rdma_conn_param conn_param = DEFAULT_CONNECT_PARAM;
			err = rdma_accept(id.get(), &conn_param);
			if(err)
			{
				IB_ERROR("fail to accept connection, err = " << err);
				mAcceptorCallback(SharedPtr<IBConnection>(), err);
			}

			break;
		}
		case RDMA_CM_EVENT_ESTABLISHED:
		{
			IB_DEBUG("connection established, cm_id = " << event->id);

			// save the rdma_cm_id (management id)
			rdma_cm_id* id = event->id;

			// acknowledge rmda_cm_event
			err = rdma_ack_cm_event(event);

			// get connection object
			SharedPtr<IBConnection> connection = mEngine->getConnection(id);

			// migrate the cm_id to new rdma_event_channel
			rdma_migrate_id(connection->mVerbs.id.get(), connection->mVerbs.rchannel.get());

			// force to exchange remote access key
			connection->forceAccessExchange();

			// notify upper user
			mEngine->getDispatcher()->dispatchConnectionEvent(IBDispatcher::CONNECTED, connection);

			mAcceptorCallback(connection, 0);

			break;
		}
		case RDMA_CM_EVENT_REJECTED:
		case RDMA_CM_EVENT_DISCONNECTED:
		{
			// save the rdma_cm_id (management id)
			rdma_cm_id* id = event->id;

			// acknowledge rmda_cm_event
			err = rdma_ack_cm_event(event);

			// get connection object
			SharedPtr<IBConnection> connection = mEngine->getConnection(id);

			// the connection is disconnected before connection establish, don't notify upper user
			mEngine->removeConnection(connection);

			// set the connected to false to prevent it from calling rdma_disconnect inside IBConnection::close(), which may result in corrupted heap
			connection->mConnected = false;

			break;
		}
		/*
		case RDMA_CM_EVENT_ADDR_CHANGE:
		{
			// TODO this event happens when original port is down, if it's listening on a bond device, then re-bind the same address might work
			// TODO re-bind and re-listen the same address
			break;
		}
		*/
		default:
		{
			IB_ERROR("capture miscellous event " << rdma_event_str(event->event));

			err = rdma_ack_cm_event(event);

			SharedPtr<IBAcceptor> holder(mWeakThis);
			mEngine->acceptorCompleted(holder);

			mStatus = ERROR;

			mAcceptorCallback(SharedPtr<IBConnection>(), err);

			break;
		}
	}
}

void IBAcceptor::handleTimeoutEvent(ev::timer& w, int revent)
{
	mAcceptInfo.timeout.again();

	if(mStatus == CANCELING)
	{
		cleanup();
	}
}

void IBAcceptor::cleanup()
{
	mAcceptInfo.watcher.stop();
	mAcceptInfo.timeout.stop();

	mAcceptInfo.listen_id.reset();
	mAcceptInfo.rchannel.reset();

	mAcceptInfo.poller.reset();

	mStatus = CANCELED;
}

} } }
