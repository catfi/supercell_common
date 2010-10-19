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

#include "utility/utf8.h"

namespace zillians {

/**
 * Nop specialization for std::string
 */
template<>
std::string utf8<std::string>(const std::string& rc_string)
{
  return rc_string;
}

/**
 * Nop specialization for std::wstring
 */
template<>
std::wstring utf8<std::wstring>(const std::wstring& rc_string)
{
  return rc_string;
}

template<>
std::string utf8<std::string>(const std::wstring& rc_string)
{
	std::string result;
	if(rc_string.empty())
		return result;

	std::vector<UChar> buffer;

	result.resize(rc_string.size() * 3); // UTF-8  uses max 3 bytes per char
	buffer.resize(rc_string.size() * 2); // UTF-16 uses max 2 bytes per char

	UErrorCode status = U_ZERO_ERROR;
	int32_t len = 0;

	u_strFromWCS(
		&buffer[0],
		buffer.size(),
		&len,
		&rc_string[0],
		rc_string.size(),
		&status);
	if(!U_SUCCESS(status))
	{
		throw std::runtime_error("utf8: u_strFromWCS failed");
	}
	buffer.resize(len);

	u_strToUTF8(
		&result[0],
		result.size(),
		&len,
		&buffer[0],
		buffer.size(),
		&status);
	if(!U_SUCCESS(status))
	{
		throw std::runtime_error("utf8: u_strToUTF8 failed");
	}
	result.resize(len);

	return result;
}


template<>
std::wstring utf8<std::wstring>(const std::string& rc_string)
{
	std::wstring result;
	if(rc_string.empty())
		return result;

	std::vector<UChar> buffer;

	result.resize(rc_string.size());
	buffer.resize(rc_string.size());

	UErrorCode status = U_ZERO_ERROR;
	int32_t len = 0;

	u_strFromUTF8(
		&buffer[0],
		buffer.size(),
		&len,
		&rc_string[0],
		rc_string.size(),
		&status
		);
	if(!U_SUCCESS(status))
	{
		throw std::runtime_error("utf8: u_strFromUTF8 failed");
	}
	buffer.resize(len);

	u_strToWCS(
		&result[0],
		result.size(),
		&len,
		&buffer[0],
		buffer.size(),
		&status
		);
	if(!U_SUCCESS(status))
	{
		throw std::runtime_error("utf8: u_strToWCS failed");
	}
	result.resize(len);

	return result;
}

}
