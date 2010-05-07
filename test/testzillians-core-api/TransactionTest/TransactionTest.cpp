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
#include "core-api/Transaction.h"
#include <iostream>
#include <string>
#include <limits>

#define BOOST_TEST_MODULE TransactionTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( TransactionTest )

struct Context
{
	int currentState;
};

struct Routines1
{
	static bool action0(Context& c)
	{
		BOOST_CHECK(c.currentState == 0);
		++c.currentState;
		return true;
	}

	static void rollback0(Context& c)
	{
		BOOST_CHECK(false);
		--c.currentState;
	}

	static bool action1(Context& c)
	{
		BOOST_CHECK(c.currentState == 1);
		++c.currentState;
		return false;
	}

	static void rollback1(Context& c)
	{
		BOOST_CHECK(false);
		--c.currentState;
	}

	static bool action2(Context& c)
	{
		BOOST_CHECK(c.currentState == 2);
		++c.currentState;
		return false;
	}

	static void rollback2(Context& c)
	{
		BOOST_CHECK(false);
		--c.currentState;
	}
};

BOOST_AUTO_TEST_CASE( TransactionTestCase1 )
{
	Context c;
	c.currentState = 0;

	Transaction<Context> transaction(c);

	transaction.add(boost::bind(Routines1::action0, _1), boost::bind(Routines1::rollback0, _1));
	transaction.add(boost::bind(Routines1::action1, _1), boost::bind(Routines1::rollback1, _1));
	transaction.add(boost::bind(Routines1::action2, _1), boost::bind(Routines1::rollback2, _1));

	BOOST_CHECK(transaction.next(true) == false);
	BOOST_CHECK(transaction.next(true) == true);
}

BOOST_AUTO_TEST_CASE( TransactionTestCase2 )
{
	Context c;
	c.currentState = 0;

	Transaction<Context> transaction(c);

	transaction.add(boost::bind(Routines1::action0, _1), boost::bind(Routines1::rollback0, _1));
	transaction.add(boost::bind(Routines1::action1, _1), boost::bind(Routines1::rollback1, _1));
	transaction.add(boost::bind(Routines1::action2, _1), boost::bind(Routines1::rollback2, _1));

	BOOST_CHECK(transaction.next(true) == false);
	BOOST_CHECK(transaction.next(false) == false);
	BOOST_CHECK(transaction.waitForComplete() == true);
}

struct Routines2
{
	static bool action0(Context& c)
	{
		BOOST_CHECK(c.currentState == 0);
		++c.currentState;
		return true;
	}

	static void rollback0(Context& c)
	{
		BOOST_CHECK(c.currentState == 1);
		--c.currentState;
	}

	static bool action1(Context& c)
	{
		BOOST_CHECK(c.currentState == 1);
		++c.currentState;
		return false;
	}

	static void rollback1(Context& c)
	{
		BOOST_CHECK(c.currentState == 2);
		--c.currentState;
	}

	static bool action2(Context& c)
	{
		BOOST_CHECK(c.currentState == 2);
		++c.currentState;
		throw std::runtime_error("error");
		return false;
	}

	static void rollback2(Context& c)
	{
		BOOST_CHECK(c.currentState == 3);
		--c.currentState;
	}
};

BOOST_AUTO_TEST_CASE( TransactionTestCase3 )
{
	Context c;
	c.currentState = 0;

	Transaction<Context> transaction(c);

	transaction.add(boost::bind(Routines2::action0, _1), boost::bind(Routines2::rollback0, _1));
	transaction.add(boost::bind(Routines2::action1, _1), boost::bind(Routines2::rollback1, _1));
	transaction.add(boost::bind(Routines2::action2, _1), boost::bind(Routines2::rollback2, _1));

	BOOST_CHECK(transaction.next(true) == false);
	BOOST_REQUIRE_THROW(transaction.next(true), std::runtime_error);
}

BOOST_AUTO_TEST_CASE( TransactionTestCase4 )
{
	Context c;
	c.currentState = 0;

	Transaction<Context> transaction(c);

	transaction.add(boost::bind(Routines2::action0, _1), boost::bind(Routines2::rollback0, _1));
	transaction.add(boost::bind(Routines2::action1, _1), boost::bind(Routines2::rollback1, _1));
	transaction.add(boost::bind(Routines2::action2, _1), boost::bind(Routines2::rollback2, _1));

	BOOST_CHECK(transaction.next(true) == false);
	BOOST_CHECK(transaction.next(false) == false);
	BOOST_REQUIRE_THROW(transaction.waitForComplete(), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
