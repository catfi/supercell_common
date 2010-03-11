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
#include "net-api/sys/Session.h"
#include "net-api/sys/SessionEngine.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <tbb/tbb_thread.h>

using namespace zillians;
using namespace zillians::net::sys;

using boost::asio::ip::tcp;

log4cxx::LoggerPtr gLogger(log4cxx::Logger::getLogger("session_server"));

struct move_message
{
	enum { TYPE = 0 };

	int64 id;
	float positionX;
	float positionY;
	float distance;

	template<typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & id;
		ar & positionX;
		ar & positionY;
		ar & distance;
	}
};

//////////////////////////////////////////////////////////////////////////
struct client_context
{
	int id;
	move_message message;
};

tbb::atomic<int> g_client_unique_id_;

// forward declaration
void handle_listen(TcpSessionEngine* engine, const boost::system::error_code& ec);
void handle_accept(TcpSessionEngine* engine, TcpSession* session, const boost::system::error_code& ec);
void handle_message_read(TcpSession* session, const boost::system::error_code& ec);
void handle_message_write(TcpSession* session, const boost::system::error_code& ec);

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

		// start reading from the newly-connected session
		session->readAsync(ctx->message, boost::bind(handle_message_read, session, placeholders::error));

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

void handle_message_read(TcpSession* session, const boost::system::error_code& ec)
{
	client_context* ctx = session->getContext<client_context>();

	if(!ec)
	{
		//LOG4CXX_INFO(gLogger, "message: id = " << ctx->message.id << ", positionX = " << ctx->message.positionX << ", positionY = " << ctx->message.positionY << ", distance = " << ctx->message.distance);
		session->readAsync(ctx->message, boost::bind(handle_message_read, session, placeholders::error));
		session->writeAsync(ctx->message, boost::bind(handle_message_write, session, placeholders::error));
	}
	else if(ec == boost::asio::error::eof)
	{
		LOG4CXX_INFO(gLogger, "client #" << ctx->id << " close connection");
		if(session->socket().is_open())
		{
			boost::system::error_code error;
			session->socket().close(error);
		}
		session->markForDeletion();
	}
	else
	{
		LOG4CXX_INFO(gLogger, "client #" << ctx->id << " handle message read error, error = " << ec.message());
		if(session->socket().is_open())
		{
			boost::system::error_code error;
			session->socket().close(error);
			if(error)
			{
				LOG4CXX_INFO(gLogger, "client #" << ctx->id << " failed to close socket, error = " << error.message());
			}
		}
		session->markForDeletion();
	}
}

void handle_message_write(TcpSession* session, const boost::system::error_code& ec)
{
	client_context* ctx = session->getContext<client_context>();

	if(!ec)
	{
		//LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " complete message write");
	}
	else if(ec == boost::asio::error::eof)
	{
		LOG4CXX_INFO(gLogger, "client #" << ctx->id << " close connection");
		if(session->socket().is_open())
		{
			boost::system::error_code error;
			session->socket().close(error);
		}
		session->markForDeletion();
	}
	else
	{
		LOG4CXX_INFO(gLogger, "client #" << ctx->id << " handle message write error, error = " << ec.message());
		session->markForDeletion();
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

	TcpSessionEngine engine;

	engine.listenAsync(tcp::v4(), port, boost::bind(handle_listen, &engine, placeholders::error));


	// run the service forever (until all events are processed)
	//io_service.run(thread_count);
	engine.run();

	return 0;
}
