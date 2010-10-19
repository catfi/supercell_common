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
 * @date Oct 19, 2010 Jerry - Initial version created.
 */

#ifndef ZILLIANS_UTF8_H_
#define ZILLIANS_UTF8_H_

#include <unicode/ustring.h>
#include <vector>
#include <stdexcept>

namespace zillians {

// http://stackoverflow.com/questions/835453/c-unicode-question

/**
 * Converts a std::wstring into a std::string with UTF-8 encoding
 */
template<typename T>
T utf8(const std::wstring& rc_string);

/**
 * Converts a std::String with UTF-8 encoding into a std::wstring
 */
template<typename T>
T utf8(const std::string& rc_string);

}

#endif
