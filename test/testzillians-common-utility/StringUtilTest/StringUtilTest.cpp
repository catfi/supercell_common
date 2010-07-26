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

#include "core/Prerequisite.h"
#include "utility/StringUtil.h"
#include <tr1/unordered_set>

#define BOOST_TEST_MODULE StringUtilTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace zillians;

/**
 * Print the tokenized result
 * @param a
 */
template <typename T>
void print_result(const vector<T> &a)
{
	int i=0;
	for(typename vector<T>::const_iterator x = a.begin(); x != a.end(); ++x, ++i)
	{
		printf("[%d] %s\n", i, x->c_str());
	}
}

/**
 * Compare two string vectors
 * @param a first string vector
 * @param b second string vector
 * @return true if equal, false otherwise
 */
template <typename T>
bool compare_result(const vector<T> &a, const vector<T> &b)
{
	if(a.size() != b.size()) return false;

	typename vector<T>::const_iterator x = a.begin();
	typename vector<T>::const_iterator y = b.begin();

	while(x != a.end() && y != b.end())
	{
		if((*x).compare(*y) != 0)
			return false;
		++x; ++y;
	}

	return true;
}

BOOST_AUTO_TEST_SUITE( StringUtilTestSuite )

BOOST_AUTO_TEST_CASE( TokenizerCase1 )
{
	string input = "a.b.c.d";

	vector<string> output;
	output.push_back("a");
	output.push_back("b");
	output.push_back("c");
	output.push_back("d");

	vector<string> result = StringUtil::tokenize(input, ".");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( TokenizerCase2 )
{
	string input = "a..b...c...d";

	vector<string> output;
	output.push_back("a");
	output.push_back("b");
	output.push_back("c");
	output.push_back("d");

	vector<string> result = StringUtil::tokenize(input, ".");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( TokenizerCase3 )
{
	string input = "a.,,,.b.,,,c,,d";

	vector<string> output;
	output.push_back("a");
	output.push_back("b");
	output.push_back("c");
	output.push_back("d");

	vector<string> result = StringUtil::tokenize(input, ".,");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( TokenizerCase4 )
{
	string input = "..,.";

	vector<string> output;

	vector<string> result = StringUtil::tokenize(input, ".,");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( TokenizerCase5 )
{
	string input = "...A....";

	vector<string> output;
	output.push_back("A");

	vector<string> result = StringUtil::tokenize(input, ".");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( TokenizerCase6 )
{
	string input = "A..B.";

	vector<string> output;
	output.push_back("A");
	output.push_back("");
	output.push_back("B");
	output.push_back("");

	vector<string> result = StringUtil::tokenize(input, ".", true);
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( TokenizerCase7 )
{
	string input = ".";

	vector<string> output;
	output.push_back("");
	output.push_back("");

	vector<string> result = StringUtil::tokenize(input, ".", true);
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( TokenizerCase8 )
{
	string input = "ABCDEFG.HIJKLMN.";

	vector<string> output;
	output.push_back("ABCDEFG");
	output.push_back("HIJKLMN");
	output.push_back("");

	vector<string> result = StringUtil::tokenize(input, ".", true);
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( TokenizerCase9 )
{
	string input = "ABCDEFG;HIJKLMN;";

	vector<string> output;
	output.push_back("ABCDEFG");
	output.push_back("HIJKLMN");

	vector<string> result = StringUtil::tokenize(input, ";", false);
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( UpperLowerCaseConversionCase1 )
{
	string s0 = "abcDEFghijk";
	string s1 = "AbcDeFGhiJk";

	string s0_l = StringUtil::tolower(s0);
	string s1_l = StringUtil::tolower(s1);

	BOOST_CHECK(s0_l.compare(s1_l) == 0);

	string s0_u = StringUtil::toupper(s0);
	string s1_u = StringUtil::toupper(s1);

	BOOST_CHECK(s0_u.compare(s1_u) == 0);
}




BOOST_AUTO_TEST_CASE( WideTokenizerCase1 )
{
	wstring input = L"a.b.c.d";

	vector<wstring> output;
	output.push_back(L"a");
	output.push_back(L"b");
	output.push_back(L"c");
	output.push_back(L"d");

	vector<wstring> result = StringUtil::tokenize(input, L".");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( WideTokenizerCase2 )
{
	wstring input = L"a..b...c...d";

	vector<wstring> output;
	output.push_back(L"a");
	output.push_back(L"b");
	output.push_back(L"c");
	output.push_back(L"d");

	vector<wstring> result = StringUtil::tokenize(input, L".");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( WideTokenizerCase3 )
{
	wstring input = L"a.,,,.b.,,,c,,d";

	vector<wstring> output;
	output.push_back(L"a");
	output.push_back(L"b");
	output.push_back(L"c");
	output.push_back(L"d");

	vector<wstring> result = StringUtil::tokenize(input, L".,");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( WideTokenizerCase4 )
{
	wstring input = L"..,.";

	vector<wstring> output;

	vector<wstring> result = StringUtil::tokenize(input, L".,");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( WideTokenizerCase5 )
{
	wstring input = L"...A....";

	vector<wstring> output;
	output.push_back(L"A");

	vector<wstring> result = StringUtil::tokenize(input, L".");
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( WideTokenizerCase6 )
{
	wstring input = L"A..B.";

	vector<wstring> output;
	output.push_back(L"A");
	output.push_back(L"");
	output.push_back(L"B");
	output.push_back(L"");

	vector<wstring> result = StringUtil::tokenize(input, L".", true);
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( WideTokenizerCase7 )
{
	wstring input = L".";

	vector<wstring> output;
	output.push_back(L"");
	output.push_back(L"");

	vector<wstring> result = StringUtil::tokenize(input, L".", true);
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( WideTokenizerCase8 )
{
	wstring input = L"ABCDEFG.HIJKLMN.";

	vector<wstring> output;
	output.push_back(L"ABCDEFG");
	output.push_back(L"HIJKLMN");
	output.push_back(L"");

	vector<wstring> result = StringUtil::tokenize(input, L".", true);
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( WideTokenizerCase9 )
{
	wstring input = L"ABCDEFG;HIJKLMN;";

	vector<wstring> output;
	output.push_back(L"ABCDEFG");
	output.push_back(L"HIJKLMN");

	vector<wstring> result = StringUtil::tokenize(input, L";", false);
	//printf("result => \n");
	//print_result(result);

	BOOST_CHECK(compare_result(output, result));
}

BOOST_AUTO_TEST_CASE( StringHashCase1 )
{
	string value1 = "ABCDEFG";
	string value2 = "ABCDEFGH";

	std::tr1::unordered_set<std::string> str_hash;

	str_hash.insert(value1);
	str_hash.insert(value2);

	BOOST_CHECK(str_hash.count(value1) == 1);
	BOOST_CHECK(str_hash.count(value2) == 1);
}

BOOST_AUTO_TEST_CASE( WStringHashCase1 )
{
	wstring value1 = L"ABCDEFG";
	wstring value2 = L"ABCDEFGH";

	std::tr1::unordered_set<std::wstring> str_hash;

	str_hash.insert(value1);
	str_hash.insert(value2);

	BOOST_CHECK(str_hash.count(value1) == 1);
	BOOST_CHECK(str_hash.count(value2) == 1);
}

BOOST_AUTO_TEST_CASE( atoiCase1 )
{
	uint32 value1 = 0x12AB;
	std::string value2;

	StringUtil::itoa(value1, value2);

	BOOST_CHECK(value2 == "000012AB");
	value2.clear();

	int32 value3 = -0xAB12;

	StringUtil::itoa(value3, value2);
}

BOOST_AUTO_TEST_SUITE_END()
