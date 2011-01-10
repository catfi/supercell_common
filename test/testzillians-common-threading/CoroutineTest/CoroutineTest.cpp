/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
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
 * @date Jan 10, 2011 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "threading/Coroutine.h"

using namespace zillians;

//int main()
//{
//	try
//	{
//		using boost::asio::ip::tcp;
//		using namespace boost::lambda;
//
//		boost::asio::io_service io_service;
//		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 54321));
//
//		const int max_clients = 100;
//		coroutine coro[max_clients];
//		std::auto_ptr<tcp::socket> socket[max_clients];
//		boost::system::error_code ec[max_clients];
//		std::size_t length[max_clients];
//		boost::array<char, 1024> data[max_clients];
//
//		// Kick off all the coroutines.
//		int n = -1;
//		for (int i = 0; i < max_clients; ++i)
//		{
//			socket[i].reset(new tcp::socket(io_service));
//			io_service.post(unlambda((var(n) = i)));
//		}
//
//		for (; io_service.run_one() > 0; n = -1)
//		{
//			if (n != -1)
//			{
//				CoroutineReenter (coro[n])
//				{
//					CoroutineEntry:
//					for (;;)
//					{
//						// Wait for a client to connect.
//						CoroutineYield acceptor.async_accept(
//								*socket[n],
//								unlambda((
//												var(n) = n,
//												var(ec[n]) = boost::lambda::_1
//										)));
//
//						// Echo at will.
//						while (!ec[n])
//						{
//							CoroutineYield socket[n]->async_read_some(
//									boost::asio::buffer(data[n]),
//									unlambda((
//													var(n) = n,
//													var(ec[n]) = boost::lambda::_1,
//													var(length[n]) = boost::lambda::_2
//											)));
//
//							if (!ec[n])
//							{
//								CoroutineYield boost::asio::async_write(
//										*socket[n],
//										boost::asio::buffer(data[n], length[n]),
//										unlambda((
//														var(n) = n,
//														var(ec[n]) = boost::lambda::_1
//												)));
//							}
//						}
//
//						// Clean up before accepting next client.
//						socket[n]->close();
//					}
//				}
//			}
//		}
//	}
//	catch (std::exception& e)
//	{
//		std::cerr << "Exception: " << e.what() << "\n";
//	}
//}

using boost::asio::async_write;
using boost::asio::buffer;
using boost::asio::ip::tcp;
using boost::system::error_code;
using std::size_t;

struct Session : public Coroutine
{
	boost::shared_ptr<tcp::socket> socket_;
	boost::shared_ptr<std::vector<char> > buffer_;

	Session(boost::shared_ptr<tcp::socket> socket) :
		socket_(socket), buffer_(new std::vector<char>(1024))
	{ }

	void operator()(error_code ec = error_code(), size_t n = 0)
	{
		if (!ec)
		{
			CoroutineReenter (this)
			{
				CoroutineEntry:
				for (;;)
				{
					CoroutineYield socket_->async_read_some(buffer(*buffer_), *this);
					CoroutineYield boost::asio::async_write(*socket_, buffer(*buffer_, n), *this);
				}
			}
		}
	}
};

struct Server : public Coroutine
{
	boost::asio::io_service& io_service_;
	boost::shared_ptr<tcp::acceptor> acceptor_;
	boost::shared_ptr<tcp::socket> socket_;

	Server(boost::asio::io_service& io_service) :
		io_service_(io_service), acceptor_(new tcp::acceptor(io_service,
				tcp::endpoint(tcp::v4(), 54321)))
	{ }

	void operator()(error_code ec = error_code())
	{
		CoroutineReenter (this)
		{
			CoroutineEntry:
			for (;;)
			{
				socket_.reset(new tcp::socket(io_service_));
				CoroutineYield acceptor_->async_accept(*socket_, *this);
				io_service_.post(Session(socket_));
			}
		}
	}
};

int main()
{
	boost::asio::io_service io_service;
	io_service.post(Server(io_service));
	io_service.run();
}

