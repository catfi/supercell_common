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
/**
 * @date Feb 28, 2009 sdk - Initial version created.
 */

#include "core-api/Prerequisite.h"
#include "net-api/rdma/infiniband/IBDeviceResourceManager.h"
#include "net-api/rdma/infiniband/IBNetEngine.h"
#include "net-api/rdma/buffer_manager/IBBufferManager.h"
#include "net-api/rdma/Poller.h"
#include "string.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <tbb/tbb_thread.h>

#define MB 1024L*1024L
#define TEST_DIRECT_BUFFER_TYPE	456
#define TEST_DIRECT_BUFFER_SIZE	256L*MB

using namespace zillians;
using namespace zillians::net::rdma;

//////////////////////////////////////////////////////////////////////////
struct SingletonPool
{
	shared_ptr<IBDeviceResourceManager> device_resource_manager;
	shared_ptr<IBBufferManager> buffer_manager;
} gSingletonPool;

//////////////////////////////////////////////////////////////////////////
void initSingleton()
{
	gSingletonPool.device_resource_manager = shared_ptr<IBDeviceResourceManager>(new IBDeviceResourceManager());
	gSingletonPool.buffer_manager = shared_ptr<IBBufferManager>(new IBBufferManager(TEST_DIRECT_BUFFER_SIZE*2 + IB_MINIMAL_MEMORY_USAGE + 10*MB));
}

void finiSingleton()
{
	gSingletonPool.device_resource_manager.reset();
	gSingletonPool.buffer_manager.reset();
}

void printUsage()
{
	fprintf(stderr, "%s <server|client> <listen_address|connecting_address>\n", "InfinibandDirectSendTest");
}

void PollerThreadProc(shared_ptr<Poller> p)
{
	p->run();
}

void checkBuffer(log4cxx::LoggerPtr &mLogger, uint32 type, shared_ptr<Buffer> b)
{
	LOG4CXX_INFO(mLogger, "receive large buffer");

	if(b->dataSize() != TEST_DIRECT_BUFFER_SIZE)
	{
		LOG4CXX_ERROR(mLogger, "large buffer size mismatch! received size = " << b->dataSize());
	}

	LOG4CXX_INFO(mLogger, "checking buffer content...");
	bool passed = true;
	for(int32 i=0;i<TEST_DIRECT_BUFFER_SIZE/sizeof(int32);++i)
	{
		int32 x = 0;
		b->read(x);
		if(x != i)
		{
			LOG4CXX_ERROR(mLogger, "checksum error: [" << i << "] == " << x << " (expect " << i <<")");
			passed = false;
		}
	}

	if(passed)
	{
		LOG4CXX_INFO(mLogger, "checksum passed");
	}
}


//////////////////////////////////////////////////////////////////////////
class ServerHandler : public IBDataHandler, public IBConnectionHandler
{
public:
	ServerHandler(shared_ptr<Poller> poller)
	{
		mPoller = poller;
		mReady = false;
	}

public:
	virtual void onConnected(shared_ptr<IBConnection> connection)
	{
		LOG4CXX_INFO(mLogger, "client connected, connection ptr = " << connection.get());
		std::string st[]={"aaa","bbb","ccc","ddd","eee","fffff"};
		shared_ptr<Buffer> b = connection->createBuffer(TEST_DIRECT_BUFFER_SIZE);
		uint64 key = connection->registrerDirect(b);
		LOG4CXX_INFO(mLogger, "buffer registered, key = " << key);

		LOG4CXX_INFO(mLogger, "preparing the source buffer...");
		sourceBuffer = connection->createBuffer(TEST_DIRECT_BUFFER_SIZE);
		for(int32 i=0;i<TEST_DIRECT_BUFFER_SIZE/sizeof(int32);++i)
		{
			sourceBuffer->write(i);
		}

		LOG4CXX_INFO(mLogger, "sending out the local key (the sink_id)...");
		shared_ptr<Buffer> key_info = connection->createBuffer(sizeof(uint64));
		key_info->write(key);
		connection->send(0, key_info);

		mReady = true;
	}

	virtual void onDisconnected(shared_ptr<IBConnection> connection)
	{
		LOG4CXX_INFO(mLogger, "client disconnected, connection ptr = " << connection.get());

		// test completed
		mPoller->terminate();
	}

	virtual void onError(shared_ptr<IBConnection> connection, int code)
	{
		LOG4CXX_ERROR(mLogger, "client error, connection ptr = " << connection.get() << ", error code = " << code);
	}

public:
	virtual void handle(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<IBConnection> connection)
	{
		if(type == 0)
		{
			// wait until we've prepared the buffer
			while(!mReady) { tbb::this_tbb_thread::yield(); }

			// get the remote key or said the sink_id
			uint64 sink_id; buffer->read(sink_id);

			// send direct via RDMA WRITE
			connection->sendDirect(TEST_DIRECT_BUFFER_TYPE, sourceBuffer, sink_id);
		}
		else if(type == 2)
		{
			LOG4CXX_INFO(mLogger, "received remote finish ack, close");
			connection->close();
		}
		else if(type == TEST_DIRECT_BUFFER_TYPE)
		{
			checkBuffer(mLogger, type, buffer);

			// send a dummy ack buffer back
			shared_ptr<Buffer> ack = connection->createBuffer(4);
			ack->write(0);
			connection->send(2, ack);
		}
	}
private:
	shared_ptr<Poller> mPoller;

private:
	bool mReady;
	shared_ptr<Buffer> sourceBuffer;

private:
	static log4cxx::LoggerPtr mLogger;

};
log4cxx::LoggerPtr ServerHandler::mLogger(log4cxx::Logger::getLogger("ServerHandler"));



//////////////////////////////////////////////////////////////////////////
class ClientHandler : public IBDataHandler, public IBConnectionHandler
{
public:
	ClientHandler(shared_ptr<Poller> poller)
	{
		mPoller = poller;
		mReady = false;
	}

public:
	virtual void onConnected(shared_ptr<IBConnection> connection)
	{
		LOG4CXX_INFO(mLogger, "connected to server, connection ptr = " << connection.get());

		shared_ptr<Buffer> b = connection->createBuffer(TEST_DIRECT_BUFFER_SIZE);
		uint64 key = connection->registrerDirect(b);
		LOG4CXX_INFO(mLogger, "buffer registered, key = " << key);

		LOG4CXX_INFO(mLogger, "preparing the source buffer...");
		sourceBuffer = connection->createBuffer(TEST_DIRECT_BUFFER_SIZE);
		for(int32 i=0;i<TEST_DIRECT_BUFFER_SIZE/sizeof(int32);++i)
		{
			sourceBuffer->write(i);
		}

		LOG4CXX_INFO(mLogger, "sending out the local key (the sink_id)...");
		shared_ptr<Buffer> key_info = connection->createBuffer(sizeof(uint64));
		key_info->write(key);
		connection->send(0, key_info);

		mReady = true;
	}

	virtual void onDisconnected(shared_ptr<IBConnection> connection)
	{
		LOG4CXX_INFO(mLogger, "disconnected from server, connection ptr = " << connection.get());

		// test completed
		mPoller->terminate();
	}

	virtual void onError(shared_ptr<IBConnection> connection, int code)
	{
		LOG4CXX_ERROR(mLogger, "client error, connection ptr = " << connection.get() << ", error code = " << code);
	}

public:
	virtual void handle(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<IBConnection> connection)
	{
		if(type == 0)
		{
			// wait until we've prepared the buffer
			while(!mReady) { tbb::this_tbb_thread::yield(); }

			// get the remote key or said the sink_id
			uint64 sink_id; buffer->read(sink_id);

			// send direct via RDMA WRITE
			connection->sendDirect(TEST_DIRECT_BUFFER_TYPE, sourceBuffer, sink_id);
		}
		else if(type == 2)
		{
			LOG4CXX_INFO(mLogger, "received remote finish ack, close");
			connection->close();
		}
		else if(type == TEST_DIRECT_BUFFER_TYPE)
		{
			checkBuffer(mLogger, type, buffer);

			// send a dummy ack buffer back
			shared_ptr<Buffer> ack = connection->createBuffer(4);
			ack->write(0);
			connection->send(2, ack);
		}
	}
private:
	shared_ptr<Poller> mPoller;

private:
	bool mReady;
	shared_ptr<Buffer> sourceBuffer;

private:
	static log4cxx::LoggerPtr mLogger;

};
log4cxx::LoggerPtr ClientHandler::mLogger(log4cxx::Logger::getLogger("ClientHandler"));

void ConnectorHandler(shared_ptr<IBConnection> connection, int err)
{
	printf("ConnectorHandler: err = %d\n", err);
}

void AcceptorHandler(shared_ptr<IBConnection> connection, int err)
{
	printf("AcceptorHandler: err = %d\n", err);
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	if(argc < 3)
	{
		printUsage();
		return -1;
	}

	// configure the log4cxx to default
	log4cxx::BasicConfigurator::configure();

	std::string address(argv[2]);

	// initialize singleton classes
	initSingleton();

	// start and run the network engine
	{
		shared_ptr<Poller> poller(new Poller(ev_loop_new(0)));

		shared_ptr<IBDispatcher> dispatcher(new IBDispatcher());
		shared_ptr<IBNetEngine> engine(new IBNetEngine());
		engine->setDispatcher(dispatcher);
		engine->setBufferManager(gSingletonPool.buffer_manager);

		if(strcmp(argv[1], "server") == 0)
		{
			shared_ptr<ServerHandler> server(new ServerHandler(poller));

			dispatcher->registerDefaultDataHandler(server);
			dispatcher->registerConnectionHandler(server);

			engine->accept(poller, address, boost::bind(AcceptorHandler, _1, _2));
		}
		else if(strcmp(argv[1], "client") == 0)
		{
			shared_ptr<ClientHandler> client(new ClientHandler(poller));

			dispatcher->registerDefaultDataHandler(client);
			dispatcher->registerConnectionHandler(client);

			engine->connect(poller, address, boost::bind(ConnectorHandler, _1, _2));
		}
		else
		{
			printUsage();
			return -1;
		}

		tbb::tbb_thread poller_thread(boost::bind(PollerThreadProc, poller));

		poller_thread.join();
	}

	// finalize singleton classes
	finiSingleton();

	return 0;
}
