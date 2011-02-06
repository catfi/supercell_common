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

#ifndef ZILLIANS_TEMPLATETRICKS_H_
#define ZILLIANS_TEMPLATETRICKS_H_

#include <boost/mpl/assert.hpp>
#include <boost/mpl/has_xxx.hpp>

// Introspection for member templates.
#define HAS_MEMBER_TEMPLATE_ACCESS( \
            args, class_type, param \
        ) \
    class_type::template BOOST_PP_ARRAY_ELEM(1, args) \
/**/
#define HAS_MEMBER_TEMPLATE_SUBSTITUTE_PARAMETER( \
            args, param \
        ) \
    template< \
        BOOST_PP_ENUM_PARAMS( \
            BOOST_PP_ARRAY_ELEM(2, args) \
          , typename param \
        ) \
> \
    class param

#define HAS_MEMBER_TEMPLATE(name, n) \
    BOOST_MPL_HAS_MEMBER_IMPLEMENTATION( \
        ( 4, ( BOOST_PP_CAT(has_, name), name, n, false ) ) \
      , BOOST_MPL_HAS_MEMBER_INTROSPECT \
      , HAS_MEMBER_TEMPLATE_SUBSTITUTE_PARAMETER \
      , HAS_MEMBER_TEMPLATE_ACCESS \
    )

// Introspection for member functions.
#define HAS_MEMBER_FUNCTION_ACCESS( \
            args, class_type, param \
        ) \
    &class_type::BOOST_PP_ARRAY_ELEM(1, args)

#define HAS_MEMBER_FUNCTION_SUBSTITUTE_PARAMETER( \
            args, param \
        ) \
    BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_ARRAY_ELEM(4, args)) \
    (U::*) \
    BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_ARRAY_ELEM(4, args))

#define HAS_MEMBER_FUNCTION(name, sig) \
    BOOST_MPL_HAS_MEMBER_IMPLEMENTATION( \
        ( 5, ( BOOST_PP_CAT(has_, name), name, 0, false, sig ) ) \
      , BOOST_MPL_HAS_MEMBER_INTROSPECT \
      , HAS_MEMBER_FUNCTION_SUBSTITUTE_PARAMETER \
      , HAS_MEMBER_FUNCTION_ACCESS \
    )

// Introspection for member static functions.
#define HAS_MEMBER_STATIC_FUNCTION_SUBSTITUTE_PARAMETER( \
            args, param \
        ) \
    BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_ARRAY_ELEM(4, args)) \
    (*) \
    BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_ARRAY_ELEM(4, args))


#define HAS_MEMBER_STATIC_FUNCTION(name, sig) \
    BOOST_MPL_HAS_MEMBER_IMPLEMENTATION( \
        ( 5, ( BOOST_PP_CAT(has_, name), name, 0, false, sig ) ) \
      , BOOST_MPL_HAS_MEMBER_INTROSPECT \
      , HAS_MEMBER_STATIC_FUNCTION_SUBSTITUTE_PARAMETER \
      , HAS_MEMBER_FUNCTION_ACCESS \
    )

/* foreach support for non-container type, the indexer iterates through 0 ~ (A-1) */
static inline zillians::int8 beginof(zillians::int8 a) { return 0; }
static inline zillians::int8 endof(zillians::int8 a) { return a; }
static inline zillians::uint8 beginof(zillians::uint8 a) { return 0; }
static inline zillians::uint8 endof(zillians::uint8 a) { return a; }

static inline zillians::int16 beginof(zillians::int16 a) { return 0; }
static inline zillians::int16 endof(zillians::int16 a) { return a; }
static inline zillians::uint16 beginof(zillians::uint16 a) { return 0; }
static inline zillians::uint16 endof(zillians::uint16 a) { return a; }

static inline zillians::int32 beginof(zillians::int32 a) { return 0; }
static inline zillians::int32 endof(zillians::int32 a) { return a; }
static inline zillians::uint32 beginof(zillians::uint32 a) { return 0; }
static inline zillians::uint32 endof(zillians::uint32 a) { return a; }

static inline zillians::int64 beginof(zillians::int64 a) { return 0; }
static inline zillians::int64 endof(zillians::int64 a) { return a; }
static inline zillians::uint64 beginof(zillians::uint64 a) { return 0; }
static inline zillians::uint64 endof(zillians::uint64 a) { return a; }

template <typename T, int N> static inline T* beginof (T (&a)[N]) { return a; }
template <typename T, int N> static inline T* endof (T (&a)[N])   { return a + N; }

// foreach support for std::vector
#include <vector>
template <typename T, typename Alloc> static inline typename std::vector<T, Alloc>::iterator beginof (std::vector<T, Alloc>& v) { return v.begin(); }
template <typename T, typename Alloc> static inline typename std::vector<T, Alloc>::iterator endof (std::vector<T, Alloc>& v)   { return v.end(); }

template <typename T, typename Alloc> static inline typename std::vector<T, Alloc>::const_iterator beginof (const std::vector<T, Alloc>& v) { return v.begin(); }
template <typename T, typename Alloc> static inline typename std::vector<T, Alloc>::const_iterator endof (const std::vector<T, Alloc>& v)   { return v.end(); }

// foreach support for std::list
#include <list>
template <typename T, typename Alloc> static inline typename std::list<T, Alloc>::iterator beginof (std::list<T, Alloc>& v) { return v.begin(); }
template <typename T, typename Alloc> static inline typename std::list<T, Alloc>::iterator endof (std::list<T, Alloc>& v)   { return v.end(); }

template <typename T, typename Alloc> static inline typename std::list<T, Alloc>::const_iterator beginof (const std::list<T, Alloc>& v) { return v.begin(); }
template <typename T, typename Alloc> static inline typename std::list<T, Alloc>::const_iterator endof (const std::list<T, Alloc>& v)   { return v.end(); }

// foreach support for std::map
#include <map>
template <typename Key, typename T, typename Compare, typename Alloc> static inline typename std::map<Key, T, Compare, Alloc>::iterator beginof (std::map<Key, T, Compare, Alloc>& m) { return m.begin(); }
template <typename Key, typename T, typename Compare, typename Alloc> static inline typename std::map<Key, T, Compare, Alloc>::iterator endof (std::map<Key, T, Compare, Alloc>& m)   { return m.end(); }

template <typename Key, typename T, typename Compare, typename Alloc> static inline typename std::map<Key, T, Compare, Alloc>::const_iterator beginof (const std::map<Key, T, Compare, Alloc>& m) { return m.begin(); }
template <typename Key, typename T, typename Compare, typename Alloc> static inline typename std::map<Key, T, Compare, Alloc>::const_iterator endof (const std::map<Key, T, Compare, Alloc>& m)   { return m.end(); }

// foreach support for std::tr1::unordered_set
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <unordered_set>
template <typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::unordered_set<Value, Hash, Pred, Alloc>::iterator beginof (std::unordered_set<Value, Hash, Pred, Alloc>& m) { return m.begin(); }
template <typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::unordered_set<Value, Hash, Pred, Alloc>::iterator endof (std::unordered_set<Value, Hash, Pred, Alloc>& m)   { return m.end(); }

template <typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::unordered_set<Value, Hash, Pred, Alloc>::const_iterator beginof (const std::unordered_set<Value, Hash, Pred, Alloc>& m) { return m.begin(); }
template <typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::unordered_set<Value, Hash, Pred, Alloc>::const_iterator endof (const std::unordered_set<Value, Hash, Pred, Alloc>& m)   { return m.end(); }

#include <unordered_map>
template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::unordered_map<Key, Value, Hash, Pred, Alloc>::iterator beginof (std::unordered_map<Key, Value, Hash, Pred, Alloc>& m) { return m.begin(); }
template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::unordered_map<Key, Value, Hash, Pred, Alloc>::iterator endof (std::unordered_map<Key, Value, Hash, Pred, Alloc>& m)   { return m.end(); }

template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::unordered_map<Key, Value, Hash, Pred, Alloc>::const_iterator beginof (const std::unordered_map<Key, Value, Hash, Pred, Alloc>& m) { return m.begin(); }
template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::unordered_map<Key, Value, Hash, Pred, Alloc>::const_iterator endof (const std::unordered_map<Key, Value, Hash, Pred, Alloc>& m)   { return m.end(); }
#else
#include <tr1/unordered_set>
template <typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::tr1::unordered_set<Value, Hash, Pred, Alloc>::iterator beginof (std::tr1::unordered_set<Value, Hash, Pred, Alloc>& m) { return m.begin(); }
template <typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::tr1::unordered_set<Value, Hash, Pred, Alloc>::iterator endof (std::tr1::unordered_set<Value, Hash, Pred, Alloc>& m)   { return m.end(); }

template <typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::tr1::unordered_set<Value, Hash, Pred, Alloc>::const_iterator beginof (const std::tr1::unordered_set<Value, Hash, Pred, Alloc>& m) { return m.begin(); }
template <typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::tr1::unordered_set<Value, Hash, Pred, Alloc>::const_iterator endof (const std::tr1::unordered_set<Value, Hash, Pred, Alloc>& m)   { return m.end(); }

// foreach support for std::tr1::unordered_map
#include <tr1/unordered_map>
template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc>::iterator beginof (std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc>& m) { return m.begin(); }
template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc>::iterator endof (std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc>& m)   { return m.end(); }

template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc>::const_iterator beginof (const std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc>& m) { return m.begin(); }
template <typename Key, typename Value, typename Hash, typename Pred, typename Alloc> static inline typename std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc>::const_iterator endof (const std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc>& m)   { return m.end(); }
#endif

// foreach support for __gnu_cxx::hash_map
#include <ext/hash_map>
template <typename Key, typename Value, typename Hash> static inline typename __gnu_cxx::hash_map<Key, Value, Hash>::iterator beginof (__gnu_cxx::hash_map<Key, Value, Hash>& m) { return m.begin(); }
template <typename Key, typename Value, typename Hash> static inline typename __gnu_cxx::hash_map<Key, Value, Hash>::iterator endof (__gnu_cxx::hash_map<Key, Value, Hash>& m)   { return m.end(); }

template <typename Key, typename Value, typename Hash> static inline typename __gnu_cxx::hash_map<Key, Value, Hash>::const_iterator beginof (const __gnu_cxx::hash_map<Key, Value, Hash>& m) { return m.begin(); }
template <typename Key, typename Value, typename Hash> static inline typename __gnu_cxx::hash_map<Key, Value, Hash>::const_iterator endof (const __gnu_cxx::hash_map<Key, Value, Hash>& m)   { return m.end(); }

#define foreach(i, c) \
   for(decltype(beginof(c)) i = beginof(c); i != endof(c); ++i)

#endif /* ZILLIANS_TEMPLATETRICKS_H_ */
