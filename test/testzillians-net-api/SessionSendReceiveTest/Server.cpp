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

#include "Common.h"

using namespace zillians;
using namespace zillians::net::sys;

using boost::asio::ip::tcp;

log4cxx::LoggerPtr gLogger(log4cxx::Logger::getLogger("Server"));

tbb::atomic<uint32> gSessionUniqueId;

// forward declaration
void handleListen(TcpSessionEngine* engine, const boost::system::error_code& ec);
void handleAccept(TcpSessionEngine* engine, TcpSession* session, const boost::system::error_code& ec);
void handleMessageRead(TcpSession* session, const boost::system::error_code& ec);
void handleClosed();

void handleListen(TcpSessionEngine* engine, const boost::system::error_code& ec)
{
	// create new session object
	TcpSession* sessionNew = engine->createSession();
	MySessionContext* context = createContext(gSessionUniqueId++);
	sessionNew->setContext<MySessionContext>(context);

	// start accepting connections
	engine->acceptAsync(sessionNew, boost::bind(handleAccept, engine, sessionNew, placeholders::error));
}

void handleAccept(TcpSessionEngine* engine, TcpSession* session, const boost::system::error_code& ec)
{
	MySessionContext* ctx = session->getContext<MySessionContext>();

	if (!ec)
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " accepted");

		sendTestMessage(*session);
		//session->readAsync(ctx->buffer, boost::bind(handleMessageRead, session, placeholders::error));
		uint32 dummy_type = 0;
		session->read(dummy_type, ctx->buffer);
		if(verifyTestMessage(*session, ctx->buffer))
		{
			LOG4CXX_INFO(gLogger, "client #" << ctx->id << " message verified");
		}
		else
		{
			LOG4CXX_ERROR(gLogger, "client #" << ctx->id << " verification failed");
		}
		//session->closeAsync(boost::bind(handleClosed));
	}
	else
	{
		LOG4CXX_INFO(gLogger, "client #" << ctx->id << " failed to accept, error = " << ec.message());
		session->markForDeletion();
	}
}

void handleClosed()
{
//	MySessionContext* ctx = session.getContext<MySessionContext>();
//
//	LOG4CXX_INFO(gLogger, "client #" << ctx->id << " closed");
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

	engine.listenAsync(tcp::v4(), port, boost::bind(handleListen, &engine, placeholders::error));


	// run the service forever (until all events are processed)
	//io_service.run(thread_count);
	engine.getIoService().run();

	return 0;
}
