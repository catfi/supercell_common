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
#include "networking/sys/TimeSynchronizer.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <tbb/tbb_thread.h>
#include <tbb/atomic.h>

using namespace zillians;
using namespace zillians::networking::sys;

using boost::asio::ip::tcp;

log4cxx::LoggerPtr gLogger(log4cxx::Logger::getLogger("TimeSynchronizerClient"));
bool gTerminated = false;

TcpSessionEngine* gEngine;

tbb::atomic<int> gClientCount;

struct client_context
{
	int id;
	int send_count;
	TimeSynchronizer<TcpSessionEngine, TcpSession>* synchronizer;
	boost::asio::deadline_timer* timer;
};

// forward declaration
client_context* create_context(int id, int count, double* latencies);
void handle_connected(int id, int count, double* latencies, const boost::system::error_code& ec, TcpSession* session);
void handle_message_write(const boost::system::error_code& ec, TcpSession* session);
void handle_message_read(const boost::system::error_code& ec, TcpSession* session);
void print_current_time(const boost::system::error_code& ec, TcpSession* session);
void handle_session_close(TcpSession& session);
void handle_session_error(TcpSession& session, const boost::system::error_code& ec);


client_context* create_context(int id, int count, boost::asio::deadline_timer* timer)
{
	client_context* ctx = new client_context;

	ctx->id = id;
	ctx->send_count = count;
	ctx->timer = timer;

	return ctx;
}

void handle_connected(const boost::system::error_code& ec, TcpSession* session)
{
	client_context* ctx = session->getContext<client_context>();

	if(!ec)
	{
		gEngine->startDispatch(
				session,
				boost::bind(handle_session_close, placeholders::dispatch::source_ref),
				boost::bind(handle_session_error, placeholders::dispatch::source_ref, placeholders::dispatch::error));

		ctx->synchronizer = new TimeSynchronizer<TcpSessionEngine, TcpSession>(*gEngine, *session);

		ctx->timer->expires_from_now(boost::posix_time::milliseconds(2000));
		ctx->timer->async_wait(boost::bind(print_current_time, placeholders::error, session));

		++gClientCount;
	}
	else
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " failed to connect to remote, error = " << ec.message());
	}
}


void handle_session_close(TcpSession& session)
{
	client_context* ctx = session.getContext<client_context>();
	LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " close connection");
	if(--gClientCount == 0)
	{
		gEngine->stop();
	}
}

void handle_session_error(TcpSession& session, const boost::system::error_code& ec)
{
	client_context* ctx = session.getContext<client_context>();
	LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " encounters an error = " << ec.message());
}

void print_current_time(const boost::system::error_code& ec, TcpSession* session)
{
	client_context* ctx = session->getContext<client_context>();
	if(!ec)
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " current time = " << ctx->synchronizer->current());
	}

	ctx->timer->expires_from_now(boost::posix_time::milliseconds(2000));
	ctx->timer->async_wait(boost::bind(print_current_time, placeholders::error, session));
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

		gClientCount = 0;

		int thread_count = atoi(argv[3]);
		int send_count = atoi(argv[4]);

		boost::asio::io_service ios;

		gEngine = new TcpSessionEngine(&ios);

		for(int i=0;i<thread_count;++i)
		{
			TcpSession* new_session = gEngine->createSession();

			boost::asio::deadline_timer* timer = new boost::asio::deadline_timer(ios);

			client_context* new_context = create_context(i, send_count, timer);
			new_session->setContext<client_context>(new_context);

			gEngine->connectAsync(
					new_session,
					tcp::v4(), std::string(argv[1]), std::string(argv[2]),
					boost::bind(
							handle_connected,
							placeholders::error, new_session));
		}

		//engine.run(thread_count-1);
		gEngine->run();

		SAFE_DELETE(gEngine);

	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
