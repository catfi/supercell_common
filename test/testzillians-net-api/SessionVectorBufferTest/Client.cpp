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
#include "networking/sys/Session.h"
#include "networking/sys/SessionEngine.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <tbb/tbb_thread.h>

using namespace zillians;
using namespace zillians::networking::sys;

using boost::asio::ip::tcp;

#define TEST_BUFFER_COUNT 2
#define TEST_SIZE 32

log4cxx::LoggerPtr gLogger(log4cxx::Logger::getLogger("Client"));
volatile bool gTerminated = false;
tbb::atomic<int> gFinishedClients;
int thread_count;

struct client_context
{
	int id_;
	std::vector< shared_ptr<Buffer> > send_buffers;
	int count_to_send;
	volatile int count_sent;
	volatile bool connected;
};

// forward declaration
client_context* create_context(int id, int count);
void handle_connected(int id, int count, double* latencies, const boost::system::error_code& ec, TcpSession* session);
void handle_message_write(const boost::system::error_code& ec);
void handle_buffer_read(TcpSession& session, uint32 type, shared_ptr<Buffer>& buffer, std::size_t size);
void handle_session_close(TcpSessionEngine* engine, TcpSession& session);
void handle_session_error(TcpSession& session, const boost::system::error_code& ec);

client_context* create_context(int id, int count)
{
	client_context* ctx = new client_context;

	for(int c=0;c<TEST_BUFFER_COUNT;++c)
	{
		shared_ptr<Buffer> buffer(new Buffer(TEST_SIZE*sizeof(uint)));
		for(int i=0;i<TEST_SIZE;++i)
		{
			*buffer << i;
		}
		ctx->send_buffers.push_back(buffer);
	}

	ctx->connected = false;

	ctx->id_ = id;
	ctx->count_to_send = count;
	ctx->count_sent = 0;

	return ctx;
}

void handle_connected(const boost::system::error_code& ec, TcpSessionEngine* engine, TcpSession* session)
{
	client_context* ctx = session->getContext<client_context>();

	if(!ec)
	{
		engine->startDispatch(
				session,
				boost::bind(handle_session_close, engine, placeholders::dispatch::source_ref),
				boost::bind(handle_session_error, placeholders::dispatch::source_ref, placeholders::dispatch::error));

		session->writeAsync(0, ctx->send_buffers, boost::bind(handle_message_write, placeholders::error));

	}
	else
	{
		gTerminated = true;
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id_ << " failed to connect to remote, error = " << ec.message());
	}
}

void handle_session_close(TcpSessionEngine* engine, TcpSession& session)
{
	if(++gFinishedClients == thread_count)
	{
		engine->stop();
		gTerminated = true;
	}
}

void handle_session_error(TcpSession& session, const boost::system::error_code& ec)
{
}

void handle_message_write(const boost::system::error_code& ec)
{
	if(!ec)
	{
	}
	else
	{
		gTerminated = true;
		LOG4CXX_DEBUG(gLogger, "client failed to write message, error = " << ec.message());
	}
}

void handle_buffer_read(TcpSession& session, uint32 type, shared_ptr<Buffer>& buffer, std::size_t size)
{
	client_context* ctx = session.getContext<client_context>();

	BOOST_ASSERT(type == 0);

	// check content
	BOOST_ASSERT(buffer->dataSize() == TEST_SIZE * TEST_BUFFER_COUNT * 2);
	for(int i=0;i<TEST_SIZE * TEST_BUFFER_COUNT * 2;++i)
	{
		int x;
		*buffer >> x;
		if(x != i % TEST_SIZE)
		{
			printf("expected %d, received %d\n", i%TEST_SIZE, x);
		}
	}

	if(ctx->count_to_send > ctx->count_sent)
	{
		session.writeAsync(0, ctx->send_buffers, boost::bind(handle_message_write, placeholders::error));
		++ctx->count_sent;
	}
	else
	{
		session.close();
	}
}

void runProc(TcpSessionEngine* engine)
{
	engine->run();
	LOG4CXX_INFO(gLogger, "runProc exit");
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure();

	try
	{
		if (argc != 5)
		{
			std::cerr << "Usage: " << argv[0] << " <host> <port> <threads> <count>\n";
			return 1;
		}

		gFinishedClients = 0;

		thread_count = atoi(argv[3]);
		int send_count = atoi(argv[4]);

		TcpSessionEngine engine;

		engine.getDispatcher().bind(
				0,
				boost::bind(handle_buffer_read,
						placeholders::dispatch::source_ref,
						placeholders::dispatch::type,
						placeholders::dispatch::buffer_ref,
						placeholders::dispatch::size));

		tbb::tbb_thread runner(boost::bind(runProc, &engine));

		for(int i=0;i<thread_count;++i)
		{
			TcpSession* new_session = engine.createSession();
			client_context* new_context = create_context(i, send_count);
			new_session->setContext<client_context>(new_context);

			engine.connect(
					new_session,
					tcp::v4(), std::string(argv[1]), std::string(argv[2]));

			boost::system::error_code ec;
			handle_connected(ec, &engine, new_session);
		}

		runner.join();

		LOG4CXX_DEBUG(gLogger, "all client threads joined");
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
