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
 * @date May 2, 2009 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "utility/TemplateTricks.h"
#include <tr1/unordered_set>

#define BOOST_TEST_MODULE TemplateTricksTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace zillians;

BOOST_AUTO_TEST_SUITE( TemplateTricksTestSuite )

//HAS_MEMBER_FUNCTION(f, (void, (char)))

//BOOST_AUTO_TEST_CASE( TemplateTricksTestCase1 )
//{
//	struct dummy_t
//	{
//		void f() { }
//	};
//}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase2 )
{
	int xs[] = { 1, 2, 3 };
	int sum = 0;
	foreach(x, xs)
		sum += *x;
	BOOST_CHECK(sum == 6);
}

struct value_t { char const* name; int value; } values[] =
{
    { "foo", 1 },
    { "bar", 2 },
    { "dumb", 3 }
};

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase3 )
{
	int sum = 0;
	foreach(value, values)
	{
		sum += value->value;
	}

	BOOST_CHECK(sum == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase4 )
{
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;

	foreach(v, vec)
		sum += *v;
	BOOST_CHECK(sum == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase5 )
{
	std::list<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;

	foreach(v, vec)
		sum += *v;
	BOOST_CHECK(sum == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase6 )
{
	std::map<int, int> map;
	map[1] = 1;
	map[2] = 2;
	map[3] = 3;

	int sum_first = 0;
	int sum_second = 0;

	foreach(m, map)
	{
		sum_first += m->first;
		sum_second += m->second;
	}

	BOOST_CHECK(sum_first == 6);
	BOOST_CHECK(sum_second == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase7 )
{
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;

	r_foreach(v, vec)
		sum += *v;
	BOOST_CHECK(sum == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase8 )
{
	std::list<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;

	r_foreach(v, vec)
		sum += *v;
	BOOST_CHECK(sum == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase9 )
{
	std::map<int, int> map;
	map[1] = 1;
	map[2] = 2;
	map[3] = 3;

	int sum_first = 0;
	int sum_second = 0;

	r_foreach(m, map)
	{
		sum_first += m->first;
		sum_second += m->second;
	}

	BOOST_CHECK(sum_first == 6);
	BOOST_CHECK(sum_second == 6);
}

BOOST_AUTO_TEST_SUITE_END()
