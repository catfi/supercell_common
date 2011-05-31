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

#include "core/Prerequisite.h"
#include "core/BinaryCast.h"
#include <iostream>
#include <string>
#include <limits>

#define BOOST_TEST_MODULE BinaryCastTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( BinaryCastTest )

BOOST_AUTO_TEST_CASE( BinaryCastTestCase1 )
{
	// 1 bytes
	{
		int8 a = std::numeric_limits<int8>::max();
		int8 b = binary_cast<int8>(binary_cast<uint8>(a));
		BOOST_CHECK(a == b);
	}

	{
		uint8 a = std::numeric_limits<uint8>::max();
		uint8 b = binary_cast<uint8>(binary_cast<int8>(a));
		BOOST_CHECK(a == b);
	}

	// 2 bytes
	{
		int16 a = std::numeric_limits<int16>::max();
		int16 b = binary_cast<int16>(binary_cast<uint16>(a));
		BOOST_CHECK(a == b);
	}

	{
		uint16 a = std::numeric_limits<uint16>::max();
		uint16 b = binary_cast<uint16>(binary_cast<int16>(a));
		BOOST_CHECK(a == b);
	}

	// 4 bytes
	{
		int32 a = std::numeric_limits<int32>::max();
		int32 b = binary_cast<int32>(binary_cast<uint32>(a));
		BOOST_CHECK(a == b);
	}

	{
		uint32 a = std::numeric_limits<uint32>::max();
		uint32 b = binary_cast<uint32>(binary_cast<int32>(a));
		BOOST_CHECK(a == b);
	}

	{
		float a = std::numeric_limits<float>::max() * 0.3f;
		float b = binary_cast<float>(binary_cast<int32>(a));
		BOOST_CHECK(a == b);
	}

	{
		float a = std::numeric_limits<float>::max() * 0.3f;
		float b = binary_cast<float>(binary_cast<uint32>(a));
		BOOST_CHECK(a == b);
	}

	{
		int32 a = std::numeric_limits<int32>::max();
		int32 b = binary_cast<int32>(binary_cast<float>(a));
		BOOST_CHECK(a == b);
	}

	{
		uint32 a = std::numeric_limits<uint32>::max();
		uint32 b = binary_cast<uint32>(binary_cast<float>(a));
		BOOST_CHECK(a == b);
	}

	// 8 bytes
	{
		int64 a = std::numeric_limits<int64>::max();
		int64 b = binary_cast<int64>(binary_cast<double>(a));
		BOOST_CHECK(a == b);
	}

	{
		uint64 a = std::numeric_limits<uint64>::max();
		uint64 b = binary_cast<uint64>(binary_cast<double>(a));
		BOOST_CHECK(a == b);
	}

	{
		int64 a = std::numeric_limits<int64>::max();
		int64 b = binary_cast<int64>(binary_cast<unsigned long long int>(a));
		BOOST_CHECK(a == b);
	}

	{
		uint64 a = std::numeric_limits<uint64>::max();
		uint64 b = binary_cast<uint64>(binary_cast<unsigned long long int>(a));
		BOOST_CHECK(a == b);
	}

	{
		int64 a = std::numeric_limits<int64>::max();
		int64 b = binary_cast<int64>(binary_cast<long long int>(a));
		BOOST_CHECK(a == b);
	}

	{
		uint64 a = std::numeric_limits<uint64>::max();
		uint64 b = binary_cast<uint64>(binary_cast<long long int>(a));
		BOOST_CHECK(a == b);
	}

	{
		double a = std::numeric_limits<double>::max() * 0.3;
		double b = binary_cast<double>(binary_cast<long long int>(a));
		BOOST_CHECK(a == b);
	}

	{
		double a = std::numeric_limits<double>::max() * 0.3;
		double b = binary_cast<double>(binary_cast<long long int>(a));
		BOOST_CHECK(a == b);
	}
}

BOOST_AUTO_TEST_SUITE_END()
