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



}

#endif/*ZILLIANS_STRINGUTIL_H_*/
