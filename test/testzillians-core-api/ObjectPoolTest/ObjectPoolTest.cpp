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
#include "core-api/ObjectPool.h"
#include <iostream>
#include <string>
#include <limits>

#define BOOST_TEST_MODULE ObjectPoolTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( ObjectPoolTest )

#define TEST_NUM_POOLED_OBJECT 2048

class PooledObject : public ObjectPool<PooledObject>
{
};

BOOST_AUTO_TEST_CASE( ObjectPoolTestCase1 )
{
	std::vector<PooledObject*> objects;

	for(int i=0;i<TEST_NUM_POOLED_OBJECT;++i)
	{
		BOOST_CHECK_NO_THROW(objects.push_back(new PooledObject));
	}

	while(objects.size() > 0)
	{
		PooledObject* obj = objects.back();
		BOOST_CHECK_NO_THROW(delete obj);
		BOOST_CHECK_NO_THROW(objects.pop_back());
	}
}

class ConcurrentPooledObject : public ConcurrentObjectPool<ConcurrentPooledObject>
{
};

void allocationProc()
{
	try
	{
		std::vector<ConcurrentPooledObject*> objects;

		for(int i=0;i<TEST_NUM_POOLED_OBJECT;++i)
		{
			objects.push_back(new ConcurrentPooledObject);
		}

		while(objects.size() > 0)
		{
			ConcurrentPooledObject* obj = objects.back();
			objects.pop_back();
			delete obj;
		}

		objects.clear();
	}
	catch(std::exception& e)
	{
		BOOST_ERROR(e.what());
	}
	catch(...)
	{
		BOOST_ERROR("Unknown exception");
	}
}

BOOST_AUTO_TEST_CASE( ObjectPoolTestCase2 )
{
	boost::thread t0(boost::bind(allocationProc));
	boost::thread t1(boost::bind(allocationProc));
	boost::thread t2(boost::bind(allocationProc));

	if(t0.joinable()) BOOST_CHECK_NO_THROW(t0.join());
	if(t1.joinable()) BOOST_CHECK_NO_THROW(t1.join());
	if(t2.joinable()) BOOST_CHECK_NO_THROW(t2.join());
}

BOOST_AUTO_TEST_SUITE_END()
