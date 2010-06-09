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

#ifndef ZILLIANS_STRINGUTIL_H_
#define ZILLIANS_STRINGUTIL_H_

#include <boost/bind.hpp> // bind
#include <locale>         // tolower
#include <algorithm>      // transform
#include <string>
#include <vector>

namespace zillians {

class StringUtil
{
private:
	StringUtil();
	~StringUtil();

public:
	static std::vector<std::string> tokenize(const std::string& str, const std::string& delimiters, bool allowEmptyTokenString = false);
	static std::vector<std::wstring> tokenize(const std::wstring& str, const std::wstring& delimiters, bool allowEmptyTokenString = false);

	template<typename ForwardIterator> inline static ForwardIterator tolower(ForwardIterator first, ForwardIterator last, const std::locale& locale_ref = std::locale())
	{
		using namespace boost;
		using namespace std;

		typedef	typename std::iterator_traits<ForwardIterator>::value_type value_type;

		return std::transform(first, last, first, bind(&std::tolower<value_type>, _1, boost::cref(locale_ref)));
	}

	template < typename CharT, typename Traits, typename Alloc> inline static std::basic_string<CharT, Traits, Alloc>& tolower(std::basic_string<CharT, Traits, Alloc>& s, const std::locale& locale_ref = std::locale())
	{
		StringUtil::tolower(s.begin(), s.end(), locale_ref);
		return s;
	}

	template < typename ForwardIterator> inline static ForwardIterator toupper(ForwardIterator first, ForwardIterator last, const std::locale& locale_ref = std::locale())
	{
		using namespace boost;
		using namespace std;

		typedef typename std::iterator_traits<ForwardIterator>::value_type value_type;

		return std::transform(first, last, first, bind(&std::toupper<value_type>, _1, boost::cref(locale_ref)));
	}

	template < typename CharT, typename Traits, typename Alloc> inline static std::basic_string<CharT, Traits, Alloc>& toupper(std::basic_string<CharT, Traits, Alloc>& s, const std::locale& locale_ref = std::locale())
	{
		StringUtil::toupper(s.begin(), s.end(), locale_ref);
		return s;
	}
};

/*
struct StringHasher
{
	inline static size_t hash(const std::string& str)
	{
		size_t s;
		const char* data = str.data();
		for(size_t i = 0; i < str.length(); ++i)
		{
			s += data[i];
		}
		return s;
	}

	inline static bool equal( const std::string& x, const std::string& y )
    {
    	return (x == y);
    }
};

extern std::size_t hash_value(const std::string& __x);

#ifdef __PLATFORM_LINUX__
namespace __gnu_cxx
{
	template<>
	struct hash<std::string>
	{
		size_t operator()(const std::string& __x) const
		{
			return StringHasher::hash(__x);
		}
	};
}
#else
namespace stdext
{
	template<>
	struct hash_compare<std::string>
	{
		size_t operator()(const std::string& __x) const
		{
			return StringHasher::hash(__x);
		}

		bool operator()(const std::string& v1, const std::string& v2) const
		{
			return (v1 == v2) ? true : false;
		}
	};
}
#endif


struct WStringHasher
{
	inline static size_t hash(const std::wstring& str)
	{
		size_t s;
		const wchar_t* data = str.data();
		for(size_t i = 0; i < str.length(); ++i)
		{
			s += data[i];
		}
		return s;
	}

	inline static bool equal( const std::wstring& x, const std::wstring& y )
    {
    	return (x == y);
    }
};

extern std::size_t hash_value(const std::wstring& __x);

#ifdef __PLATFORM_LINUX__
namespace __gnu_cxx
{
	template<>
	struct hash<std::wstring>
	{
		size_t operator()(const std::wstring& __x) const
		{
			return WStringHasher::hash(__x);
		}
	};
}
#else
namespace stdext
{
	template<>
	struct hash_compare<std::wstring>
	{
		size_t operator()(const std::wstring& __x) const
		{
			return WStringHasher::hash(__x);
		}

		bool operator()(const std::wstring& v1, const std::wstring& v2) const
		{
			return (v1 == v2) ? true : false;
		}
	};
}
#endif

*/
}

#endif/*ZILLIANS_STRINGUTIL_H_*/
