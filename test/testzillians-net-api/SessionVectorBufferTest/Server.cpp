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
 * @date Jul 22, 2009 sdk - Initial version created.
 */

#include "core-api/Prerequisite.h"
#include "networking/sys/Session.h"
#include "networking/sys/SessionEngine.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <tbb/tbb_thread.h>

//#define USE_BUFFER_DISPATCH
#define USE_MESSAGE_DISPATCH

using namespace zillians;
using namespace zillians::networking::sys;

using boost::asio::ip::tcp;

log4cxx::LoggerPtr gLogger(log4cxx::Logger::getLogger("session_server"));

//////////////////////////////////////////////////////////////////////////
struct client_context
{
	int id;
};

tbb::atomic<int> g_client_unique_id_;
tbb::atomic<int> g_client_counts;

// forward declaration
void handle_listen(TcpSessionEngine* engine, const boost::system::error_code& ec);
void handle_accept(TcpSessionEngine* engine, TcpSession* session, const boost::system::error_code& ec);
void handle_session_close(TcpSessionEngine* engine, TcpSession& session);
void handle_session_error(TcpSession& session, const boost::system::error_code& ec);
void handle_message_write(const boost::system::error_code& ec);

void handle_listen(TcpSessionEngine* engine, const boost::system::error_code& ec)
{
	// create new session object
	TcpSession* new_session = engine->createSession();
	client_context* new_ctx = new client_context;
	new_ctx->id = g_client_unique_id_++;
	new_session->setContext<client_context>(new_ctx);

	// start accepting connections
	engine->acceptAsync(new_session, boost::bind(handle_accept, engine, new_session, placeholders::error));
}


void handle_accept(TcpSessionEngine* engine, TcpSession* session, const boost::system::error_code& ec)
{
	client_context* ctx = session->getContext<client_context>();

	if (!ec)
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " accepted");

		g_client_counts++;

		// let the session engine dispatch message automatically
		engine->startDispatch(
				session,
				boost::bind(handle_session_close, engine, placeholders::dispatch::source_ref),
				boost::bind(handle_session_error, placeholders::dispatch::source_ref, placeholders::dispatch::error));

		// create new session object
		TcpSession* new_session = engine->createSession();
		client_context* new_ctx = new client_context;
		new_ctx->id = g_client_unique_id_++;
		new_session->setContext<client_context>(new_ctx);

		// accept the next connection
		engine->acceptAsync(
				new_session,
				boost::bind(handle_accept, engine, new_session, placeholders::error));
	}
	else
	{
		LOG4CXX_INFO(gLogger, "client #" << ctx->id << " failed to accept, error = " << ec.message());
		session->markForDeletion();
	}
}

void handle_session_close(TcpSessionEngine* engine, TcpSession& session)
{
	client_context* ctx = session.getContext<client_context>();
	LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " close connection");
	if(--g_client_counts == 0)
	{
		engine->stop();
	}
}

void handle_session_error(TcpSession& session, const boost::system::error_code& ec)
{
	client_context* ctx = session.getContext<client_context>();
	LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " encounters an error = " << ec.message());
}

void handle_message_write(const boost::system::error_code& ec)
{
	//client_context* ctx = session->getContext<client_context>();
	//LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " complete message write");
}

void handle_buffer_read(TcpSession& session, uint32 type, shared_ptr<Buffer>& buffer, std::size_t size)
{
	client_context* ctx = session.getContext<client_context>();

	try
	{
		std::vector< shared_ptr<Buffer> > buffers;
		buffers.push_back(buffer);
		buffers.push_back(buffer);
		session.writeAsync(type, buffers, boost::bind(handle_message_write, placeholders::error));
	}
	catch(std::exception& e)
	{
		LOG4CXX_INFO(gLogger, "client #" << ctx->id << " failed write, error = " << e.what());
	}
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <concurrency>\n";
		return 1;
	}

	log4cxx::BasicConfigurator::configure();

	int port = atoi(argv[1]);
	int thread_count = atoi(argv[2]);

	g_client_unique_id_ = 0;
	g_client_counts = 0;

	TcpSessionEngine engine;

	engine.getDispatcher().bind(
			0,
			boost::bind(handle_buffer_read,
					placeholders::dispatch::source_ref,
					placeholders::dispatch::type,
					placeholders::dispatch::buffer_ref,
					placeholders::dispatch::size));

	engine.listenAsync(tcp::v4(), port, boost::bind(handle_listen, &engine, placeholders::error));

	// run the service forever (until all events are processed)
	tbb::tbb_thread** threads = new tbb::tbb_thread*[thread_count];

	for(int i=0;i<thread_count;++i)
	{
		//threads[i] = new tbb::tbb_thread(boost::bind(&boost::asio::io_service::run, &engine.getIoService()));
		threads[i] = new tbb::tbb_thread(boost::bind(&TcpSessionEngine::run, &engine));
	}

	for(int i=0;i<thread_count;++i)
	{
		if(threads[i]->joinable()) threads[i]->join();
		delete threads[i]; threads[i] = NULL;
	}

	delete[] threads; threads = NULL;

	return 0;
}
