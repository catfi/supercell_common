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

#ifndef ZILLIANS_SHAREDPTR_H_
#define ZILLIANS_SHAREDPTR_H_

#include "core/Common.h"
#include <hash_set>

#if defined _WIN32
	#include <boost/shared_ptr.hpp>
	#include <boost/weak_ptr.hpp>
	#include <boost/enable_shared_from_this.hpp>
	#include <boost/make_shared.hpp>
	using boost::static_pointer_cast;
	using boost::const_pointer_cast;
	using boost::dynamic_pointer_cast;
	using boost::shared_ptr;
	using boost::weak_ptr;
	using boost::enable_shared_from_this;
	using boost::make_shared;
#else
	#ifndef __GXX_EXPERIMENTAL_CXX0X__
		#include <boost/shared_ptr.hpp>
		#include <boost/weak_ptr.hpp>
		#include <boost/enable_shared_from_this.hpp>
		#include <boost/make_shared.hpp>
		using boost::static_pointer_cast;
		using boost::const_pointer_cast;
		using boost::dynamic_pointer_cast;
		using boost::shared_ptr;
		using boost::weak_ptr;
		using boost::enable_shared_from_this;
		using boost::make_shared;
	#else
		#include <memory>
		using std::static_pointer_cast;
		using std::const_pointer_cast;
		using std::dynamic_pointer_cast;
		using std::shared_ptr;
		using std::weak_ptr;
		using std::enable_shared_from_this;
		using std::make_shared;
	#endif
#endif

namespace zillians {

struct null_deleter
{
	void operator()(void const *) const
	{ }
};

template<typename T>
struct reference_holder
{
	reference_holder(const shared_ptr<T>& obj) : ref(obj) { }
	void operator()(void const*) const
	{ }
	const shared_ptr<T> ref;
};

template<typename A, typename B>
shared_ptr<B> reinterpret_pointer_cast(const shared_ptr<A>& obj)
{
	return shared_ptr<B>((B*)obj.get(), reference_holder<A>(obj));
}

}

/**
 * @brief Allow direct comparison of shared_ptr and its wrapping type (for "equal to" operator)
 */
template<class T>
inline bool operator == (const shared_ptr<T> &a, const T* b)
{
    return (a.get() == b);
}

/**
 * @brief Allow direct comparison of shared_ptr and its wrapping type (for "not equal to" operator)
 */
template<class T>
inline bool operator != (const shared_ptr<T> &a, const T* b)
{
    return (a.get() != b);
}


#ifdef __PLATFORM_LINUX__
namespace __gnu_cxx
{
	template<typename T>
	struct hash<shared_ptr<T> >
	{
		size_t operator()(const shared_ptr<T>& __x) const
		{
			return reinterpret_cast<size_t>(__x.get());
		}
	};
}
#else
namespace stdext
{
	template<typename T>
	struct hash_compare<shared_ptr<T> >
	{
		size_t operator()(const shared_ptr<T>& __x) const
		{
			return reinterpret_cast<size_t>(__x.get());
		}

		bool operator()(const shared_ptr<T>& v1, const shared_ptr<T>& v2) const
		{
			return (v1 == v2) ? true : false;
		}
	};
}
#endif

#endif/*ZILLIANS_SHAREDPTR_H_*/
