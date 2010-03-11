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
bool gTerminated = false;

TcpSessionEngine* gEngine;

tbb::atomic<int> gClientCount;

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

struct client_context
{
	int id_;
	move_message m_recv_;
	move_message m_send_;
	int count_to_send_;
	volatile int count_sent_;
	tbb::tick_count start_time_;
	tbb::tick_count end_time_;
	double latency_sum_;
	volatile bool connected_;
	double* latencies_;
};

// forward declaration
client_context* create_context(int id, int count, double* latencies);
void handle_connected(int id, int count, double* latencies, const boost::system::error_code& ec, TcpSession* session);
void handle_message_write(const boost::system::error_code& ec, TcpSession* session);
void handle_message_read(const boost::system::error_code& ec, TcpSession* session);

client_context* create_context(int id, int count, double* latencies)
{
	client_context* ctx = new client_context;

	ctx->m_send_.id = 0;
	ctx->m_send_.positionX = 100.0f;
	ctx->m_send_.positionY = 200.0f;
	ctx->m_send_.distance = 0.5f;

	ctx->connected_ = false;

	ctx->id_ = id;
	ctx->count_to_send_ = count;
	ctx->count_sent_ = 0;
	ctx->latency_sum_ = 0.0;
	ctx->latencies_ = latencies;

	return ctx;
}

void handle_connected(const boost::system::error_code& ec, TcpSession* session)
{
	client_context* ctx = session->getContext<client_context>();

	if(!ec)
	{
		ctx->start_time_ = tbb::tick_count::now();
		session->writeAsync(ctx->m_send_, boost::bind(handle_message_write, placeholders::error, session));
		session->readAsync(ctx->m_recv_, boost::bind(handle_message_read, placeholders::error, session));
		++gClientCount;
	}
	else
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id_ << " failed to connect to remote, error = " << ec.message());
		//session->close();
	}
}

void handle_message_write(const boost::system::error_code& ec, TcpSession* session)
{
	client_context* ctx = session->getContext<client_context>();

	if(!ec)
	{
		//LOG4CXX_DEBUG(gLogger, "client #" << ctx->id_ << " complete message write");
	}
	else
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id_ << " failed to write message, error = " << ec.message());
		--gClientCount;
		//session->close();
	}
}

void handle_message_read(const boost::system::error_code& ec, TcpSession* session)
{
	client_context* ctx = session->getContext<client_context>();

	if(!ec)
	{
		//LOG4CXX_DEBUG(gLogger, "client #" << ctx->id_ << " complete message read");
		ctx->end_time_ = tbb::tick_count::now();
		double elapsed = (ctx->end_time_ - ctx->start_time_).seconds()*1000.0;
		ctx->latency_sum_ += elapsed;
		ctx->count_sent_++;

		//tbb::this_tbb_thread::sleep(tbb::tick_count::interval_t(0.2));

		if(ctx->count_to_send_ > ctx->count_sent_)
		{
			ctx->start_time_ = tbb::tick_count::now();
			session->writeAsync(ctx->m_send_, boost::bind(handle_message_write, placeholders::error, session));
			session->readAsync(ctx->m_recv_, boost::bind(handle_message_read, placeholders::error, session));
		}
		else
		{
			//LOG4CXX_DEBUG(gLogger, "client #" << ctx->id_ << " completed");
			ctx->latencies_[ctx->id_] = ctx->latency_sum_/(double)(ctx->count_sent_);

			session->markForDeletion();
			session->close();

			if(--gClientCount == 0)
			{
				gEngine->stop();
			}
		}
	}
	else
	{
		LOG4CXX_DEBUG(gLogger, "client #" << ctx->id_ << " failed to read message, error = " << ec.message());
		--gClientCount;
		session->close();
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

		gClientCount = 0;

		int thread_count = atoi(argv[3]);
		int send_count = atoi(argv[4]);

		gEngine = new TcpSessionEngine();

		double* latencies = new double[thread_count];

		for(int i=0;i<thread_count;++i)
		{
			TcpSession* new_session = gEngine->createSession();
			client_context* new_context = create_context(i, send_count, latencies);
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

		double latency_sum = 0.0;
		double max_latency = std::numeric_limits<double>::min();
		double min_latency = std::numeric_limits<double>::max();

		int latency_div = 0;

		for(int i=0;i<thread_count;++i)
		{
			if(latencies[i] > 0.0)
			{
				max_latency = std::max(max_latency, latencies[i]);
				min_latency = std::min(min_latency, latencies[i]);

				latency_sum += latencies[i];
				latency_div++;
			}
		}

		LOG4CXX_DEBUG(gLogger, "all client threads joined");

		delete[] latencies; latencies = NULL;

		LOG4CXX_INFO(gLogger, "Avg latency = " << latency_sum / (double)latency_div << " ms");
		LOG4CXX_INFO(gLogger, "Max latency = " << max_latency << " ms");
		LOG4CXX_INFO(gLogger, "Min latency = " << min_latency << " ms");

		SAFE_DELETE(gEngine);

	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
