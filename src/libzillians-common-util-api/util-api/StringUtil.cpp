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

#include "util-api/StringUtil.h"

namespace zillians {

//////////////////////////////////////////////////////////////////////////
StringUtil::StringUtil()
{ }

StringUtil::~StringUtil()
{ }

//////////////////////////////////////////////////////////////////////////
std::vector<std::string> StringUtil::tokenize(const std::string& str, const std::string& delimiters, bool allowEmptyTokenString)
{
	std::vector<std::string> tokens;
	std::string::size_type delimPos = 0, tokenPos = 0, pos = 0;

	if (str.length() < 1)
		return tokens;

	while (true)
	{
		delimPos = str.find_first_of(delimiters, pos);
		tokenPos = str.find_first_not_of(delimiters, pos);

		if (std::string::npos != delimPos)
		{
			if (std::string::npos != tokenPos)
			{
				if (tokenPos < delimPos)
				{
					tokens.push_back(str.substr(pos, delimPos - pos));
				}
				else
				{
					if (allowEmptyTokenString)	tokens.push_back("");
				}
			}
			else
			{
				if (allowEmptyTokenString) tokens.push_back("");
			}
			pos = delimPos + 1;
		}
		else
		{
			if (std::string::npos != tokenPos)
			{
				tokens.push_back(str.substr(pos));
			}
			else
			{
				if (allowEmptyTokenString) tokens.push_back("");
			}
			break;
		}
	}
	return tokens;
}



std::vector<std::wstring> StringUtil::tokenize(const std::wstring& str, const std::wstring& delimiters, bool allowEmptyTokenString)
{
	std::vector<std::wstring> tokens;
	std::wstring::size_type delimPos = 0, tokenPos = 0, pos = 0;

	if (str.length() < 1)
		return tokens;

	while (true)
	{
		delimPos = str.find_first_of(delimiters, pos);
		tokenPos = str.find_first_not_of(delimiters, pos);

		if (std::wstring::npos != delimPos)
		{
			if (std::wstring::npos != tokenPos)
			{
				if (tokenPos < delimPos)
				{
					tokens.push_back(str.substr(pos, delimPos - pos));
				}
				else
				{
					if (allowEmptyTokenString)	tokens.push_back(L"");
				}
			}
			else
			{
				if (allowEmptyTokenString) tokens.push_back(L"");
			}
			pos = delimPos + 1;
		}
		else
		{
			if (std::wstring::npos != tokenPos)
			{
				tokens.push_back(str.substr(pos));
			}
			else
			{
				if (allowEmptyTokenString) tokens.push_back(L"");
			}
			break;
		}
	}
	return tokens;
}// tokenize(wstring)

}
