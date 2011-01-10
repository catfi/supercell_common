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

class session : coroutine
{
public:
	session(boost::asio::ip::tcp::acceptor& acceptor) :
		acceptor_(acceptor),
		socket_(new boost::asio::ip::tcp::socket(acceptor.get_io_service())),
		data_(new boost::array<char, 1024>)
	{ }

	void operator() (boost::system::error_code ec = boost::system::error_code(), size_t length = 0)
	{
		coro_reenter(this)
		{
			coro_entry:
			for (;;)
			{
				coro_yield acceptor_.async_accept(*socket_, *this);

				while (!ec)
				{
					coro_yield socket_->async_read_some(boost::asio::buffer(*data_), *this);

					if (ec) break;

					coro_yield boost::asio::async_write(*socket_, boost::asio::buffer(*data_, length), *this);
				}

				socket_->close();
			}
		}
	}

private:
	boost::asio::ip::tcp::acceptor& acceptor_;
	shared_ptr<boost::asio::ip::tcp::socket> socket_;
	shared_ptr<boost::array<char, 1024> > data_;
};

int main()
{
	try
	{
		using boost::asio::ip::tcp;
		using namespace boost::lambda;

		boost::asio::io_service io_service;
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 54321));

		const int max_clients = 100;
		coroutine coro[max_clients];
		std::auto_ptr<tcp::socket> socket[max_clients];
		boost::system::error_code ec[max_clients];
		std::size_t length[max_clients];
		boost::array<char, 1024> data[max_clients];

		// Kick off all the coroutines.
		int n = -1;
		for (int i = 0; i < max_clients; ++i)
		{
			socket[i].reset(new tcp::socket(io_service));
			io_service.post(unlambda((var(n) = i)));
		}

		for (; io_service.run_one() > 0; n = -1)
		{
			if (n != -1)
			{
				coro_reenter (coro[n])
				{
					coro_entry:
					for (;;)
					{
						// Wait for a client to connect.
						coro_yield acceptor.async_accept(
								*socket[n],
								unlambda((
												var(n) = n,
												var(ec[n]) = boost::lambda::_1
										)));

						// Echo at will.
						while (!ec[n])
						{
							coro_yield socket[n]->async_read_some(
									boost::asio::buffer(data[n]),
									unlambda((
													var(n) = n,
													var(ec[n]) = boost::lambda::_1,
													var(length[n]) = boost::lambda::_2
											)));

							if (!ec[n])
							{
								coro_yield boost::asio::async_write(
										*socket[n],
										boost::asio::buffer(data[n], length[n]),
										unlambda((
														var(n) = n,
														var(ec[n]) = boost::lambda::_1
												)));
							}
						}

						// Clean up before accepting next client.
						socket[n]->close();
					}
				}
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}
