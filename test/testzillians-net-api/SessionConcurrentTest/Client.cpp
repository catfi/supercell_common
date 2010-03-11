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
#include <tbb/atomic.h>

using namespace zillians;
using namespace zillians::net::sys;

using boost::asio::ip::tcp;

log4cxx::LoggerPtr gLogger(log4cxx::Logger::getLogger("session_client"));

struct dummy_message
{
	enum { TYPE = 0 };

	int id;
//	std::vector<int> data;
//	int checksum;

	template<typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & id;
//		ar & data;
//		ar & checksum;
	}
};

struct client_context
{
	int id;
	dummy_message message_to_receive;
	dummy_message message_to_send;
	int total_send;
	tbb::atomic<int> sent_count;
	tbb::atomic<int> read_count;
	int last_message_id;
};

TcpSessionEngine* gEngine;
tbb::atomic<int> gClientCount;
bool gTerminated = false;


// forward declaration
client_context* create_context(int id, int count);
void handle_connected(int id, int count, double* latencies, const boost::system::error_code& ec, TcpSession* session);
void handle_message_write(const boost::system::error_code& ec, TcpSession* session);
void handle_message_read(const boost::system::error_code& ec, TcpSession* session);

client_context* create_context(int id, int count)
{
	client_context* ctx = new client_context;

	ctx->id = id;
	ctx->message_to_send.id = 0;
	ctx->total_send = count;
	ctx->sent_count = 0;
	ctx->read_count = 0;
	ctx->last_message_id = 0;

	return ctx;
}

void handle_connected(const boost::system::error_code& ec, TcpSession* session)
{
	client_context* ctx = session->getContext<client_context>();

	if(!ec)
	{
		++gClientCount;

		for(int i=0;i<ctx->total_send;++i)
		{
			ctx->message_to_send.id++;
			session->writeAsync(ctx->message_to_send, boost::bind(handle_message_write, placeholders::error, session));
		}
		session->readAsync(ctx->message_to_receive, boost::bind(handle_message_read, placeholders::error, session));
	}
	else
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " failed to connect to remote, error = " << ec.message());
		//session->close();
	}
}

void handle_message_write(const boost::system::error_code& ec, TcpSession* session)
{
	client_context* ctx = session->getContext<client_context>();

	int count = ++ctx->sent_count;

	if(!ec)
	{
		//LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " has completed #" << count << " write");
		if(count == ctx->total_send)
		{
			LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " has completed all message writes");
		}
	}
	else
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " failed to write #" << count << " message, error = " << ec.message());
		//session->close();
		//gEngine->stop();
	}
}

void handle_message_read(const boost::system::error_code& ec, TcpSession* session)
{
	client_context* ctx = session->getContext<client_context>();

	int count = ++ctx->read_count;

	if(!ec)
	{
		//LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " complete #" << count << " message read");
		if(ctx->last_message_id + 1 != ctx->message_to_receive.id)
		{
			LOG4CXX_ERROR(gLogger, "client #" << ctx->id << " receives " << ctx->message_to_receive.id << ", expected " << ctx->last_message_id);
		}

		++ctx->last_message_id;

		if(count == ctx->total_send)
		{
			LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " has completed all message reads");

			session->markForDeletion();
			session->close();

			if(--gClientCount == 0)
			{
				gEngine->stop();
			}
		}
		else
		{
			session->readAsync(ctx->message_to_receive, boost::bind(handle_message_read, placeholders::error, session));
		}
	}
	else
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id << " failed to read #" << count << " message, error = " << ec.message());
		//session->close();
	}
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

		gEngine = new TcpSessionEngine();
		gClientCount = 0;

		for(int i=0;i<thread_count;++i)
		{
			TcpSession* new_session = gEngine->createSession();
			client_context* new_context = create_context(i, send_count);
			new_session->setContext<client_context>(new_context);

			gEngine->connectAsync(
					new_session,
					tcp::v4(), std::string(argv[1]), std::string(argv[2]),
					boost::bind(
							handle_connected,
							placeholders::error, new_session));
		}

		gEngine->run();

		SAFE_DELETE(gEngine);
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
