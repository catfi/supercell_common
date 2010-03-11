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
#include "net-api/sys/tcp/TcpNetEngine.h"
#include "net-api/sys/buffer_manager/TcpBufferManager.h"
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
	SharedPtr<TcpBufferManager> buffer_manager;
} gSingletonPool;

//////////////////////////////////////////////////////////////////////////
void initSingleton()
{
	gSingletonPool.buffer_manager = SharedPtr<TcpBufferManager>(new TcpBufferManager(2048L*1024L*1024L));
}

void finiSingleton()
{
	gSingletonPool.buffer_manager.reset();
}

void printUsage()
{
	fprintf(stderr, "%s <server|client> <address> <threads> <number_of_messages>\n", "RdmaEchoTest");
}

//////////////////////////////////////////////////////////////////////////
class ServerHandler : public TcpDataHandler, public TcpConnectionHandler
{
public:
	ServerHandler(SharedPtr<Poller> poller)
	{
		mPoller = poller;
		mConnectionCount = 0;
	}

public:
	virtual void onConnected(SharedPtr<TcpConnection> connection)
	{
		int count = ++mConnectionCount;
		LOG4CXX_INFO(mLogger, "\t" << "connection #" << count << " accepted");

		connection->setMaxSendInFlight(-1);
	}

	virtual void onDisconnected(SharedPtr<TcpConnection> connection)
	{
		int count = --mConnectionCount;
		LOG4CXX_INFO(mLogger, "\t" << "connection #" << count << " closed");

		// test completed
		if(count == 0)
		{
			mPoller->terminate();
		}
	}

	virtual void onError(SharedPtr<TcpConnection> connection, int code)
	{
		LOG4CXX_ERROR(mLogger, "client error, connection ptr = " << connection.get() << ", error code = " << code);
	}

public:
	virtual void handle(uint32 type, SharedPtr<Buffer> buffer, SharedPtr<TcpConnection> connection)
	{
		connection->send(type, buffer);
	}

public:
	SharedPtr<Poller> mPoller;
	int mConnectionCount;

private:
	static log4cxx::LoggerPtr mLogger;

};
log4cxx::LoggerPtr ServerHandler::mLogger(log4cxx::Logger::getLogger("ServerHandler"));

//////////////////////////////////////////////////////////////////////////
struct ClientConnectionCtx
{
	tbb::concurrent_bounded_queue<tbb::tick_count> q;
	double latency_sum;
	int sent_count;
};

class ClientHandler : public TcpDataHandler, public TcpConnectionHandler
{
public:
	ClientHandler(SharedPtr<Poller> poller)
	{
		mPoller = poller;
		mConnectionCount = 0;
	}

public:
	virtual void onConnected(SharedPtr<TcpConnection> connection)
	{
		int count = ++mConnectionCount;
	}

	virtual void onDisconnected(SharedPtr<TcpConnection> connection)
	{
		if(mConnectionCount == 0)
		{
			mPoller->terminate();
		}
	}

	virtual void onError(SharedPtr<TcpConnection> connection, int code)
	{
		LOG4CXX_ERROR(mLogger, "client error, connection ptr = " << connection.get() << ", error code = " << code);
	}

public:
	virtual void handle(uint32 type, SharedPtr<Buffer> buffer, SharedPtr<TcpConnection> connection)
	{
		SharedPtr<ClientConnectionCtx> ctx = boost::static_pointer_cast<ClientConnectionCtx>(connection->getContext());

		tbb::tick_count start; ctx->q.pop(start);
		tbb::tick_count end = tbb::tick_count::now();

		double elapsed = (end-start).seconds()*1000.0;
		ctx->latency_sum += elapsed;
		ctx->sent_count++;
	}

public:
	static log4cxx::LoggerPtr mLogger;
	tbb::atomic<int> mConnectionCount;
	SharedPtr<Poller> mPoller;
};

log4cxx::LoggerPtr ClientHandler::mLogger(log4cxx::Logger::getLogger("ClientHandler"));

enum {max_length = 1024};

void poller_thread(SharedPtr<Poller> p)
{
	p->run();
}

tbb::atomic<int> gActiveThreadCount;

void runner_thread(SharedPtr<TcpConnection> connection, int tid, int count, double freq, double* latencies)
{
	++gActiveThreadCount;

	SharedPtr<ClientConnectionCtx> ctx(new ClientConnectionCtx);
	connection->setContext(ctx);

	ctx->latency_sum = 0.0;
	ctx->sent_count = 0;

	char request[max_length];
	char reply[max_length];

	for(int i=0;i<max_length;++i)
		request[i] = i%(std::numeric_limits<char>::max());

	SharedPtr<Buffer> request_buffer(new Buffer(&request[0], max_length));
	SharedPtr<Buffer> reply_buffer(new Buffer(&reply[0], max_length));

	try
	{
		for(int i=1;i<=count;++i)
		{
			request_buffer->wpos(request_buffer->allocatedSize());

			if(!connection->send(1000, request_buffer))
			{
				LOG4CXX_ERROR(ClientHandler::mLogger, "Failed to send");
				break;
			}

			ctx->q.push(tbb::tick_count::now());

			usleep(200000);
		}
	}
	catch (std::exception& e)
	{
		LOG4CXX_ERROR(ClientHandler::mLogger, "Exception: " << e.what());
	}

	sleep(1);

	if(ctx->sent_count>0)
	{
		latencies[tid] = ctx->latency_sum/(double)(ctx->sent_count);
	}
	else
	{
		latencies[tid] = 0.0;
	}

	/*
	try
	{
		connection->close();
	}
	catch(...) { }
	*/
}


double* gLatencies = NULL;
tbb::tbb_thread* gClientThreads = NULL;

void connector_completion_handler(SharedPtr<TcpConnection> connection, int err, int tid, int send_count)
{
	//printf("ConnectorHandler: err = %d\n", err);

	tbb::tbb_thread t(boost::bind(runner_thread, connection, tid, send_count, 0.2, gLatencies));
	tbb::move(gClientThreads[tid], t);
}

void acceptor_completion_handler(SharedPtr<TcpConnection> connection, int err)
{
	//printf("AcceptorHandler: err = %d\n", err);
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
		SharedPtr<Poller> poller(new Poller(ev_loop_new(0)));

		SharedPtr<TcpDispatcher> dispatcher(new TcpDispatcher());
		SharedPtr<TcpNetEngine> engine(new TcpNetEngine());
		engine->setDispatcher(dispatcher);
		engine->setBufferManager(gSingletonPool.buffer_manager);

		if(strcmp(argv[1], "server") == 0)
		{
			SharedPtr<ServerHandler> server(new ServerHandler(poller));
			dispatcher->registerDataHandler(0, server);
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

			gActiveThreadCount = 0;

			gClientThreads = new tbb::tbb_thread[thread_count];
			gLatencies = new double[thread_count];

			SharedPtr<ClientHandler> client(new ClientHandler(poller));
			dispatcher->registerDataHandler(1000, client);
			dispatcher->registerConnectionHandler(client);

			for(int i=0;i<thread_count;++i)
			{
				engine->connect(poller, address, boost::bind(connector_completion_handler, _1, _2, i, send_count));
			}

			tbb::tbb_thread t(boost::bind(poller_thread, poller));

			while(gActiveThreadCount < thread_count)
			{
				usleep(500000);
			}

			for(int i=0;i<thread_count;++i)
			{
				if(gClientThreads[i].joinable())
					gClientThreads[i].join();
			}

			double latency_sum = 0.0;
			double max_latency = std::numeric_limits<double>::min();
			double min_latency = std::numeric_limits<double>::max();

			int latency_div = 0;
			for(int i=0;i<thread_count;++i)
			{
				if(gLatencies[i] > 0.0)
				{
					max_latency = std::max(max_latency, gLatencies[i]);
					min_latency = std::min(min_latency, gLatencies[i]);

					latency_sum += gLatencies[i];
					latency_div++;
				}
			}

			LOG4CXX_INFO(ClientHandler::mLogger, "Avg latency = " << latency_sum / (double)latency_div << " ms");
			LOG4CXX_INFO(ClientHandler::mLogger, "Max latency = " << max_latency << " ms");
			LOG4CXX_INFO(ClientHandler::mLogger, "Min latency = " << min_latency << " ms");

			if(t.joinable())
				t.join();

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
