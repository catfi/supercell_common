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

	static bool substitute(std::string &s, const std::string &to_search, const std::string &to_replace);

	template<typename ForwardIterator> inline static ForwardIterator tolower(ForwardIterator first, ForwardIterator last, const std::locale& locale_ref = std::locale())
	{
//		using namespace boost;
//		using namespace std;

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
//		using namespace boost;
//		using namespace std;

		typedef typename std::iterator_traits<ForwardIterator>::value_type value_type;

		return std::transform(first, last, first, bind(&std::toupper<value_type>, _1, boost::cref(locale_ref)));
	}

	template < typename CharT, typename Traits, typename Alloc> inline static std::basic_string<CharT, Traits, Alloc>& toupper(std::basic_string<CharT, Traits, Alloc>& s, const std::locale& locale_ref = std::locale())
	{
		StringUtil::toupper(s.begin(), s.end(), locale_ref);
		return s;
	}

	template<typename T>
	static void itoa(T t, std::string& s)
	{
		unsigned char index;
		for(int i = sizeof(T)-1; i >= 0; --i)
		{
			index = (t >> (i*8)) & 0xFF;
			s.append(itoaAlphabet[index]);
		}
	}

	static std::wstring toWStr_ascii(std::string s)
	{
		wchar_t *wBuf = new wchar_t[s.length()+1];
		for(size_t i = 0; i<s.length(); i++)
			wBuf[i] = static_cast<wchar_t>(s[i]);
		std::wstring Ret(wBuf);
		delete wBuf;
		return Ret;
	}

	static std::string ws2s_ascii(std::wstring s)
	{
		char *buf = new char[s.length()+1];
		for(size_t i = 0; i<s.length(); i++)
			buf[i] = static_cast<char>(s[i]);
		std::string Ret(buf);
		delete buf;
		return Ret;
	}

	static std::string wstrToUtf8(const std::wstring& wstr)
	{
		std::string ret("");
		wchar_t wc = 0;
		for(std::size_t i = 0; i < wstr.size(); ++i)
		{
			wc = wstr[i];
			if(wc < 0x80)
			{
				ret += (char)wc;
			}
			else if(wc < 0x800)
			{
				ret += (0xc0 | (wc >> 6));	// 110xxxxx
				ret += (0x80 | (wc & 0x3f));	// 10xxxxxx
			}
			else if(wc < 0x10000)
			{
				ret += (0xe0 | (wc >> 12));			// 1110xxxx
				ret += (0x80 | ((wc >> 6) & 0x3f));	// 10xxxxxx
				ret += (0x80 | (wc & 0x3f));		// 10xxxxxx
			}
#if !defined(WIN32)// Following codeprints does not exist in UCS-2
			else if(wc < 0x20000)
			{
				ret += (0xf0 | (wc >> 18));			// 11110xxx
				ret += (0x80 | ((wc >> 12) & 0x3f));// 10xxxxxx
				ret += (0x80 | ((wc >> 6) & 0x3f));	// 10xxxxxx
				ret += (0x80 | (wc & 0x3f));		// 10xxxxxx
			}
			else if(wc < 0x4000000)
			{
				ret += (0xf8 | (wc >> 24));			// 111110xx
				ret += (0x80 | ((wc >> 18) & 0x3f));// 10xxxxxx
				ret += (0x80 | ((wc >> 12) & 0x3f));// 10xxxxxx
				ret += (0x80 | ((wc >> 6) & 0x3f));	// 10xxxxxx
				ret += (0x80 | (wc & 0x3f));		// 10xxxxxx
			}
			else if(wc < 0x8000000)
			{
				ret += (0xfc | (wc >> 30));			// 1111110x
				ret += (0x80 | ((wc >> 24) & 0x3f));// 10xxxxxx
				ret += (0x80 | ((wc >> 18) & 0x3f));// 10xxxxxx
				ret += (0x80 | ((wc >> 12) & 0x3f));// 10xxxxxx
				ret += (0x80 | ((wc >> 6) & 0x3f));	// 10xxxxxx
				ret += (0x80 | (wc & 0x3f));		// 10xxxxxx
			}
#endif
		}
		return ret;
	}//wstrToUtf8()

	
	static std::wstring utf8ToWstr(const std::string& str)
	{
		std::wstring ret(L"");
		wchar_t wc = 0;
		int tailing = 0;
		for(std::size_t i = 0; i < str.size(); ++i)
		{
			if(tailing > 0)
			{
				wc = ((wc << 6) | ((wchar_t)(str[i] & 0x3f)));
				--tailing;
				if(tailing == 0) { ret += wc; wc = 0; }
			}
			else if((str[i] & 0x80) == 0x80)// control byte
			{
				if((str[i] & 0xe0) == 0xc)// 0x80 - 0x7FF unicode  2bytes 110xxxxx 10xxxxxx
				{
					tailing = 1;
					wc = ((wchar_t)str[i]) & 0x1f;
				}
				else if((str[i] & 0xf0) == 0xe0)// 0x800 - 0xFFFF unicode  3bytes 1110xxxx 10xxxxxx 10xxxxxx
				{
					tailing = 2;
					wc = ((wchar_t)str[i]) & 0x0f;
				}
				else if((str[i] & 0xf8) == 0xf0)// 0x10000 - 0x1FFFF unicode  4bytes 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
				{
#if !defined(WIN32)
					tailing = 3;
					wc = ((wchar_t)str[i]) & 0x07;
#else
					i += 3;
					ret += L'?';// Cannot display codeprints outside UCS-2
					wc = 0;
#endif
				}
				else if((str[i] & 0xfc) == 0xf8)// 0x20000 - 0x3FFFFFF unicode  5bytes 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
				{
#if !defined(WIN32)
					tailing = 4;
					wc = ((wchar_t)str[i]) & 0x3;
#else
					i += 4;
					ret += L'?';// Cannot display codeprints outside UCS-2
					wc = 0;
#endif
				}
				else if((str[i] & 0xfe) == 0xfd)// 0x4000000 - 0x7FFFFFF unicode  6bytes 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
				{
#if !defined(WIN32)
					tailing = 5;
					wc = ((wchar_t)str[i]) & 0x1f;
#else
					i += 5;
					ret += L'?';// Cannot display codeprints outside UCS-2
					wc = 0;
#endif
				}
			}
			else
			{
				tailing = 0;
				ret += (wchar_t)str[i];
			}
		}
		return ret;

	}//utf8ToWstr()

	
private:
	static const char* itoaAlphabet [256];
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
