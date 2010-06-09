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
 * @date Feb 26, 2009 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "networking/rdma/infiniband/IBDeviceResourceManager.h"
#include "networking/rdma/infiniband/IBNetEngine.h"
#include "networking/rdma/buffer_manager/IBBufferManager.h"
#include "networking/rdma/Poller.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <tbb/tbb_thread.h>
#include <limits.h>

int TEST_SEND_COUNT = 1000;

int LOG_INTERVAL = TEST_SEND_COUNT / 1000;

#define TEST_BUFFER_TYPE	123
#define TEST_BUFFER_SIZE	32*1024

using namespace zillians;
using namespace zillians::networking::rdma;

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
	gSingletonPool.buffer_manager = shared_ptr<IBBufferManager>(new IBBufferManager(256*1024*1024 + IB_MINIMAL_MEMORY_USAGE));
}

void finiSingleton()
{
	gSingletonPool.device_resource_manager.reset();
	gSingletonPool.buffer_manager.reset();
}

void printUsage()
{
	fprintf(stderr, "%s <server|client> <listen_address|connecting_address> <number_of_messages>\n", "InfinibandBasicSendReceiveTest");
}

void PollerThreadProc(shared_ptr<Poller> p)
{
	p->run();
}

//////////////////////////////////////////////////////////////////////////
class ServerHandler : public IBDataHandler, public IBConnectionHandler
{
public:
	ServerHandler(shared_ptr<Poller> poller)
	{
		mPoller = poller;
		mConnectionCount = 0;
	}

public:
	virtual void onConnected(shared_ptr<IBConnection> connection)
	{
		++mConnectionCount;
		LOG4CXX_INFO(mLogger, "client connected, connection ptr = " << connection.get());
		connection->setMaxSendInFlight(-1);
	}

	virtual void onDisconnected(shared_ptr<IBConnection> connection)
	{
		LOG4CXX_INFO(mLogger, "client disconnected, connection ptr = " << connection.get());

		--mConnectionCount;
		// test completed
		if(mConnectionCount == 0)
		{
			mPoller->terminate();
		}
	}

	virtual void onError(shared_ptr<IBConnection> connection, int code)
	{
		LOG4CXX_ERROR(mLogger, "client error, connection ptr = " << connection.get() << ", error code = " << code);
	}

public:
	virtual void handle(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<IBConnection> connection)
	{

		static int s = 0;
		// here we simply echo it back to client
		if(s%LOG_INTERVAL == 0)
		{
			//LOG4CXX_INFO(mLogger, "#" << s << " echo buffer back to connection " << connection.get() << ", type = " << type << ", buffer data size = " << buffer->dataSize());
		}
		connection->send(type, buffer);

		++s;

		/*
		static int s = 0;

		BOOST_ASSERT(type == TEST_BUFFER_TYPE);
		BOOST_ASSERT(buffer->dataSize() == TEST_BUFFER_SIZE);
		BOOST_ASSERT(connection.get() != NULL);

		// check buffer content
		for(int i=0;i<TEST_BUFFER_SIZE/sizeof(int);++i)
		{
			int x = buffer->read<int>();
			if(x != s)
			{
				LOG4CXX_ERROR(mLogger, "buffer #" << i << " = " << x << " != " << s << " (checksum error)");
				buffer->wpos(buffer->dataSize() - buffer->wpos());
				break;
			}
		}

		if(buffer->dataSize() != 0)
		{
			LOG4CXX_ERROR(mLogger, "remaining size = " << buffer->dataSize() << ", rpos = " << buffer->rpos() << ", wpos = " << buffer->wpos());
			BOOST_ASSERT(buffer->dataSize() == 0);
		}

		if(s%LOG_INTERVAL == 0)
		{
			LOG4CXX_INFO(mLogger, "buffers #" << s << " checked");
		}

		if(s == TEST_SEND_COUNT-1)
		{log4cxx::BasicConfigurator::configure();
			// finish all test, disconnect
			LOG4CXX_INFO(mLogger, "finish all test, disconnect");
			connection->close();
		}

		++s;
		*/
	}

private:
	shared_ptr<Poller> mPoller;
	int mConnectionCount;

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
	}

public:
	virtual void onConnected(shared_ptr<IBConnection> connection)
	{
		LOG4CXX_INFO(mLogger, "connected to server, connection ptr = " << connection.get());

		// as we got connected, create a thread to send a series of messages
		tbb::tbb_thread t(boost::bind(&ClientHandler::run, this, connection));
		tbb::move(mSender, t);
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
	/**
	 * We implemented some simple application level flow control here!
	 * @param connection
	 */
	void run(shared_ptr<IBConnection> connection)
	{
		bool slow_down = false;
		double slow_factor = 0.0005;
		double observation_time = 1.0;
		double slow_step = 0.0005;
		int ok_count = 0;
		int ok_threshold = 100000;

		for(int s=0;s<TEST_SEND_COUNT;++s)
		{
			shared_ptr<Buffer> buffer = connection->createBuffer(TEST_BUFFER_SIZE);

			BOOST_ASSERT(buffer.get() != NULL);
			BOOST_ASSERT(buffer->allocatedSize() == TEST_BUFFER_SIZE);
			BOOST_ASSERT(buffer->dataSize() == 0);

			for(int i=0;i<TEST_BUFFER_SIZE/sizeof(int);++i)
			{
				buffer->write(s);
			}

			int size = TEST_BUFFER_SIZE/sizeof(int); size *= sizeof(int);
			BOOST_ASSERT(buffer->dataSize() == size);

			/*
			if(!slow_down)
			{
				tbb::this_tbb_thread::yield();
			}
			else
			{
				tbb::tick_count::interval_t t(slow_factor);
				tbb::this_tbb_thread::sleep(t);
			}

			bool halt = false;
			while(!connection->send(TEST_BUFFER_TYPE, buffer))
			{
				halt = true;

				slow_factor += slow_step;
				ok_threshold = observation_time/slow_factor;
				LOG4CXX_INFO(mLogger, "sending too fast, halt for a moment and resend, slow_factor = " << slow_factor);
				tbb::tick_count::interval_t t(slow_factor);
				tbb::this_tbb_thread::sleep(t);

				if(!slow_down)
				{
					LOG4CXX_INFO(mLogger, "enable application-level flow control");
				}
				slow_down = true;
			}

			if(!halt && slow_down)
			{
				++ok_count;

				if(ok_count > ok_threshold)
				{
					slow_factor -= slow_step;
					ok_threshold = observation_time/slow_factor;

					LOG4CXX_INFO(mLogger, "reducing rate, slow_factor = " << slow_factor);

					if(slow_factor <= 0.0005)
					{
						LOG4CXX_INFO(mLogger, "disable application-level flow control");
						slow_down = false;
					}
					ok_count = 0;
				}
			}
			*/

			if(!connection->sendThrottled(TEST_BUFFER_TYPE, buffer))
			{
				LOG4CXX_INFO(mLogger, "failed to send, connection problem");
				break;
			}

			if(s%LOG_INTERVAL == 0)
			{
				LOG4CXX_INFO(mLogger, "buffers #" << s << " sent");
			}
		}
	}

public:
	virtual void handle(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<IBConnection> connection)
	{
		static int s = 0;

		BOOST_ASSERT(type == TEST_BUFFER_TYPE);
		BOOST_ASSERT(buffer->dataSize() == TEST_BUFFER_SIZE);
		BOOST_ASSERT(connection.get() != NULL);

		// check buffer contenteducing rate, slow_factor = " << slow_factor);
		for(int i=0;i<TEST_BUFFER_SIZE/sizeof(int);++i)
		{
			int x; buffer->read(x);
			if(x != s)
			{
				LOG4CXX_ERROR(mLogger, "buffer #" << i << " = " << x << " != " << s << " (checksum error)");
				buffer->wpos(buffer->dataSize() - buffer->wpos());
				break;
			}
		}

		if(buffer->dataSize() != 0)
		{
			LOG4CXX_ERROR(mLogger, "remaining size = " << buffer->dataSize() << ", rpos = " << buffer->rpos() << ", wpos = " << buffer->wpos());
			BOOST_ASSERT(buffer->dataSize() == 0);
		}

		if(s%LOG_INTERVAL == 0)
		{
			LOG4CXX_INFO(mLogger, "buffers #" << s << " checked");
			LOG4CXX_INFO(mLogger, "memory usage = " << gSingletonPool.buffer_manager->bytesInUsed() / 1048576.0 << " MB");
		}

		if(s == TEST_SEND_COUNT - 1)
		{
			// finish all test, disconnect
			connection->close();
		}

		++s;
	}
private:
	shared_ptr<Poller> mPoller;

private:
	tbb::tbb_thread mSender;

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

	if(strcmp(argv[1], "client") == 0/* || strcmp(argv[1], "server") == 0*/)
	{
		if(argc < 4)
		{
			printUsage();
			return -1;
		}
		TEST_SEND_COUNT = atoi(argv[3]);

		if(TEST_SEND_COUNT == -1)
		{
			// send forever
			TEST_SEND_COUNT = INT_MAX;
			LOG_INTERVAL = 1000000;
		}
		else
		{
			LOG_INTERVAL = TEST_SEND_COUNT/1000;
		}
	}

	// configure the log4cxx to default
	log4cxx::BasicConfigurator::configure();

	std::string address(argv[2]);

	// initialize singleton classes
	initSingleton();

	// start and run the network engine
	if(1)
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
