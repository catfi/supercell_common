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
#include <iostream>
#include <string>
#include <limits>

#define BOOST_TEST_MODULE CoroutineBasicTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( CoroutineBasicTest )

struct CoroutineBasicTestCase1_Coroutine1
{
	struct State : public Coroutine
	{
		State(boost::asio::io_service& _io_service, int _iterations) : io_service(_io_service), timer(_io_service, boost::posix_time::milliseconds(200)), iterations(_iterations), current_iteration(0)
		{ printf("State constructed, this = %p\n", this); }

		~State()
		{ printf("State destroyed, this = %p\n", this); }

		int current_iteration;
		int iterations;
		boost::asio::io_service& io_service;
		boost::asio::deadline_timer timer;
	};

	CoroutineBasicTestCase1_Coroutine1(boost::asio::io_service& _io_service, int _iterations) : state(new State(_io_service, _iterations))
	{ }

	void operator() (const boost::system::error_code& ec = boost::system::error_code())
	{
		printf("this = %p, entered\n", state.get());
		CoroutineReenter(state.get())
		{
			CoroutineEntry:
			for (;state->current_iteration < state->iterations; ++state->current_iteration)
			{
				printf("this = %p, before current iteration = %d\n", state.get(), state->current_iteration);
				CoroutineYield state->timer.async_wait(*this);
				state->timer.expires_at(state->timer.expires_at() + boost::posix_time::milliseconds(200));
				printf("this = %p, after current iteration = %d\n", state.get(), state->current_iteration);
			}
		}
		printf("this = %p, leaved\n", state.get());
	}

	shared_ptr<State> state;
};

BOOST_AUTO_TEST_CASE( CoroutineBasicTestCase1 )
{
	boost::asio::io_service io_service;
	CoroutineBasicTestCase1_Coroutine1 c0(io_service, 10);
	io_service.post(c0);
	CoroutineBasicTestCase1_Coroutine1 c1(io_service, 5);
	io_service.post(c1);
	io_service.run();
}

BOOST_AUTO_TEST_SUITE_END()

