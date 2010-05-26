/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
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
/**
 * @date Mar 19, 2010 sdk - Initial version created.
 */

#include "messaging/MessagingEngine.h"

namespace zillians { namespace messaging {

//////////////////////////////////////////////////////////////////////////
bool MessagingEngine::Remote::Remote()
{ }

bool MessagingEngine::Remote::write(shared_ptr<Buffer> buffer)
{
	bool result = true;

	// make sure a single write does not overflow the remote buffer size
	BOOST_ASSERT(buffer->dataSize() <= bufferSize);

	volatile std::size_t* rptr = rposCache->rptr();
	std::size_t rposCurrent = *rptr;

	volatile std::size_t* wptr = wposCache->rptr();
	std::size_t wposCurrent = *wptr;

	// check if there's still space available for RDMA write
	while(wposCurrent - rposCurrent < buffer->size())
	{
		// try to update the rposCache through RDMA read
		connection->read(rposCache, rposKey, 0, sizeof(std::size_t));
		rposCurrent = *rptr;
	}

	// write to the remote buffer through RDMA write (may need to be splited into two RDMA write operation in case of crossing buffer boundary)
	std::size_t rpos; rposCache->getDirect<std::size_t>(rpos, 0);
	std::size_t wpos; wposCache->getDirect<std::size_t>(wpos, 0);

	result = connection->writeDirect(buffer, this->bufferKey, wpos);

	if(!result) return false;

	// update the remote rpos through RDMA write
	wpos += buffer->dataSize();

	wposCache->setDirect<std::size_t>(wpos, 0);

	result = connection->writeDirect(wposCache, wposKey, 0);
}

bool MessagingEngine::Remote::sendControl(int32 type, shared_ptr<Buffer> buffer)
{
	return connection->send(type, buffer);
}

//////////////////////////////////////////////////////////////////////////
std::size_t MessagingEngine::Local::Local()
{ }

std::size_t MessagingEngine::Local::read(shared_ptr<Buffer> buffer)
{
	volatile uint64* wptr = wpos->rptr();
	uint64 wposCurrent = *wptr;

	volatile uint64* rptr = rpos->rptr();
	uint64 rposCurrent = *rptr;

	BOOST_ASSERT(rposCurrent >= wposCurrent);

	if(rposCurrent == wposCurrent)
	{
		return 0;
	}
	else
	{
		wposCurrent = wposCurrent % buffer->allocatedSize();
		rposCurrent = rposCurrent % buffer->allocatedSize();

		if(wposCurrent < rposCurrent)
		{

		}
	}
}

std::size_t MessagingEngine::Local::peek(shared_ptr<Buffer> buffer)
{

}

bool MessagingEngine::Local::skip(std::size_t size)
{

}

bool MessagingEngine::Local::checkAvailable()
{

}

void MessagingEngine::Local::getAvailableBufferRanges(std::vector<std::pair<void*, std::size_t> >& ranges)
{

}

//////////////////////////////////////////////////////////////////////////
MessagingEngine::MessagingEngine(const std::string& localEndPoint)
{
	// create dispatcher
	mDispatcher = shared_ptr<IBDispatcher>(new IBDispatcher());

	// register all internal message types
	{
		shared_ptr<IBDataHandlerBinder> dataHandler(
				new IBDataHandlerBinder(
						boost::bind(
								MessagingEngine::handleQueueInfoExchangeRequest, this,
								IBDataHandlerBinder::placeholders::connection,
								IBDataHandlerBinder::placeholders::type,
								IBDataHandlerBinder::placeholders::buffer)));

		mDispatcher->registerDataHandler(QueueInfoExchangeMsg::Request::TYPE, dataHandler);
	}

	{
		shared_ptr<IBDataHandlerBinder> dataHandler(
				new IBDataHandlerBinder(
						boost::bind(
								MessagingEngine::handleQueueInfoExchangeResponse, this,
								IBDataHandlerBinder::placeholders::connection,
								IBDataHandlerBinder::placeholders::type,
								IBDataHandlerBinder::placeholders::buffer)));

		mDispatcher->registerDataHandler(QueueInfoExchangeMsg::Response::TYPE, dataHandler);
	}

	{
		shared_ptr<IBDataHandlerBinder> dataHandler(
				new IBDataHandlerBinder(
						boost::bind(
								MessagingEngine::handleEnableAtomicMsgRequest, this,
								IBDataHandlerBinder::placeholders::connection,
								IBDataHandlerBinder::placeholders::type,
								IBDataHandlerBinder::placeholders::buffer)));

		mDispatcher->registerDataHandler(EnableAtomicMsg::Request::TYPE, dataHandler);
	}

	{
		shared_ptr<IBDataHandlerBinder> dataHandler(
				new IBDataHandlerBinder(
						boost::bind(
								MessagingEngine::handleEnableAtomicMsgResponse, this,
								IBDataHandlerBinder::placeholders::connection,
								IBDataHandlerBinder::placeholders::type,
								IBDataHandlerBinder::placeholders::buffer)));

		mDispatcher->registerDataHandler(EnableAtomicMsg::Response::TYPE, dataHandler);
	}

	// create poller to poll for event for RDMA
	mPoller     = shared_ptr<Poller>(new Poller(ev_loop_new(0)));

	// create RDMA engine with the dispatcher and poller
	mEngine     = shared_ptr<IBNetEngine>(new IBNetEngine());
	mEngine->setDispatcher(mDispatcher);

	// start accepting connection using the default RDMA engine and poller
	engine->accept(mPoller, localEndPoint, boost::bind(MessagingEngine::handleAcceptor, this, _1, _2));

	// create a thread for the poller
	tbb::tbb_thread t(boost::bind(PollerThreadProc, poller));
	tbb::move(mPollerThread, t);
}

MessagingEngine::~MessagingEngine()
{
	// ask the poller to stop itself
	mPoller->terminate();

	// wait for all events being processed by the poller thread
	mPollerThread.join();
}

//////////////////////////////////////////////////////////////////////////
bool MessagingEngine::createRemote(const std::string& name, /*OUT*/ shared_ptr<Remote>& remote)
{
	// create a connection context, which will be associated with the newly created connection later, and store some info there
	ConnectionContext *context = new ConnectionContext;
	context->endPoint = remoteEndPoint;
	context->queueName = queueName;

	// ask the engine to connect to the remote RDMA endpoint
	mEngine->connect(mPoller, remoteEndPoint, boost::bind(MessagingEngine::handleConnector, this, _1, _2, context));

	// wait for the connector to signal the result back
	bool result;
	context->condVariable.wait(result);

	if(result)
	{
		hash_map<std::string, shared_ptr<Remote> >::iterator it = mRemoteInfos.find(name);

		BOOST_ASSERT(it != mRemoteInfos.end());

		remote = it->second;
	}

	SAFE_DELETE(context);

	return result;
}

bool MessagingEngine::createLocal(const std::string& name, shared_ptr<Buffer> buffer, /*OUT*/ shared_ptr<Local>& local)
{
	// check for duplicated local queue name
	hash_map<std::string, shared_ptr<Local> >::const_iterator it = mLocalInfos.find(name);
	if(it != mLocalInfos.end())
		return false;

	// create local queue
	local = shared_ptr<Local>(new Local);
	local->name = name;
	local->rpos = IBBufferManager::instance()->createBuffer(sizeof(uint64));
	local->wpos = IBBufferManager::instance()->createBuffer(sizeof(uint64));
	local->buffer = buffer;

	BOOST_ASSERT(!!local->rpos);
	BOOST_ASSERT(!!local->wpos);
	BOOST_ASSERT(!!local->buffer);

	// insert the local queue into the map
	mLocalInfos[name] = local;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void MessagingEngine::registerDefaultControlHandler(shared_ptr<ControlHandler> Handler)
{
	mControlHandler.def = Handler;
}

void MessagingEngine::unregisterDefaultControlHandler()
{
	mControlHandler.def.reset();
}

void MessagingEngine::registerControlHandler(int32 type, shared_ptr<ControlHandler> Handler)
{
	mControlHandler.map[type] = Handler;
}

void MessagingEngine::unregisterControlHandler(int32 type)
{
	tControlHandlerMap::iterator it = mControlHandler.map.find(type);
	if(it == mControlHandler.map.end())
	{
		LOG4CXX_ERROR(mLogger, "Unable to un-register control handler for type " << type);
	}
	else
	{
		mControlHandler.map.erase(it);
	}
}

//////////////////////////////////////////////////////////////////////////
void MessagingEngine::handleAcceptorCompleted(shared_ptr<IBConnection> connection, int err)
{

}

void MessagingEngine::handleConnectorCompleted(shared_ptr<IBConnection> connection, int err, ConnectionContext *context)
{
	// if the connection was established successfully
	if(err == 0)
	{
		// prepare the "queue info exchange request message"
		shared_ptr<Buffer> buffer = IBBufferManager::instance()->createBuffer(Buffer::probeSize(QueueInfoExchangeMsg::Request));

		QueueInfoExchangeMsg::Request req;
		req.queueIndex = remoteQueueIndex;

		*buffer << req;

		// associate default connection context with the newly created connection
		connection->set<ConnectionContext>(context);

		// send the "queue info exchange message"
		connection->send(QueueInfoExchangeMsg::Request::TYPE, buffer);
	}
}

void MessagingEngine::handleConnected(shared_ptr<IBConnection> connection)
{
	// do nothing
}

void MessagingEngine::handleDisconnected(shared_ptr<IBConnection> connection)
{
	// do nothing
}

//////////////////////////////////////////////////////////////////////////
void MessagingEngine::handleQueueInfoExchangeRequest(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<IBConnection> connection)
{
	BOOST_ASSERT(type == QueueInfoExchangeMsg::Request::TYPE);

	// extract the "queue info exchange request message"
	QueueInfoExchangeMsg::Request req;

	*buffer >> req;

	// find corresponding local queue info
	hash_map<uint32, Local*>::iterator it = mLocalInfos.find(req.queueName);
	if(it != mLocalInfos.end())
	{
		shared_ptr<Buffer> buffer = IBBufferManager::instance()->createBuffer(Buffer::probeSize(QueueInfoExchangeMsg::Request));

		// register RMDA direct buffer and obtain sink id
		it->second->bufferKey = connection->registrerDirect(it->second->buffer);

		// prepare the "queue info exchange response message"
		QueueInfoExchangeMsg::Response res;
		res.success = true;
		res.bufferKey = it->second->bufferKey;

		*buffer << res;

		// send the "queue info exchange response message"
		connection->send(QueueInfoExchangeMsg::Response::TYPE, buffer);
	}
	else
	{
		// failed to find the corresponding local queue info, send response back

		// prepare the "queue info exchange response message"
		shared_ptr<Buffer> buffer = IBBufferManager::instance()->createBuffer(Buffer::probeSize(QueueInfoExchangeMsg::Request));

		QueueInfoExchangeMsg::Response res;
		res.success = false;

		*buffer << res;

		// send the "queue info exchange response message"
		connection->send(QueueInfoExchangeMsg::Response::TYPE, buffer);
	}
}

void MessagingEngine::handleQueueInfoExchangeResponse(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<IBConnection> connection)
{
	// extract the "queue info exchange response message"
	QueueInfoExchangeMsg::Response res;

	*buffer >> res;

	// check the request result
	if(res.success)
	{
		// if the remote queue is found and request is handled successfully, create the remote queue info and save it into container
		ConnectionContext* context = connection->get<ConnectionContext>();

		shared_ptr<Remote> info(new Remote);
		info->name = context->name;
		info->connection = connection;
		info->bufferKey = res.bufferKey;

		mRemoteQueueInfos[context->name] = info;
	}

	// signal the original caller
	context->condVariable.signal(res.success);
}

} }
