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

#include "net-api/rdma/infiniband/IBConnector.h"
#include "net-api/rdma/infiniband/IBNetEngine.h"

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr IBConnector::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.rdma.infiniband.IBConnector"));

//////////////////////////////////////////////////////////////////////////
IBConnector::IBConnector(IBNetEngine* engine) : mEngine(engine)
{
	IB_DEBUG("CTOR");
	mStatus = IDLE;
}

IBConnector::~IBConnector()
{
	IB_DEBUG("DTOR");
	if(mStatus == CONNECTING)
	{
		BOOST_ASSERT(false);
	}
	cleanup();
	IB_DEBUG("DTOR_COMPLETE");
}

//////////////////////////////////////////////////////////////////////////
bool IBConnector::connect(SharedPtr<Poller> poller, SharedPtr<InetSocketAddress> address, ConnectorCallback callback)
{
	int err = 0;

	mConnectorCallback = callback;

	// create connector rdma_event_channel
	mConnectInfo.rchannel = IBFactory::createEventChannel();

	// create connector cm_id
	mConnectInfo.id = IBFactory::createManagementId(mConnectInfo.rchannel.get(), NULL, RDMA_PS_TCP);;

	// save poller reference
	mConnectInfo.poller = poller;

	// create rdma_event_channel watcher
	mConnectInfo.watcher.set(mConnectInfo.rchannel->fd, ev::READ);
	mConnectInfo.watcher.set<IBConnector, &IBConnector::handleChannelEvent>(this);
	mConnectInfo.poller->start(&mConnectInfo.watcher);

	// create timeout watcher
	mConnectInfo.timeout.set(0.0, 0.5);
	mConnectInfo.timeout.set<IBConnector, &IBConnector::handleTimeoutEvent>(this);
	mConnectInfo.poller->start(&mConnectInfo.timeout);

	// trying to resolve address
	// start asynchronous rdma_resolve_addr
	IB_DEBUG("trying to resolve Infiniband address");
	err = rdma_resolve_addr(mConnectInfo.id.get(), NULL, address->getSocketAddress(), IB_DEFAULT_ADDR_RESOLVE_TIMEOUT);
	if(err)
	{
		IB_ERROR("failed to resolve address, err = " << err);
		cleanup();
		return false;
	}

	mStatus = CONNECTING;

	return true;
}

void IBConnector::cancel()
{
	// TODO here we should really cancel the connecting process
	if(mStatus == CONNECTING)
		mStatus = CANCELED;
}

void IBConnector::handleChannelEvent(ev::io& w, int revent)
{
	int err = 0;
	rdma_cm_event* event = NULL;

	// get rmda_cm_event
	err = rdma_get_cm_event(mConnectInfo.rchannel.get(), &event);
	if(err)
	{
		IB_DEBUG("fail to get cm event!");
		return;
	}

	IB_DEBUG("capture cm event = " << rdma_event_str(event->event));

	switch(event->event)
	{
		case RDMA_CM_EVENT_ADDR_RESOLVED:
		{
			// acknowledge rmda_cm_event
			err = rdma_ack_cm_event(event);

			// try to resolve RDMA route
			err = rdma_resolve_route(mConnectInfo.id.get(), IB_DEFAULT_ROUTE_RESOLVE_TIMEOUT);
			if(err)
			{
				IB_DEBUG("failed to resolve route, err = " << err);
				mConnectorCallback(SharedPtr<IBConnection>(), err);
			}

			break;
		}
		case RDMA_CM_EVENT_ROUTE_RESOLVED:
		{
			// acknowledge rmda_cm_event
			err = rdma_ack_cm_event(event);

			// create new rdma_event_channel
			//SharedPtr<rdma_event_channel> rchannel = IBFactory::createEventChannel();

			// create the IBConnection object
			SharedPtr<IBConnection> connection = IBConnection::create(mEngine, mConnectInfo.id/*, rchannel*/);

			// start poller
			connection->start(mConnectInfo.poller);

			// add the connection object to NetEngine
			mEngine->addConnection(connection);

			// try to establish RDMA connection
			IB_DEBUG("trying to connect to remote through Infiniband");
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
			err = rdma_connect(mConnectInfo.id.get(), &conn_param);
			if(err)
			{
				IB_DEBUG("failed to make rdma connection, err = " << err);
				mConnectorCallback(SharedPtr<IBConnection>(), err);
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

			mStatus = CONNECTED;

			// complete connecting process, remove itself
			SharedPtr<IBConnector> holder(mWeakThis);
			mEngine->connectorCompleted(holder);

			mConnectorCallback(connection, 0);

			break;
		}
		case RDMA_CM_EVENT_ROUTE_ERROR:
		{
			IB_ERROR("capture event " << rdma_event_str(event->event) << ", maybe the route has some problem (the link is down)");
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

			mStatus = ERROR;

			// complete connecting process, remove itself
			SharedPtr<IBConnector> holder(mWeakThis);
			mEngine->connectorCompleted(holder);

			break;
		}
		default:
		{
			IB_ERROR("capture miscellaneous event " << rdma_event_str(event->event));

			err = rdma_ack_cm_event(event);

			// complete connecting process, remove itself
			SharedPtr<IBConnector> holder(mWeakThis);
			mEngine->connectorCompleted(holder);

			mStatus = ERROR;

			break;
		}
	}
}

void IBConnector::handleTimeoutEvent(ev::timer& w, int revent)
{
	mConnectInfo.timeout.stop();

	if(mStatus == CANCELING)
	{
		cleanup();
	}
}

void IBConnector::cleanup()
{
	mConnectInfo.watcher.stop();
	mConnectInfo.timeout.stop();

	mConnectInfo.id.reset();
	mConnectInfo.rchannel.reset();

	mConnectInfo.poller.reset();
}

} } }
