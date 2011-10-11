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
 * @date Aug 6, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_FOREACH_H_
#define ZILLIANS_FOREACH_H_

#include "core/Types.h"
#include <boost/foreach.hpp>
#include <boost/type_traits.hpp>

#include <vector>
#include <list>
#include <map>
#include <set>
#include <queue>
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <unordered_set>
#include <unordered_map>
#else
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#endif
#include <ext/hash_set>
#include <ext/hash_map>

namespace zillians {

template<typename T>
struct foreach_trait;

template<>
struct foreach_trait<zillians::int8>
{
	typedef zillians::int8 container_type;
	typedef zillians::int8 iterator_type;
	typedef zillians::int8 const_iterator_type;
	typedef zillians::int8 value_type;

	static inline iterator_type beginof(container_type a)
	{
		return 0;
	}
	static inline iterator_type endof(container_type a)
	{
		return a;
	}
};

template<>
struct foreach_trait<zillians::int16>
{
	typedef zillians::int16 container_type;
	typedef zillians::int16 iterator_type;
	typedef zillians::int16 const_iterator_type;
	typedef zillians::int16 value_type;

	static inline iterator_type beginof(container_type a)
	{
		return 0;
	}
	static inline iterator_type endof(container_type a)
	{
		return a;
	}
};

template<>
struct foreach_trait<zillians::int32>
{
	typedef zillians::int32 container_type;
	typedef zillians::int32 iterator_type;
	typedef zillians::int32 const_iterator_type;
	typedef zillians::int32 value_type;

	static inline iterator_type beginof(container_type a)
	{
		return 0;
	}
	static inline iterator_type endof(container_type a)
	{
		return a;
	}
};

template<>
struct foreach_trait<zillians::int64>
{
	typedef zillians::int64 container_type;
	typedef zillians::int64 iterator_type;
	typedef zillians::int64 const_iterator_type;
	typedef zillians::int64 value_type;

	static inline iterator_type beginof(container_type a)
	{
		return 0;
	}
	static inline iterator_type endof(container_type a)
	{
		return a;
	}
};

template<>
struct foreach_trait<zillians::uint8>
{
	typedef zillians::uint8 container_type;
	typedef zillians::uint8 iterator_type;
	typedef zillians::uint8 const_iterator_type;
	typedef zillians::uint8 value_type;

	static inline iterator_type beginof(container_type a)
	{
		return 0;
	}
	static inline iterator_type endof(container_type a)
	{
		return a;
	}
};

template<>
struct foreach_trait<zillians::uint16>
{
	typedef zillians::uint16 container_type;
	typedef zillians::uint16 iterator_type;
	typedef zillians::uint16 const_iterator_type;
	typedef zillians::uint16 value_type;

	static inline iterator_type beginof(container_type a)
	{
		return 0;
	}
	static inline iterator_type endof(container_type a)
	{
		return a;
	}
};
template<>
struct foreach_trait<zillians::uint32>
{
	typedef zillians::uint32 container_type;
	typedef zillians::uint32 iterator_type;
	typedef zillians::uint32 const_iterator_type;
	typedef zillians::uint32 value_type;

	static inline iterator_type beginof(container_type a)
	{
		return 0;
	}
	static inline iterator_type endof(container_type a)
	{
		return a;
	}
};
template<>
struct foreach_trait<zillians::uint64>
{
	typedef zillians::uint64 container_type;
	typedef zillians::uint64 iterator_type;
	typedef zillians::uint64 const_iterator_type;
	typedef zillians::uint64 value_type;

	static inline iterator_type beginof(container_type a)
	{
		return 0;
	}
	static inline iterator_type endof(container_type a)
	{
		return a;
	}
};

template <typename T, int N>
struct foreach_trait<T[N]>
{
	typedef T container_type[N];
	typedef T* iterator_type;
	typedef const T* const_iterator_type;
	typedef T value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a; }

	static inline iterator_type endof(container_type& a)
	{ return a + N; }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a; }

	static inline const_iterator_type endof(const container_type& a)
	{ return a + N; }

};

template <typename T, typename Alloc>
struct foreach_trait<std::vector<T, Alloc>>
{
	typedef std::vector<T,Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::reverse_iterator reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_reverse_iterator const_reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline reverse_iterator_type reverse_beginof(container_type& a)
	{ return a.rbegin(); }

	static inline reverse_iterator_type reverse_endof(container_type& a)
	{ return a.rend(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }

	static inline const_reverse_iterator_type reverse_beginof(const container_type& a)
	{ return a.rbegin(); }

	static inline const_reverse_iterator_type reverse_endof(const container_type& a)
	{ return a.rend(); }

};

template <typename T, typename Alloc>
struct foreach_trait<std::list<T, Alloc>>
{
	typedef std::list<T, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::reverse_iterator reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_reverse_iterator const_reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline reverse_iterator_type reverse_beginof(container_type& a)
	{ return a.rbegin(); }

	static inline reverse_iterator_type reverse_endof(container_type& a)
	{ return a.rend(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }

	static inline const_reverse_iterator_type reverse_beginof(const container_type& a)
	{ return a.rbegin(); }

	static inline const_reverse_iterator_type reverse_endof(const container_type& a)
	{ return a.rend(); }

};

template <typename Key, typename T, typename Compare, typename Alloc>
struct foreach_trait<std::map<Key, T, Compare, Alloc>>
{
	typedef std::map<Key, T, Compare, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::reverse_iterator reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_reverse_iterator const_reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline reverse_iterator_type reverse_beginof(container_type& a)
	{ return a.rbegin(); }

	static inline reverse_iterator_type reverse_endof(container_type& a)
	{ return a.rend(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }

	static inline const_reverse_iterator_type reverse_beginof(const container_type& a)
	{ return a.rbegin(); }

	static inline const_reverse_iterator_type reverse_endof(const container_type& a)
	{ return a.rend(); }
};

template <typename Key, typename T, typename Compare, typename Alloc>
struct foreach_trait<std::multimap<Key, T, Compare, Alloc>>
{
	typedef std::multimap<Key, T, Compare, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::reverse_iterator reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_reverse_iterator const_reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline reverse_iterator_type reverse_beginof(container_type& a)
	{ return a.rbegin(); }

	static inline reverse_iterator_type reverse_endof(container_type& a)
	{ return a.rend(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }

	static inline const_reverse_iterator_type reverse_beginof(const container_type& a)
	{ return a.rbegin(); }

	static inline const_reverse_iterator_type reverse_endof(const container_type& a)
	{ return a.rend(); }
};

template <typename Key, typename Compare, typename Alloc>
struct foreach_trait<std::set<Key, Compare, Alloc>>
{
	typedef std::set<Key, Compare, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::reverse_iterator reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_reverse_iterator const_reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline reverse_iterator_type reverse_beginof(container_type& a)
	{ return a.rbegin(); }

	static inline reverse_iterator_type reverse_endof(container_type& a)
	{ return a.rend(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }

	static inline const_reverse_iterator_type reverse_beginof(const container_type& a)
	{ return a.rbegin(); }

	static inline const_reverse_iterator_type reverse_endof(const container_type& a)
	{ return a.rend(); }
};

template <typename Key, typename Compare, typename Alloc>
struct foreach_trait<std::multiset<Key, Compare, Alloc>>
{
	typedef std::multiset<Key, Compare, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::reverse_iterator reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_reverse_iterator const_reverse_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline reverse_iterator_type reverse_beginof(container_type& a)
	{ return a.rbegin(); }

	static inline reverse_iterator_type reverse_endof(container_type& a)
	{ return a.rend(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }

	static inline const_reverse_iterator_type reverse_beginof(const container_type& a)
	{ return a.rbegin(); }

	static inline const_reverse_iterator_type reverse_endof(const container_type& a)
	{ return a.rend(); }
};

#ifdef __GXX_EXPERIMENTAL_CXX0X__
template <typename Value, typename Hash, typename Pred, typename Alloc>
struct foreach_trait<std::unordered_set<Value, Hash, Pred, Alloc>>
{
	typedef std::unordered_set<Value, Hash, Pred, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }
};

template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc>
struct foreach_trait<std::unordered_map<Key, Value, Hash, Pred, Alloc>>
{
	typedef std::unordered_map<Key, Value, Hash, Pred, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }
};
#else
template <typename Value, typename Hash, typename Pred, typename Alloc>
struct foreach_trait<std::tr1::unordered_set<Value, Hash, Pred, Alloc>>
{
	typedef std::tr1::unordered_set<Value, Hash, Pred, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }
};

template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc>
struct foreach_trait<std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc>>
{
	typedef std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }
};
#endif

template <typename Key, typename Value, typename EqualKey, typename Hash, typename Alloc>
struct foreach_trait<__gnu_cxx::hash_map<Key, Value, Hash, EqualKey, Alloc>>
{
	typedef __gnu_cxx::hash_map<Key, Value, Hash, EqualKey, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }
};

template <typename Value, typename Hash, typename EqualKey, typename Alloc>
struct foreach_trait<__gnu_cxx::hash_set<Value, Hash, EqualKey, Alloc>>
{
	typedef __gnu_cxx::hash_set<Value, Hash, EqualKey, Alloc> container_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::iterator iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::const_iterator const_iterator_type;
	typedef BOOST_DEDUCED_TYPENAME container_type::value_type value_type;

	static inline iterator_type beginof(container_type& a)
	{ return a.begin(); }

	static inline iterator_type endof(container_type& a)
	{ return a.end(); }

	static inline const_iterator_type beginof(const container_type& a)
	{ return a.begin(); }

	static inline const_iterator_type endof(const container_type& a)
	{ return a.end(); }
};

struct foreach_value_dummy_cond
{
	foreach_value_dummy_cond() : value(true)
	{ }

	foreach_value_dummy_cond(bool value) : value(value)
	{ }

	foreach_value_dummy_cond& operator = (bool v)
	{
		value = v;
		return *this;
	}

	operator bool() const
	{
		return false;
	}

	bool value;
};

}

#define make_begin(c)	\
		zillians::foreach_trait<boost::remove_const<boost::remove_reference<decltype((c))>::type>::type>::beginof((c))

#define make_end(c)	\
		zillians::foreach_trait<boost::remove_const<boost::remove_reference<decltype((c))>::type>::type>::endof((c))

#define make_reverse_begin(c)	\
		zillians::foreach_trait<boost::remove_const<boost::remove_reference<decltype((c))>::type>::type>::reverse_beginof((c))

#define make_reverse_end(c)	\
		zillians::foreach_trait<boost::remove_const<boost::remove_reference<decltype((c))>::type>::type>::reverse_endof((c))

#define make_deduced_begin(c)	\
		zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::beginof((c))

#define make_deduced_end(c)	\
		zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::endof((c))

#define make_deduced_reverse_begin(c)	\
		zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::reverse_beginof((c))

#define make_deduced_reverse_end(c)	\
		zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::reverse_endof((c))

#define foreach(i, c) \
		for(auto i = make_begin((c)), _end_of_foreach = make_end((c)); i != _end_of_foreach; ++i)


#define foreach_value(i, c) \
		for(auto _iter_foreach = make_begin((c)), _end_of_foreach = make_end((c)); _iter_foreach != _end_of_foreach; ++_iter_foreach) \
			if(zillians::foreach_value_dummy_cond __dummy_cond = true) { } else \
				for(auto i = *_iter_foreach; __dummy_cond.value; __dummy_cond.value = false)

#define deduced_foreach(i, c) \
		for(auto i = make_deduced_begin((c)), _end_of_foreach = make_deduced_end((c)); i != _end_of_foreach; ++i)

#define deduced_foreach_value(i, c) \
		for(auto _iter_foreach = make_deduced_begin((c)), _end_of_foreach = make_deduced_end((c)); _iter_foreach != _end_of_foreach; ++_iter_foreach) \
			if(zillians::foreach_value_dummy_cond __dummy_cond = true) { } else \
				for(auto i = *_iter_foreach; __dummy_cond.value; __dummy_cond.value = false)

#define reverse_foreach(i, c) \
		for(auto i = make_reverse_begin((c)), _end_of_foreach = make_reverse_end((c)); i != _end_of_foreach; ++i)

#define reverse_foreach_value(i, c) \
		for(auto _iter_foreach = make_reverse_begin((c)), _end_of_foreach = make_reverse_end((c)); _iter_foreach != _end_of_foreach; ++_iter_foreach) \
			if(zillians::foreach_value_dummy_cond __dummy_cond = true) { } else \
				for(auto i = *_iter_foreach; __dummy_cond.value; __dummy_cond.value = false)

#define deduced_reverse_foreach(i, c) \
		for(auto i = zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::reverse_beginof((c)), _end_of_foreach = zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::reverse_endof((c)); i != _end_of_foreach; ++i)

#define deduced_reverse_foreach_value(i, c) \
		for(auto _iter_foreach = make_deduced_reverse_begin((c)), _end_of_foreach = make_deduced_reverse_end((c)); _iter_foreach != _end_of_foreach; ++_iter_foreach) \
			if(zillians::foreach_value_dummy_cond __dummy_cond = true) { } else \
				for(auto i = *_iter_foreach; __dummy_cond.value; __dummy_cond.value = false)

#define is_begin_of_foreach(i, c)  \
		((i) == zillians::foreach_trait<boost::remove_const<boost::remove_reference<decltype((c))>::type>::type>::beginof(c))

#define is_end_of_foreach(i, c)  \
		((i+1) == _end_of_foreach)

#define is_begin_of_reverse_foreach(i, c)  \
		((i) == zillians::foreach_trait<boost::remove_const<boost::remove_reference<decltype((c))>::type>::type>::reverse_beginof((c)))

#define is_end_of_reverse_foreach(i, c)  \
		((i+1) == _end_of_foreach)

#define is_begin_of_foreach_value(c)  \
		((_iter_foreach) == zillians::foreach_trait<boost::remove_const<boost::remove_reference<decltype((c))>::type>::type>::beginof((c)))

#define is_end_of_foreach_value(c)	\
		((_iter_foreach+1) == _end_of_foreach)

#define is_begin_of_reverse_foreach_value(c)  \
		((_iter_foreach) == zillians::foreach_trait<boost::remove_const<boost::remove_reference<decltype((c))>::type>::type>::reverse_beginof((c)))

#define is_end_of_reverse_foreach_value(c)	\
		((_iter_foreach+1) == _end_of_foreach)



#define is_begin_of_deduced_foreach(i, c)  \
		((i) == zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::beginof((c)))

#define is_end_of_deduced_foreach(i, c)  \
		((i+1) == _end_of_foreach)

#define is_begin_of_deduced_reverse_foreach(i, c)  \
		((i) == zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::reverse_beginof((c)))

#define is_end_of_deduced_reverse_foreach(i, c)  \
		((i+1) == _end_of_foreach)

#define is_begin_of_deduced_foreach_value(c)  \
		((_iter_foreach) == zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::beginof((c)))

#define is_end_of_deduced_foreach_value(c)	\
		((_iter_foreach+1) == _end_of_foreach)

#define is_begin_of_deduced_reverse_foreach_value(c)  \
		((_iter_foreach) == zillians::foreach_trait<BOOST_DEDUCED_TYPENAME boost::remove_const<BOOST_DEDUCED_TYPENAME boost::remove_reference<decltype((c))>::type>::type>::reverse_beginof((c)))

#define is_end_of_deduced_reverse_foreach_value(c)	\
		((_iter_foreach+1) == _end_of_foreach)

#endif /* ZILLIANS_FOREACH_H_ */
