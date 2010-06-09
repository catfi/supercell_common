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

#include "core/Prerequisite.h"
#include "core/Worker.h"
#include <iostream>
#include <string>
#include <limits>
#include <tbb/tick_count.h>

#define BOOST_TEST_MODULE WorkerTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( WorkerTest )

void increment(int* value)
{
	++(*value);
}

BOOST_AUTO_TEST_CASE( WorkerTestCase1 )
{
	Worker worker;

	int counter = 0;
	for(int i=0;i<5000;++i)
	{
		worker.post(boost::bind(increment, &counter));
	}
	worker.dispatch(boost::bind(increment, &counter), true);
	BOOST_CHECK(counter == 5001);
}

BOOST_AUTO_TEST_CASE( WorkerTestCase2 )
{
	Worker worker;

	int counter = 0;
	for(int i=0;i<5000;++i)
	{
		worker.dispatch(boost::bind(increment, &counter), false);
	}
	worker.dispatch(boost::bind(increment, &counter), true);
	BOOST_CHECK(counter == 5001);
}

BOOST_AUTO_TEST_CASE( WorkerTestCase3 )
{
	Worker worker;

	int counter = 0;
	for(int i=0;i<5000;++i)
	{
		worker.dispatch(boost::bind(increment, &counter), true);
	}
	BOOST_CHECK(counter == 5000);
}

BOOST_AUTO_TEST_CASE( WorkerTestCase6 )
{
	if(!GlobalWorker::instance())
		new GlobalWorker();

	int counter = 0;
	for(int i=0;i<5000;++i)
	{
		int key = GlobalWorker::instance()->async(boost::bind(increment, &counter));
		GlobalWorker::instance()->wait(key);
		BOOST_CHECK(counter == i+1);
	}
}

BOOST_AUTO_TEST_CASE( WorkerTestCase7 )
{
	WorkerGroup group(2, 2, WorkerGroup::load_balancing_t::round_robin, 10);

	int counter = 0;
	for(int i=0;i<5000;++i)
	{
		int key = group.async(boost::bind(increment, &counter));
		group.wait(key);
		BOOST_CHECK(counter == i+1);
	}
}

BOOST_AUTO_TEST_SUITE_END()
