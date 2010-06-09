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
#include "core/Transaction.h"
#include <iostream>
#include <string>
#include <limits>

#define BOOST_TEST_MODULE TransactionTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( TransactionTest )

struct Routines1
{
	struct Context
	{
		int currentState;
	};

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
	Routines1::Context c;
	c.currentState = 0;

	Transaction<Routines1::Context> transaction(c);

	transaction.add(boost::bind(Routines1::action0, _1), boost::bind(Routines1::rollback0, _1));
	transaction.add(boost::bind(Routines1::action1, _1), boost::bind(Routines1::rollback1, _1));
	transaction.add(boost::bind(Routines1::action2, _1), boost::bind(Routines1::rollback2, _1));

	BOOST_CHECK(transaction.next(true) == false);
	BOOST_CHECK(c.currentState == 2);
	BOOST_CHECK(transaction.next(true) == true);
	BOOST_CHECK(c.currentState == 3);
}

BOOST_AUTO_TEST_CASE( TransactionTestCase2 )
{
	Routines1::Context c;
	c.currentState = 0;

	Transaction<Routines1::Context> transaction(c);

	transaction.add(boost::bind(Routines1::action0, _1), boost::bind(Routines1::rollback0, _1));
	transaction.add(boost::bind(Routines1::action1, _1), boost::bind(Routines1::rollback1, _1));
	transaction.add(boost::bind(Routines1::action2, _1), boost::bind(Routines1::rollback2, _1));

	BOOST_CHECK(transaction.next(true) == false);
	BOOST_CHECK(c.currentState == 2);
	BOOST_CHECK(transaction.next(false) == false);
	BOOST_CHECK(transaction.waitForCompletion() == true);
	BOOST_CHECK(c.currentState == 3);
}

struct Routines2
{
	struct Context
	{
		int currentState;
	};

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
	Routines2::Context c;
	c.currentState = 0;

	Transaction<Routines2::Context> transaction(c);

	transaction.add(boost::bind(Routines2::action0, _1), boost::bind(Routines2::rollback0, _1));
	transaction.add(boost::bind(Routines2::action1, _1), boost::bind(Routines2::rollback1, _1));
	transaction.add(boost::bind(Routines2::action2, _1), boost::bind(Routines2::rollback2, _1));

	BOOST_CHECK(transaction.next(true) == false);
	BOOST_CHECK(c.currentState == 2);
	BOOST_REQUIRE_THROW(transaction.next(true), std::runtime_error);
	BOOST_CHECK(c.currentState == 0);
}

BOOST_AUTO_TEST_CASE( TransactionTestCase4 )
{
	Routines2::Context c;
	c.currentState = 0;

	Transaction<Routines2::Context> transaction(c);

	transaction.add(boost::bind(Routines2::action0, _1), boost::bind(Routines2::rollback0, _1));
	transaction.add(boost::bind(Routines2::action1, _1), boost::bind(Routines2::rollback1, _1));
	transaction.add(boost::bind(Routines2::action2, _1), boost::bind(Routines2::rollback2, _1));

	BOOST_CHECK(transaction.next(true) == false);
	BOOST_CHECK(c.currentState == 2);
	BOOST_CHECK(transaction.next(false) == false);
	BOOST_REQUIRE_THROW(transaction.waitForCompletion(), std::runtime_error);
	BOOST_CHECK(c.currentState == 0);
}

struct Routines3
{
	struct Context
	{
		int currentState;
	};

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
		return true;
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
		return true;
	}

	static void rollback2(Context& c)
	{
		BOOST_CHECK(c.currentState == 3);
		--c.currentState;
	}
};

BOOST_AUTO_TEST_CASE( TransactionTestCase6 )
{
	Routines3::Context c;
	c.currentState = 0;

	Transaction<Routines3::Context> transaction(c);

	transaction.add(boost::bind(Routines3::action0, _1), boost::bind(Routines3::rollback0, _1));
	transaction.add(boost::bind(Routines3::action1, _1), boost::bind(Routines3::rollback1, _1));
	transaction.add(boost::bind(Routines3::action2, _1), boost::bind(Routines3::rollback2, _1));

	BOOST_CHECK(transaction.next(true) == true);
	BOOST_CHECK(c.currentState == 3);
	BOOST_CHECK_NO_THROW(transaction.rollback());
	BOOST_CHECK(c.currentState == 0);
}

BOOST_AUTO_TEST_CASE( TransactionTestCase7 )
{
	Routines3::Context c;

	// concate all transaction into a single line
	{
		c.currentState = 0;
		BOOST_CHECK(Transaction<Routines3::Context>(c)
				.add(boost::bind(Routines3::action0, _1), boost::bind(Routines3::rollback0, _1))
				.add(boost::bind(Routines3::action1, _1), boost::bind(Routines3::rollback1, _1))
				.add(boost::bind(Routines3::action2, _1), boost::bind(Routines3::rollback2, _1))
				.next(true) == true);
	}

	// or alternatively,
	{
		c.currentState = 0;
		Transaction<Routines3::Context> transaction(c);
		BOOST_CHECK(transaction
				.add(boost::bind(Routines3::action0, _1), boost::bind(Routines3::rollback0, _1))
				.add(boost::bind(Routines3::action1, _1), boost::bind(Routines3::rollback1, _1))
				.add(boost::bind(Routines3::action2, _1), boost::bind(Routines3::rollback2, _1))
				.next(true) == true);
	}

	// or alternatively,
	{
		c.currentState = 0;
		Transaction<Routines3::Context> transaction(c);
		BOOST_CHECK(transaction
				.add(boost::bind(Routines3::action0, _1), boost::bind(Routines3::rollback0, _1))
				.add(boost::bind(Routines3::action1, _1), boost::bind(Routines3::rollback1, _1))
				.add(boost::bind(Routines3::action2, _1), boost::bind(Routines3::rollback2, _1))
				.next(false) == false);
		BOOST_CHECK(transaction.waitForCompletion() == true);
	}
}

#include <chrono>

struct Routines4
{
	struct Context
	{
		int currentState;
	};

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

	static bool action1_wait_for_condition(Context& c, std::future<bool>* result)
	{
		BOOST_CHECK(c.currentState == 1);
		++c.currentState;
		if(!result->get())
		{
			throw std::runtime_error("");
		}
		return true;
	}

	static void rollback1_wait_for_condition(Context& c)
	{
		BOOST_CHECK(c.currentState == 2);
		--c.currentState;
	}

	static void dummy_signal(std::promise<bool>* result, bool value)
	{
		//std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(1000));
		//std::this_thread::sleep_for(std::chrono::duration<short, std::ratio<1,1> >(1));
		std::this_thread::sleep_for(std::chrono::duration<short, std::ratio<1,20> >(20));
		result->set_value(value);
	}
};

BOOST_AUTO_TEST_CASE( TransactionTestCase8 )
{
	Routines4::Context c;

	{
		c.currentState = 0;
		Transaction<Routines4::Context> transaction(c);

		std::promise<bool> result;
		std::future<bool> fresult = result.get_future();

		std::thread t(boost::bind(Routines4::dummy_signal, &result, true));

		transaction.add(boost::bind(Routines4::action0, _1), boost::bind(Routines4::rollback0, _1));
		transaction.add(boost::bind(Routines4::action1_wait_for_condition, _1, &fresult), boost::bind(Routines4::rollback1_wait_for_condition, _1));
		BOOST_CHECK(transaction.next(true) == true);
		BOOST_CHECK(c.currentState == 2);

		t.join();
	}

	{
		c.currentState = 0;
		Transaction<Routines4::Context> transaction(c);

		std::promise<bool> result;
		std::future<bool> fresult = result.get_future();

		std::thread t(std::bind(Routines4::dummy_signal, &result, false));

		transaction.add(boost::bind(Routines4::action0, _1), boost::bind(Routines4::rollback0, _1));
		transaction.add(boost::bind(Routines4::action1_wait_for_condition, _1, &fresult), boost::bind(Routines4::rollback1_wait_for_condition, _1));
		BOOST_REQUIRE_THROW(transaction.next(true), runtime_error);
		BOOST_CHECK(c.currentState == 0);

		t.join();
	}
}

BOOST_AUTO_TEST_SUITE_END()
