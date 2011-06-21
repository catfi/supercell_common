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
 * @date May 3, 2009 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "utility/UUIDUtil.h"
#include "utility/StringUtil.h"

#define BOOST_TEST_MODULE UUIDUtilTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace zillians;

BOOST_AUTO_TEST_SUITE( UUIDUtilTestSuite )

BOOST_AUTO_TEST_CASE( UUIDCase1 )
{
	std::string value = "1a03c570-379a-11de-9b9a-001d92648305";

	// test copy constructor (from string)
	UUID id0 = value;
	BOOST_CHECK(value.compare((std::string)id0) == 0);

	// test copy constructor (from another UUID)
	UUID id1 = id0;
	BOOST_CHECK(value.compare((std::string)id1) == 0);
}

BOOST_AUTO_TEST_CASE( UUIDCase2 )
{
	// test random UUID
	for(int i=0;i<1000;++i)
	{
		UUID id0; UUID::random(id0);
		UUID id1; UUID::random(id1);
		BOOST_CHECK_MESSAGE(id0 != id1, "fail to check at index " << i << ", id0 = " << id0 << ", id1 = " << id1);
	}
}

BOOST_AUTO_TEST_CASE( UUIDCase3 )
{
	// test invalidate UUID
	UUID id0; UUID::invalidate(id0);
	UUID id1; UUID::invalidate(id1);
	BOOST_CHECK(id0 == id1);

	// test invalidate string
	std::string value = "00000000-0000-0000-0000-000000000000";
	id0 = value;
	BOOST_CHECK(id0 == id1);

	// test valid/invalid flag
	BOOST_CHECK(id0.invalid());
	BOOST_CHECK(!id0.valid());
}

BOOST_AUTO_TEST_CASE( UUIDCase4 )
{
	// test UUID comparison
	for(int i=0;i<1000;++i)
	{
		UUID id0; UUID::random(id0);
		UUID id1; UUID::random(id1);
		BOOST_CHECK(id0 > id1 || id0 < id1);
	}
}

BOOST_AUTO_TEST_CASE( UUIDCase5 )
{
	// test iostream input & output
	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	UUID id0; UUID::random(id0);
	UUID id1;
	ss << id0;
	ss >> id1;
	BOOST_CHECK(id0 == id1);
}

BOOST_AUTO_TEST_CASE( UUIDCase6 )
{
	// test std::string conversion
	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	UUID id0; UUID::random(id0);
	UUID id1;
	ss << (std::string)id0;
	ss >> id1;
	BOOST_CHECK(id0 == id1);
}

BOOST_AUTO_TEST_CASE( UUIDCase7 )
{
	// test std::string conversion
	std::string value = "1A03C570-379A-11DE-9B9A-001D92648305";

	// test string comparison (case insensitive comparison)
	UUID id0 = value;
	BOOST_CHECK(id0 == value);
}

BOOST_AUTO_TEST_CASE( UUIDCase8 )
{
	std::string value = "5a1e115437bf11deb7fc001d92648382";

	// test invalid string comparison (case insensitive comparison)
	UUID id0 = value;
	BOOST_CHECK(id0 != value);
}

BOOST_AUTO_TEST_CASE( UUIDCase9 )
{
	std::string value = "ef5eebfa-37b8-11de-9d60-001d92648305";

	// test special UUID
	UUID id0 = value;
	UUID id1 = value;
	BOOST_CHECK(id0 == id1);
}

BOOST_AUTO_TEST_CASE( UUIDCase10 )
{
	std::string value0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	std::string value1 = "eefe4c0a-37b8-11de-aa06-001d92648305";

	// test the comparison (must be smaller or greater or equal)
	UUID id0 = value0;
	UUID id1 = value1;
	BOOST_CHECK( (id0 > id1 && !(id0<id1) && !(id0==id1)) || !(id0 > id1 && (id0<id1) && !(id0==id1)) || !(id0 > id1 && !(id0<id1) && (id0==id1)) );
}

BOOST_AUTO_TEST_CASE( UUIDCase11 )
{
	UUID id0;
	UUID id1;

	// test the comparison
	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-001d92648306";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-001d92648315";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-001d92648405";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-001d92649305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-001d92658305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-001d92748305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-001d93648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-001e02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-002d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-011d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d60-101d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d61-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d61-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9d70-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-9e60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11de-ad60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11df-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-11ee-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-12de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b8-21de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37b9-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-37c8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-38b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfa-47b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eebfb-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5eec0a-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5efbfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef5febfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "ef6eebfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648305";
	id1 = "f05eebfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648306";
	id1 = "ff5eebfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648315";
	id1 = "ee5eebfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92648405";
	id1 = "ef6eebfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92649305";
	id1 = "ef5febfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92658305";
	id1 = "ef5efbfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d92748305";
	id1 = "ef5eecfa-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001d93648305";
	id1 = "ef5eec0a-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001da2648305";
	id1 = "ef5eebfb-37b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-001e92648305";
	id1 = "ef5eebfa-47b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-002d92648305";
	id1 = "ef5eebfa-38b8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-011d92648305";
	id1 = "ef5eebfa-37c8-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d60-101d92648305";
	id1 = "ef5eebfa-37b9-11de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d61-001d92648305";
	id1 = "ef5eebfa-37b8-21de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );

	id0 = "ef5eebfa-37b8-11de-9d70-001d92648305";
	id1 = "ef5eebfa-37b8-12de-9d60-001d02648305";
	BOOST_CHECK( !(id0 > id1 && (id0<id1) && !(id0==id1)) );
}

BOOST_AUTO_TEST_CASE( UUIDCase12 )
{
	BOOST_CHECK(sizeof(UUID) == 16);
}

BOOST_AUTO_TEST_SUITE_END()
