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
 * @date Mar 3, 2009 sdk - Initial version created.
 */

#include "core-api/Prerequisite.h"
#include "core-api/JustThread.h"
#include <iostream>
#include <string>
#include <limits>
#include <utility>

#define BOOST_TEST_MODULE AdvancedThreadingTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

std::mutex io_mutex;

void greeting(const char* message)
{
    std::lock_guard<std::mutex> lk(io_mutex);
    std::cout<<message<<std::endl;
}

int calculate_some_lengthy_dummy_value(int y)
{
	int sum = 0;
	for(int i=1;i<=y;++i)
	{
		sum += i;
		std::this_thread::sleep_for(std::chrono::duration<int,std::milli>(200));
	}
	return sum;
}

BOOST_AUTO_TEST_SUITE( AdvancedThreadingTest )

BOOST_AUTO_TEST_CASE( AdvancedThreadingTestCase1 )
{
    std::thread t(greeting,"Hello from another thread");
    greeting("Hello from the main thread");
    t.join();
}

BOOST_AUTO_TEST_CASE( AdvancedThreadingTestCase2 )
{
    std::future<int> some_dummy_value = std::async(calculate_some_lengthy_dummy_value, 10);
    BOOST_CHECK(some_dummy_value.get() == (1+10)*10/2);
}

struct expensive_data
{
	char data[2048];
};

class lazy_init
{
    mutable std::once_flag flag;
    mutable std::auto_ptr<expensive_data> data;

    void do_init() const
    {
        data.reset(new expensive_data);
    }
public:
    expensive_data* const& get_data() const
    {
        std::call_once(flag,&lazy_init::do_init,this);
        return data.get();
    }
};

BOOST_AUTO_TEST_CASE( AdvancedThreadingTestCase3 )
{
	lazy_init lazy;
	BOOST_CHECK(lazy.get_data() != NULL);
}

BOOST_AUTO_TEST_SUITE_END()
