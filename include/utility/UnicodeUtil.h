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
 * @date Jul 11, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_UNICODEUTIL_H_
#define ZILLIANS_UNICODEUTIL_H_

#include <iostream>
#include <fstream>
#include <locale>

#define BOOST_UTF8_BEGIN_NAMESPACE namespace zillians {
#define BOOST_UTF8_END_NAMESPACE }
#define BOOST_UTF8_DECL

#include <boost/detail/utf8_codecvt_facet.hpp>
#include "utility/detail/boost/utf8_codecvt_facet.cpp"

namespace zillians {

namespace detail {

struct disable_stdio_sync
{
	explicit disable_stdio_sync(bool sync) { previous_sync = std::ios::sync_with_stdio(sync); }
	~disable_stdio_sync() { std::ios::sync_with_stdio(previous_sync); }
	bool previous_sync;
};

}

template<typename StreamT>
void enable_default_locale(StreamT& stream)
{
	static detail::disable_stdio_sync s(false);
	std::locale default_locale("");
	stream.imbue(default_locale);
}

template<typename StreamT>
void enable_utf8_locale(StreamT& stream)
{
	static detail::disable_stdio_sync s(false);
	static utf8_codecvt_facet* utf8_facet = new utf8_codecvt_facet;
	std::locale utf8_locale(std::locale(), utf8_facet);
	stream.imbue(utf8_locale);
}

}
#endif /* ZILLIANS_UNICODEUTIL_H_ */
