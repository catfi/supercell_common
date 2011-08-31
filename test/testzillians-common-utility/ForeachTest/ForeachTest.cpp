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
#include "utility/Foreach.h"
#include <tr1/unordered_set>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>

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

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase4_0 )
{
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;
	int sum_value = 0;

	foreach(v, vec)
		sum += *v;

	foreach_value(v, vec)
		sum_value += v;

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_value == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase4_1 )
{
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;
	int sum_const = 0;
	int sum_value = 0;

	std::vector<int>& vec_ref = vec;
	const std::vector<int>& vec_const = vec;

	foreach(v, vec_ref)
		sum += *v;

	//typedef boost::mpl::if_<boost::is_const<decltype(vec_const)>, int, double>::type t;
	//typedef boost::mpl::if_<boost::is_same<decltype(vec_const), boost::remove_const<decltype(vec_const)>::type>, int, double>::type t;
//	typedef boost::mpl::if_<boost::is_same<decltype(vec_ref), boost::remove_const<decltype(vec_ref)>::type>, int, double>::type t;
//	printf("%s %s\n", typeid(t).name(), typeid(decltype((vec_const))).name());

	foreach(v, vec_const)
		sum_const += *v;

	foreach_value(v, vec_ref)
		sum_value += v;

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_const == 6);
	BOOST_CHECK(sum_value == 6);
}

template<typename T>
struct dummy_vector
{
	static void test()
	{
		std::vector<T> vec;
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
		int sum = 0;
		int sum_value = 0;

		std::vector<T>& vec_ref = vec;

		deduced_foreach(v, vec_ref)
			sum += *v;

		deduced_foreach_value(v, vec_ref)
			sum_value += v;

		BOOST_CHECK(sum == 6);
		BOOST_CHECK(sum_value == 6);
	}
};

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase4_2 )
{
	dummy_vector<int>::test();
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase4_3 )
{
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;
	int sum_value = 0;

	reverse_foreach(v, vec)
		sum += *v;

	reverse_foreach_value(v, vec)
		sum_value += v;

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_value == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase4_4 )
{
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;
	int sum_value = 0;

	std::vector<int>& vec_ref = vec;

	reverse_foreach(v, vec_ref)
		sum += *v;

	reverse_foreach_value(v, vec_ref)
		sum_value += v;

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_value == 6);
}


template<typename T>
struct reverse_dummy_vector
{
	static void test()
	{
		std::vector<T> vec;
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
		int sum = 0;
		int sum_value = 0;

		std::vector<T>& vec_ref = vec;

		deduced_reverse_foreach(v, vec_ref)
			sum += *v;

		deduced_reverse_foreach_value(v, vec_ref)
			sum_value += v;

		BOOST_CHECK(sum == 6);
		BOOST_CHECK(sum_value == 6);
	}
};

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase4_5 )
{
	reverse_dummy_vector<int>::test();
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase5_0 )
{
	std::list<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;
	int sum_value = 0;

	foreach(v, vec)
		sum += *v;

	foreach_value(v, vec)
		sum_value += v;

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_value == 6);
}


BOOST_AUTO_TEST_CASE( TemplateTricksTestCase5_1 )
{
	std::list<int> l;
	l.push_back(1);
	l.push_back(2);
	l.push_back(3);
	int sum = 0;
	int sum_value = 0;

	std::list<int>& list_ref = l;

	foreach(v, list_ref)
		sum += *v;

	foreach_value(v, list_ref)
		sum_value += v;

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_value == 6);
}

template<typename T>
struct dummy_list
{
	static void test()
	{
		std::list<T> l;
		l.push_back(1);
		l.push_back(2);
		l.push_back(3);
		int sum = 0;
		int sum_value = 0;

		std::list<T>& list_ref = l;

		deduced_foreach(v, list_ref)
			sum += *v;

		deduced_foreach_value(v, list_ref)
			sum_value += v;

		BOOST_CHECK(sum == 6);
		BOOST_CHECK(sum_value == 6);
	}
};

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase5_2 )
{
	dummy_list<int>::test();
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase5_3 )
{
	std::list<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	int sum = 0;
	int sum_value = 0;

	reverse_foreach(v, vec)
		sum += *v;

	reverse_foreach_value(v, vec)
		sum_value += v;

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_value == 6);
}


BOOST_AUTO_TEST_CASE( TemplateTricksTestCase5_4 )
{
	std::list<int> l;
	l.push_back(1);
	l.push_back(2);
	l.push_back(3);
	int sum = 0;
	int sum_value = 0;

	std::list<int>& list_ref = l;

	reverse_foreach(v, list_ref)
		sum += *v;

	reverse_foreach_value(v, list_ref)
		sum_value += v;

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_value == 6);
}

template<typename T>
struct reverse_dummy_list
{
	static void test()
	{
		std::list<T> l;
		l.push_back(1);
		l.push_back(2);
		l.push_back(3);
		int sum = 0;
		int sum_value = 0;

		std::list<T>& list_ref = l;

		deduced_reverse_foreach(v, list_ref)
			sum += *v;

		deduced_reverse_foreach_value(v, list_ref)
			sum_value += v;

		BOOST_CHECK(sum == 6);
		BOOST_CHECK(sum_value == 6);
	}
};

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase5_5 )
{
	reverse_dummy_list<int>::test();
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase6_0 )
{
	std::map<int, int> map;
	map[1] = 1;
	map[2] = 2;
	map[3] = 3;

	int sum_first = 0;
	int sum_second = 0;

	int sum_first_value = 0;
	int sum_second_value = 0;

	foreach(m, map)
	{
		sum_first += m->first;
		sum_second += m->second;
	}

	foreach_value(m, map)
	{
		sum_first_value += m.first;
		sum_second_value += m.second;
	}

	BOOST_CHECK(sum_first == 6);
	BOOST_CHECK(sum_second == 6);
	BOOST_CHECK(sum_first_value == 6);
	BOOST_CHECK(sum_second_value == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase6_1 )
{
	std::map<int, int> map;
	map[1] = 1;
	map[2] = 2;
	map[3] = 3;

	int sum_first = 0;
	int sum_second = 0;

	int sum_first_value = 0;
	int sum_second_value = 0;

	reverse_foreach(m, map)
	{
		sum_first += m->first;
		sum_second += m->second;
	}

	reverse_foreach_value(m, map)
	{
		sum_first_value += m.first;
		sum_second_value += m.second;
	}

	BOOST_CHECK(sum_first == 6);
	BOOST_CHECK(sum_second == 6);
	BOOST_CHECK(sum_first_value == 6);
	BOOST_CHECK(sum_second_value == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase7_0 )
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	using std::unordered_map;
#else
	using std::tr1::unordered_map;
#endif
	unordered_map<int,int> map;
	map.insert(std::make_pair(1,1));
	map.insert(std::make_pair(2,2));
	map.insert(std::make_pair(3,3));

	int sum_first = 0;
	int sum_second = 0;

	int sum_first_value = 0;
	int sum_second_value = 0;

	foreach(m, map)
	{
		sum_first += m->first;
		sum_second += m->second;
	}

	foreach_value(m, map)
	{
		sum_first_value += m.first;
		sum_second_value += m.second;
	}

	BOOST_CHECK(sum_first == 6);
	BOOST_CHECK(sum_second == 6);
	BOOST_CHECK(sum_first_value == 6);
	BOOST_CHECK(sum_second_value == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase7_1 )
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	using std::unordered_map;
#else
	using std::tr1::unordered_map;
#endif
	unordered_map<int,int> map;
	map.insert(std::make_pair(1,1));
	map.insert(std::make_pair(2,2));
	map.insert(std::make_pair(3,3));

	int sum_first = 0;
	int sum_second = 0;
	int sum_first_value = 0;
	int sum_second_value = 0;

	unordered_map<int,int>& map_ref = map;

	foreach(m, map_ref)
	{
		sum_first += m->first;
		sum_second += m->second;
	}

	foreach_value(m, map_ref)
	{
		sum_first_value += m.first;
		sum_second_value += m.second;
	}

	BOOST_CHECK(sum_first == 6);
	BOOST_CHECK(sum_second == 6);
	BOOST_CHECK(sum_first_value == 6);
	BOOST_CHECK(sum_second_value == 6);
}

template<typename T>
struct reverse_dummy_unordered_map
{
	static void test()
	{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
		using std::unordered_map;
#else
		using std::tr1::unordered_map;
#endif

		unordered_map<T,T> map;
		map.insert(std::make_pair(1,1));
		map.insert(std::make_pair(2,2));
		map.insert(std::make_pair(3,3));

		int sum_first = 0;
		int sum_second = 0;
		int sum_first_value = 0;
		int sum_second_value = 0;

		unordered_map<T,T>& map_ref = map;

		deduced_foreach(m, map_ref)
		{
			sum_first += m->first;
			sum_second += m->second;
		}

		deduced_foreach_value(m, map_ref)
		{
			sum_first_value += m.first;
			sum_second_value += m.second;
		}

		BOOST_CHECK(sum_first == 6);
		BOOST_CHECK(sum_second == 6);
		BOOST_CHECK(sum_first_value == 6);
		BOOST_CHECK(sum_second_value == 6);
	}
};

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase7_2 )
{
	reverse_dummy_unordered_map<int>::test();
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase8_0 )
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	using std::unordered_set;
#else
	using std::tr1::unordered_set;
#endif
	unordered_set<int> set;
	set.insert(1);
	set.insert(2);
	set.insert(3);

	int sum = 0;
	int sum_value = 0;

	foreach(s, set)
	{
		sum += *s;
	}

	foreach_value(s, set)
	{
		sum_value += s;
	}

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_value == 6);
}

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase8_1 )
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	using std::unordered_set;
#else
	using std::tr1::unordered_set;
#endif
	unordered_set<int> set;
	set.insert(1);
	set.insert(2);
	set.insert(3);

	int sum = 0;
	int sum_value = 0;

	unordered_set<int>& set_ref = set;

	foreach(s, set_ref)
	{
		sum += *s;
	}

	foreach_value(s, set_ref)
	{
		sum_value += s;
	}

	BOOST_CHECK(sum == 6);
	BOOST_CHECK(sum_value == 6);
}

template<typename T>
struct dummy_unordered_set
{
	static void test()
	{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
		using std::unordered_set;
#else
		using std::tr1::unordered_set;
#endif
		unordered_set<int> set;
		set.insert(1);
		set.insert(2);
		set.insert(3);

		int sum = 0;
		int sum_value = 0;

		unordered_set<int>& set_ref = set;

		deduced_foreach(s, set_ref)
		{
			sum += *s;
		}

		deduced_foreach_value(s, set_ref)
		{
			sum_value += s;
		}

		BOOST_CHECK(sum == 6);
		BOOST_CHECK(sum_value == 6);
	}
};

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase8_2 )
{
	dummy_unordered_set<int>::test();
}

struct complicated_constness
{
	complicated_constness()
	{
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
	}

	int sum()
	{
		int s = 0;

		foreach(i, vec)
			s += *i;
		return s;
	}

	std::vector<int> vec;
};

BOOST_AUTO_TEST_CASE( TemplateTricksTestCase9 )
{
	complicated_constness c;
	BOOST_CHECK(c.sum() == 6);

	const complicated_constness& const_c = c;
	BOOST_CHECK(c.sum() == 6);
}

BOOST_AUTO_TEST_SUITE_END()
