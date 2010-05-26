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

#include "core-api/Prerequisite.h"
#include "net-api/sys/rdma/RdmaDeviceResourceManager.h"
#include "net-api/sys/rdma/RdmaNetEngine.h"
#include "net-api/sys/buffer_manager/RdmaBufferManager.h"
#include "net-api/sys/Poller.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <tbb/tbb_thread.h>
#include <limits.h>

#include "AckQueue.h"

using namespace zillians;
using namespace zillians::net;

//////////////////////////////////////////////////////////////////////////
struct SingletonPool
{
	shared_ptr<RdmaDeviceResourceManager> device_resource_manager;
	shared_ptr<RdmaBufferManager> buffer_manager;
} gSingletonPool;

//////////////////////////////////////////////////////////////////////////
void initSingleton()
{
	gSingletonPool.device_resource_manager = shared_ptr<RdmaDeviceResourceManager>(new RdmaDeviceResourceManager());
	gSingletonPool.buffer_manager = shared_ptr<RdmaBufferManager>(new RdmaBufferManager(256*1024*1024 + RDMA_MINIMAL_MEMORY_USAGE));
}

void finiSingleton()
{
	gSingletonPool.device_resource_manager.reset();
	gSingletonPool.buffer_manager.reset();
}

void printUsage()
{
	fprintf(stderr, "%s <server|client> <address> <threads> <number_of_messages>\n", "RdmaEchoTest");
}

//////////////////////////////////////////////////////////////////////////
class ServerHandler : public RdmaDataHandler, public RdmaConnectionHandler
{
public:
	ServerHandler(shared_ptr<Poller> poller)
	{
		mPoller = poller;
		mConnectionCount = 0;
	}

public:
	virtual void onConnected(shared_ptr<RdmaConnection> connection)
	{
		++mConnectionCount;
		LOG4CXX_INFO(mLogger, "client connected, connection ptr = " << connection.get());
		connection->setMaxSendInFlight(-1);
	}

	virtual void onDisconnected(shared_ptr<RdmaConnection> connection)
	{
		LOG4CXX_INFO(mLogger, "client disconnected, connection ptr = " << connection.get());

		--mConnectionCount;
		// test completed
		if(mConnectionCount == 0)
		{
			mPoller->terminate();
		}
	}

	virtual void onError(shared_ptr<RdmaConnection> connection, int code)
	{
		LOG4CXX_ERROR(mLogger, "client error, connection ptr = " << connection.get() << ", error code = " << code);
	}

public:
	virtual void handle(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<RdmaConnection> connection)
	{
		connection->send(type, buffer);
	}

public:
	shared_ptr<Poller> mPoller;
	int mConnectionCount;

private:
	static log4cxx::LoggerPtr mLogger;

};
log4cxx::LoggerPtr ServerHandler::mLogger(log4cxx::Logger::getLogger("ServerHandler"));

//////////////////////////////////////////////////////////////////////////
class ClientHandler : public RdmaDataHandler, public RdmaConnectionHandler
{
public:
	ClientHandler()
	{
	}

public:
	virtual void onConnected(shared_ptr<RdmaConnection> connection)
	{
		LOG4CXX_INFO(mLogger, "connected to server, connection ptr = " << connection.get());
	}

	virtual void onDisconnected(shared_ptr<RdmaConnection> connection)
	{
		LOG4CXX_INFO(mLogger, "disconnected from server, connection ptr = " << connection.get());
	}

	virtual void onError(shared_ptr<RdmaConnection> connection, int code)
	{
		LOG4CXX_ERROR(mLogger, "client error, connection ptr = " << connection.get() << ", error code = " << code);
	}

public:
	virtual void handle(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<RdmaConnection> connection)
	{
		shared_ptr<AckSlot> ack = boost::static_pointer_cast<AckSlot>(connection->getContext());
		ack->signal();
	}

public:
	static log4cxx::LoggerPtr mLogger;

};
log4cxx::LoggerPtr ClientHandler::mLogger(log4cxx::Logger::getLogger("ClientHandler"));

enum {max_length = 1024};

void poller_thread(shared_ptr<Poller> p)
{
	p->run();
}

void runner_thread(shared_ptr<RdmaConnection> connection, int tid, int count, double freq, double* latencies)
{
	shared_ptr<AckSlot> ack(new AckSlot);
	connection->setContext(ack);

	double latency_sum = 0.0;
	int sent_count = 0;

	char request[max_length];
	char reply[max_length];

	for(int i=0;i<max_length;++i)
		request[i] = i%(std::numeric_limits<char>::max());

	shared_ptr<Buffer> request_buffer(new Buffer(&request[0], max_length));
	shared_ptr<Buffer> reply_buffer(new Buffer(&reply[0], max_length));

	try
	{
		for(int i=1;i<=count;++i)
		{
			tbb::tick_count start, end;

			ack->reset();
			request_buffer->wpos(request_buffer->freeSize());
			connection->send(1000, request_buffer);
			start = tbb::tick_count::now();

			ack->wait();
			end = tbb::tick_count::now();

			{
				++sent_count;
				double elapsed = (end-start).seconds()*1000.0;
				latency_sum += elapsed;
				if(elapsed * 1000.0 < 1000000.0 * freq)
				{
					uint32 time_to_wait = (1000000 * freq) - (elapsed * 1000.0);
					usleep(time_to_wait);
				}
			}

			if(i % (int)(1.0/freq) == 0)
			{
				//LOG4CXX_INFO(gLogger, "\tThread #" << tid << " has avg response time: " << latency_sum/(double)i << " ms");
			}
		}
	}
	catch (std::exception& e)
	{
		LOG4CXX_ERROR(ClientHandler::mLogger, "Exception: " << e.what());
	}

	if(sent_count>0)
	{
		latencies[tid] = latency_sum/(double)(sent_count);
	}
	else
	{
		latencies[tid] = 0.0;
	}
}


double* gLatencies = NULL;
tbb::tbb_thread* gClientThreads = NULL;

void connector_completion_handler(shared_ptr<RdmaConnection> connection, int err, int tid, int send_count)
{
	printf("ConnectorHandler: err = %d\n", err);

	tbb::tbb_thread t(boost::bind(runner_thread, connection, tid, send_count, 0.2, gLatencies));
	tbb::move(gClientThreads[tid], t);
}

void acceptor_completion_handler(shared_ptr<RdmaConnection> connection, int err)
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

		shared_ptr<RdmaDispatcher> dispatcher(new RdmaDispatcher());
		shared_ptr<RdmaNetEngine> engine(new RdmaNetEngine());
		engine->setDispatcher(dispatcher);
		engine->setBufferManager(gSingletonPool.buffer_manager);

		if(strcmp(argv[1], "server") == 0)
		{
			shared_ptr<ServerHandler> server(new ServerHandler(poller));
			dispatcher->registerDataHandler(1000, server);
			dispatcher->registerConnectionHandler(server);

			engine->accept(poller, address, boost::bind(acceptor_completion_handler, _1, _2));

			tbb::tbb_thread t(boost::bind(poller_thread, poller));

			if(t.joinable())
				t.join();

			printf("server ends\n");
		}
		else if(strcmp(argv[1], "client") == 0)
		{
			int thread_count = atoi(argv[3]);
			int send_count = atoi(argv[4]);

			gClientThreads = new tbb::tbb_thread[thread_count];
			gLatencies = new double[thread_count];

			shared_ptr<ClientHandler> client(new ClientHandler());
			dispatcher->registerDataHandler(1000, client);
			dispatcher->registerConnectionHandler(client);

			for(int i=0;i<thread_count;++i)
			{
				engine->connect(poller, address, boost::bind(connector_completion_handler, _1, _2, i, send_count));
			}

			tbb::tbb_thread t(boost::bind(poller_thread, poller));

			if(t.joinable())
				t.join();

			for(int i=0;i<thread_count;++i)
			{
				if(gClientThreads[i].joinable()) gClientThreads[i].join();
			}


			delete[] gClientThreads;
			delete[] gLatencies;

			printf("client ends\n");
		}
		else
		{
			printUsage();
			return -1;
		}
	}

	// finalize singleton classes
	finiSingleton();

	return 0;
}
