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
#include <memory>
#include <string>
#include <algorithm>
#include <boost/regex/pending/unicode_iterator.hpp>

namespace zillians {

namespace detail {

struct disable_stdio_sync
{
	explicit disable_stdio_sync(bool sync) { previous_sync = std::ios::sync_with_stdio(sync); }
	~disable_stdio_sync() { std::ios::sync_with_stdio(previous_sync); }
	bool previous_sync;
};

}

std::locale& get_default_locale();
std::locale& get_posix_locale();
std::locale& get_utf8_locale();
std::locale& get_c_locale();

template<typename StreamT>
void enable_default_locale(StreamT& stream)
{
	static detail::disable_stdio_sync s(false);
	stream.imbue(get_default_locale());
}

template<typename StreamT>
void enable_posix_locale(StreamT& stream)
{
	static detail::disable_stdio_sync s(false);
	stream.imbue(get_posix_locale());
}

template<typename StreamT>
void enable_utf8_locale(StreamT& stream)
{
	static detail::disable_stdio_sync s(false);
	stream.imbue(get_utf8_locale());
}

template<typename StreamT>
void enable_c_locale(StreamT& stream)
{
	static detail::disable_stdio_sync s(false);
	stream.imbue(get_c_locale());
}

void utf8_to_ucs4(const std::string& input, std::wstring& output);

}

#endif /* ZILLIANS_UNICODEUTIL_H_ */
