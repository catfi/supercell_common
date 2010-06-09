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

#include "Common.h"

using namespace zillians;
using namespace zillians::networking::sys;

using boost::asio::ip::tcp;

log4cxx::LoggerPtr gLogger(log4cxx::Logger::getLogger("Client"));

// forward declaration
void handleConnected(int id, int count, double* latencies, const boost::system::error_code& ec, TcpSession* session);
void handleMessageRead(const boost::system::error_code& ec, TcpSession* session);
void handleClosed(TcpSession& session);

void handleConnected(const boost::system::error_code& ec, TcpSession* session)
{
	MySessionContext* ctx = session->getContext<MySessionContext>();

	if(!ec)
	{
		//session->readAsync(ctx->buffer, boost::bind(handleMessageRead, placeholders::error, session));
		sendTestMessage(*session);

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
		//session->closeAsync(boost::bind(handleClosed, _1));
	}
	else
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " failed to connect to remote, error = " << ec.message());
	}
}

void handleClosed(TcpSession& session)
{
	MySessionContext* ctx = session.getContext<MySessionContext>();

	LOG4CXX_INFO(gLogger, "client #" << ctx->id << " closed");
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

		int thread_count = atoi(argv[3]);
		int send_count = atoi(argv[4]);

		TcpSessionEngine engine;

		for(int i=0;i<thread_count;++i)
		{
			TcpSession* sessionNew = engine.createSession();

			MySessionContext* ctx = createContext(i);
			sessionNew->setContext(ctx);

			engine.connectAsync(
					sessionNew,
					tcp::v4(), std::string(argv[1]), std::string(argv[2]),
					boost::bind(
							handleConnected,
							placeholders::error, sessionNew));
		}

		//engine.run(thread_count-1);
		engine.getIoService().run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
