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
 * @date Aug 10, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_DEMANGLINGUTIL_H_
#define ZILLIANS_DEMANGLINGUTIL_H_

#include <typeinfo>
#include <string>
#include <sstream>
#include <boost/tuple/tuple.hpp>

namespace zillians {

std::string demangle(const std::type_info &ti);

template<typename T>
std::string demangle()
{
	return demangle(typeid(T));
}


namespace detail {
/**
 * Print boost::tuple recursively
 */
template<typename T>
struct tuple_dumper
{
	static void dump(std::stringstream& ss)
	{
		ss << demangle<T>();
	}
};

template<typename T, int N>
struct recursive_tuple_dumper
{
	static void dump(std::stringstream& ss)
	{
		tuple_dumper<typename boost::tuples::element< boost::tuples::length<T>::value - N,T>::type>::dump(ss);

		if(N-1 != 0)
			ss << std::string(",");
		recursive_tuple_dumper<T, N-1>::dump(ss);
	}
};

template<typename T>
struct recursive_tuple_dumper<T, 0>
{
	static void dump(std::stringstream& ss)
	{ }
};

template<>
struct tuple_dumper<boost::tuples::null_type>
{
	static void dump(std::stringstream& ss)
	{
		ss << "null";
	}
};

template<typename Head, typename Tail>
struct tuple_dumper<boost::tuples::cons<Head, Tail>>
{
	typedef boost::tuples::cons<Head, Tail> current_tuple;

	static void dump(std::stringstream& ss)
	{
		ss << "tuple<";
		recursive_tuple_dumper<current_tuple, boost::tuples::length<current_tuple>::value>::dump(ss);
		ss << ">";
	}
};

template <class T0, class T1, class T2, class T3, class T4,
          class T5, class T6, class T7, class T8, class T9>
struct tuple_dumper<boost::tuple<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>>
{
	typedef boost::tuple<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> current_tuple;

	static void dump(std::stringstream& ss)
	{
		ss << "tuple<";
		recursive_tuple_dumper<current_tuple, boost::tuples::length<current_tuple>::value>::dump(ss);
		ss << ">";
	}
};

//template<>
//struct tuple_dumper<default_dim>
//{
//	static void dump(std::stringstream& ss)
//	{
//		ss << "default<>";
//	}
//};

}

template<typename T>
std::string demangle_tuple()
{
	std::stringstream ss;
	detail::tuple_dumper<T>::dump(ss);
	return ss.str();
}

}

#endif /* ZILLIANS_DEMANGLINGUTIL_H_ */
