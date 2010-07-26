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

#include "utility/StringUtil.h"

namespace zillians {

//////////////////////////////////////////////////////////////////////////

const char* StringUtil::itoaAlphabet[] = { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
									 "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
									 "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
									 "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
									 "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
									 "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
									 "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
									 "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
									 "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
									 "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
									 "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
									 "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
									 "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
									 "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
									 "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
									 "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
								   };

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

/*
std::size_t hash_value(const std::string& __x)
{
	return StringHasher::hash(__x);
}

std::size_t hash_value(const std::wstring& __x)
{
	return WStringHasher::hash(__x);
}
*/
}
