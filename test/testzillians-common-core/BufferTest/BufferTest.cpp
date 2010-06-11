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
 * @date Mar 3, 2009 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "core/Buffer.h"
#include "utility/UUIDUtil.h"
#include <iostream>
#include <string>
#include <limits>

#define BOOST_TEST_MODULE BufferTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( BufferTest )

BOOST_AUTO_TEST_CASE( StringEncodingAndDecodingTest )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	string input[] = {"1", "22", "333", "444"};
	for(int i=0;i<4;++i)
		(*b) << input[i];

	for(int i=0;i<4;++i)
	{
		string output;
		(*b) >> output;

		BOOST_CHECK(input[i] == output);
	}

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

struct BitFieldStruct
{
	bool a:1;
	bool b:8;
	int  c:16;
	int  d:32;
};

BOOST_AUTO_TEST_CASE( BitFieldTest )
{
	Buffer b(4096);

	BitFieldStruct input;
	input.a = true;
	input.b = true;
	input.c = 0xFFFF;
	input.d = 0xFFFF;

	b << input.a << input.b << input.c << input.d;

	bool result_a = false;
	bool result_b = false;
	int  result_c = 0;
	int  result_d = 0;

	b >> result_a >> result_b >> result_c >> result_d;

	BOOST_CHECK(input.a == result_a);
	BOOST_CHECK(input.b == result_b);
	BOOST_CHECK(input.c == result_c);
	BOOST_CHECK(input.d == result_d);
}

BOOST_AUTO_TEST_CASE( CharArrayEncodingAndDecodingTest )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	const char* input[4] = {"1", "22", "333", "444"};
	for(int i=0;i<4;++i)
		(*b) << input[i];

	for(int i=0;i<4;++i)
	{
		char output[10];
		(*b) >> output;

		BOOST_CHECK(strcmp(input[i], output) == 0);
	}

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

BOOST_AUTO_TEST_CASE( IntEncodingAndDecodingTest )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	int32 input[4] = {1, 2, 3, 4};
	for(int i=0;i<4;++i)
		(*b) << input[i];

	for(int i=0;i<4;++i)
	{
		int32 output;
		(*b) >> output;

		BOOST_CHECK(input[i] == output);
	}

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

BOOST_AUTO_TEST_CASE( LongEncodingAndDecodingTest )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	long input[4] = {1L, 2L, 3L, 4L};
	for(int i=0;i<4;++i)
		(*b) << input[i];

	for(int i=0;i<4;++i)
	{
		long output;
		(*b) >> output;

		BOOST_CHECK(input[i] == output);
	}
	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

BOOST_AUTO_TEST_CASE( FloatEncodingAndDecodingTest )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	float input[4] = {1.0f, 2.0f, 3.0f, 4.0f};
	for(int i=0;i<4;++i)
		(*b) << input[i];

	for(int i=0;i<4;++i)
	{
		float output;
		(*b) >> output;

		BOOST_CHECK(input[i] == output);
	}

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

BOOST_AUTO_TEST_CASE( DoubleEncodingAndDecodingTest )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	double input[4] = {1.0, 2.0, 3.0, 4.0};
	for(int i=0;i<4;++i)
		(*b) << input[i];

	for(int i=0;i<4;++i)
	{
		double output;
		(*b) >> output;

		BOOST_CHECK(input[i] == output);
	}

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

BOOST_AUTO_TEST_CASE( IntVectorEncodingAndDecodingTest )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	std::vector<int32> input;
	input.push_back(1);
	input.push_back(2);
	input.push_back(3);
	input.push_back(4);
	input.push_back(5);

	std::size_t sizeProbed = b->probeSize(input);
	(*b) << input;
	BOOST_CHECK(b->dataSize() == sizeProbed);

	std::vector<int32> output;

	(*b) >> output;
	BOOST_CHECK(output.size() == input.size());

	for(int i=0;i<input.size();++i)
	{
		BOOST_CHECK(input[i] == output[i]);
	}

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

BOOST_AUTO_TEST_CASE( UUIDVectorEncodingAndDecodingTest )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	std::vector<UUID> input;
	UUID id;
	id.random(); input.push_back(id);
	id.random(); input.push_back(id);
	id.random(); input.push_back(id);
	id.random(); input.push_back(id);
	id.random(); input.push_back(id);

	std::size_t sizeProbed = b->probeSize(input);
	(*b) << input;
	BOOST_CHECK(b->dataSize() == sizeProbed);

	std::vector<UUID> output;

	(*b) >> output;
	BOOST_CHECK(output.size() == input.size());

	for(int i=0;i<input.size();++i)
	{
		BOOST_CHECK(input[i] == output[i]);
	}

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

BOOST_AUTO_TEST_CASE( BoostArrayEncodingAndDecodingTest )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	boost::array<int32, 256> input;
	for(int i=0;i<input.size();++i)
	{
		input[i] = i;
	}

	(*b) << input;

	boost::array<int32, 256> output;

	(*b) >> output;

	for(int i=0;i<input.size();++i)
	{
		BOOST_CHECK(input[i] == output[i]);
	}

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

struct SerializableObject1
{
	template <typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & x;
		ar & y;
	}

	inline bool operator == (const SerializableObject1& obj) const
	{
		return x == obj.x && y == obj.y;
	}

	uint32 x;
	double y;
};

BOOST_AUTO_TEST_CASE( SerializableEncodingAndDecodingTest1 )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	SerializableObject1 obj1, obj2;
	obj1.x = 10; obj1.y = 12345.67;

//	b->writeSerializable(obj1);
//	b->readSerializable(obj2);

	std::size_t sizeProbed = b->probeSize(obj1);
	(*b) << obj1;
	BOOST_CHECK(b->dataSize() == sizeProbed);

	(*b) >> obj2;

	BOOST_CHECK(obj1.x == obj2.x);
	BOOST_CHECK(obj1.y == obj2.y);

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

struct SerializableObject2
{
	template <typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & x;
		ar & y;
	}

	inline bool operator == (const SerializableObject2& obj) const
	{
		return x == obj.x && y == obj.y;
	}

	SerializableObject1 x;
	double y;
};

BOOST_AUTO_TEST_CASE( SerializableEncodingAndDecodingTest2 )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	SerializableObject2 obj1, obj2;
	obj1.x.x = 10; obj1.x.y = 12345.67; obj1.y = 345.678;

//	b->writeSerializable(obj1);
//	b->readSerializable(obj2);

	std::size_t sizeProbed = b->probeSize(obj1);
	(*b) << obj1;
	BOOST_CHECK(b->dataSize() == sizeProbed);

	(*b) >> obj2;

	BOOST_CHECK(obj1.x.x == obj2.x.x);
	BOOST_CHECK(obj1.x.y == obj2.x.y);
	BOOST_CHECK(obj1.y == obj2.y);

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

#include "utility/UUIDUtil.h"

struct SerializableObject3
{
	template <typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & x;
		ar & y;
		ar & z;
	}

	inline bool operator == (const SerializableObject3& obj) const
	{
		for(int i=0;i<x.size();++i)
		{
			if(!(x[i] == obj.x[i])) return false;
		}

		if(y.size() != obj.y.size()) return false;

		for(int i=0;i<y.size();++i)
		{
			if(!(y[i] == obj.y[i])) return false;
		}

		if(z.size() != obj.z.size()) return false;

		std::list<SerializableObject1>::const_iterator ia=z.begin();
		std::list<SerializableObject1>::const_iterator ib=obj.z.begin();
		for(;ia != z.end() && ib != obj.z.end();++ia, ++ib)
		{
			//if(!(*ia == *ib)) return false;
			if(ia->x != ib->x || ia->y != ib->y) return false;
		}

		return true;
	}

	boost::array<char,10> x;
	std::vector<double> y;
	std::list<SerializableObject1> z;
};

BOOST_AUTO_TEST_CASE( SerializableEncodingAndDecodingTest3 )
{
	char* data = new char[4096];
	Buffer *b = new Buffer((byte*)data, 4096);

	SerializableObject3 obj1, obj2;
	for(int i=0;i<obj1.x.size();++i)
	{
		obj1.x[i] = rand() % std::numeric_limits<char>::max();
	}

	for(int i=0;i<20;++i)
	{
		obj1.y.push_back(rand() * 0.1);
	}

	for(int i=0;i<10;++i)
	{
		SerializableObject1 x;
		x.x = rand() % numeric_limits<int32>::max();
		x.y = rand() * 0.2;
		obj1.z.push_back(x);
	}

	std::size_t sizeProbed = b->probeSize(obj1);
	(*b) << obj1;
	BOOST_CHECK(b->dataSize() == sizeProbed);

	(*b) >> obj2;

	BOOST_CHECK(obj1 == obj2);

	SAFE_DELETE(b);
	SAFE_DELETE_ARRAY(data);
}

//BOOST_AUTO_TEST_CASE( BufferCollectionTest1 )
//{
//	shared_ptr<Buffer> a0(new Buffer(1024));
//	shared_ptr<Buffer> a1(new Buffer(1024));
//	shared_ptr<Buffer> a2(new Buffer(1024));
//	shared_ptr<Buffer> a3(new Buffer(1024));
//
//	for(int32 i=0;i<1024/sizeof(int32);++i)
//	{
//		int32 x = 0;
//		a0->write(x);
//	}
//	for(int32 i=0;i<1024/sizeof(int32);++i)
//	{
//		int32 x = 1;
//		a1->write(x);
//	}
//	for(int32 i=0;i<1024/sizeof(int32);++i)
//	{
//		int32 x = 2;
//		a2->write(x);
//	}
//	for(int32 i=0;i<1024/sizeof(int32);++i)
//	{
//		int32 x = 3;
//		a3->write(x);
//	}
//
//	shared_ptr<BufferCollection> collection(new BufferCollection);
//	collection->add(a0);
//	collection->add(a1);
//	collection->add(a2);
//	collection->add(a3);
//
//	BOOST_CHECK_NO_THROW(collection->rskip(4096));
//
//	BOOST_REQUIRE_THROW(collection->wskip(1024), std::out_of_range);
//
//	BOOST_CHECK_NO_THROW(collection->rrev(4096));
//
//	shared_ptr<Buffer> b(new Buffer(4096));
//	b->write(*collection);
//
//	BOOST_REQUIRE_THROW(collection->rskip(1024), std::out_of_range);
//
//	for(int32 i=0;i<4096/sizeof(int32);++i)
//	{
//		int32 x;
//		b->read(x);
//
//		if(i < (1024/sizeof(int32)))
//		{
//			BOOST_CHECK(x == 0);
//		}
//		else if(i < (1024/sizeof(int32))*2)
//		{
//			BOOST_CHECK(x == 1);
//		}
//		else if(i < (1024/sizeof(int32))*3)
//		{
//			BOOST_CHECK(x == 2);
//		}
//		else if(i < (1024/sizeof(int32))*4)
//		{
//			BOOST_CHECK(x == 3);
//		}
//	}
//
//	collection->reset();
//
//	BOOST_CHECK_NO_THROW(collection->wskip(1024));
//	BOOST_CHECK(collection->dataSize() == 1024);
//
//	BOOST_CHECK_NO_THROW(collection->rskip(1024));
//	BOOST_CHECK(collection->dataSize() == 0);
//
//	BOOST_CHECK(collection->freeSize() == 1024*3);
//
//	BOOST_CHECK(collection->allocatedSize() == 1024*4);
//}

BOOST_AUTO_TEST_CASE( MapReadWriteTest1 )
{
	shared_ptr<Buffer> b(new Buffer(4096));

	std::map<std::string, std::string> input;
	input["A_KEY"] = "A_VALUE";
	input["B_KEY"] = "B_VALUE";
	input["C_KEY"] = "C_VALUE";
	input["D_KEY"] = "D_VALUE";
	input["E_KEY"] = "E_VALUE";

	std::size_t sizeProbed = b->probeSize(input);
	(*b) << input;
	BOOST_CHECK(b->dataSize() == sizeProbed);

	std::map<std::string, std::string> output;

	(*b) >> output;
	BOOST_CHECK(output.size() == input.size());

	for(std::map<std::string, std::string>::iterator i = input.begin();i != input.end();++i)
	{
		std::map<std::string, std::string>::iterator o;
		o = output.find(i->first);
		BOOST_CHECK(o != output.end());
		if(o != output.end())
		{
			BOOST_CHECK(i->second == o->second);
		}
	}
}

BOOST_AUTO_TEST_CASE( MapReadWriteTest2 )
{
	shared_ptr<Buffer> b(new Buffer(4096));

	std::map<long, std::string> input;
	input[100]   = "A_VALUE";
	input[200]   = "B_VALUE";
	input[65536] = "C_VALUE";
	input[0]     = "D_VALUE";
	input[1234]  = "E_VALUE";

	std::size_t sizeProbed = b->probeSize(input);
	(*b) << input;
	BOOST_CHECK(b->dataSize() == sizeProbed);

	std::map<long, std::string> output;

	(*b) >> output;
	BOOST_CHECK(output.size() == input.size());

	for(std::map<long, std::string>::iterator i = input.begin();i != input.end();++i)
	{
		std::map<long, std::string>::iterator o;
		o = output.find(i->first);
		BOOST_CHECK(o != output.end());
		if(o != output.end())
		{
			BOOST_CHECK(i->second == o->second);
		}
	}
}

BOOST_AUTO_TEST_CASE( MapReadWriteTest3 )
{
	shared_ptr<Buffer> b(new Buffer(4096));

	std::map<long, long> input;
	input[100]   = 456;
	input[200]   = 123;
	input[65536] = 0;
	input[0]     = 54645;
	input[1234]  = 95123;

	std::size_t sizeProbed = b->probeSize(input);
	(*b) << input;
	BOOST_CHECK(b->dataSize() == sizeProbed);

	std::map<long, long> output;

	(*b) >> output;
	BOOST_CHECK(output.size() == input.size());

	for(std::map<long, long>::iterator i = input.begin();i != input.end();++i)
	{
		std::map<long, long>::iterator o;
		o = output.find(i->first);
		BOOST_CHECK(o != output.end());
		if(o != output.end())
		{
			BOOST_CHECK(i->second == o->second);
		}
	}
}

BOOST_AUTO_TEST_CASE( ErrorCodeSerializationTest1 )
{
	shared_ptr<Buffer> b(new Buffer(4096));

	{
		boost::system::error_code input; // the default error code is success without error

		BOOST_CHECK_NO_THROW((*b) << input);

		boost::system::error_code output;

		BOOST_CHECK_NO_THROW((*b) >> output);

		BOOST_CHECK(input == output);
	}

	{
		boost::system::error_code input;
		input = boost::system::errc::make_error_code(boost::system::errc::address_in_use);

		BOOST_CHECK_NO_THROW((*b) << input);

		boost::system::error_code output;

		BOOST_CHECK_NO_THROW((*b) >> output);

		BOOST_CHECK(input == output);
	}

	{
		boost::system::error_code input;
		input = boost::system::errc::make_error_code(boost::system::errc::executable_format_error);

		BOOST_CHECK_NO_THROW((*b) << input);

		boost::system::error_code output;

		BOOST_CHECK_NO_THROW((*b) >> output);

		BOOST_CHECK(input == output);
	}

	{
		boost::system::error_code input;
		input.assign(100, boost::system::get_system_category());

		BOOST_CHECK_NO_THROW((*b) << input);

		boost::system::error_code output;

		BOOST_CHECK_NO_THROW((*b) >> output);

		BOOST_CHECK(input == output);
	}
}

BOOST_AUTO_TEST_CASE( StringStreamReadWriteTest1 )
{
	shared_ptr<Buffer> br(new Buffer(4096));

	SerializableObject1 obj1;
	obj1.x = 1024;
	obj1.y = 2048.0;

	SerializableObject1 obj2;
	obj1.x = 4096;
	obj1.y = 1234.0;

	(*br) << obj1 << obj2;

	std::stringstream ss;
	ss << (*br);

	BOOST_CHECK(ss.str().length() > 0);

	shared_ptr<Buffer> bw(new Buffer(4096));

	ss >> (*bw);

	SerializableObject1 obj3;
	SerializableObject1 obj4;
	(*bw) >> obj3 >> obj4;

	BOOST_CHECK(obj1.x == obj3.x);
	BOOST_CHECK(obj1.y == obj3.y);
	BOOST_CHECK(obj2.x == obj4.x);
	BOOST_CHECK(obj2.y == obj4.y);
}

BOOST_AUTO_TEST_CASE( Int64ByteOrderingTest )
{
	int64 value = 0xFFBBCCDD11223344;
	unsigned char* value_in_hex = (unsigned char*)&value;

	BOOST_CHECK(value_in_hex[0] == 0x44);
	BOOST_CHECK(value_in_hex[1] == 0x33);
	BOOST_CHECK(value_in_hex[2] == 0x22);
	BOOST_CHECK(value_in_hex[3] == 0x11);
	BOOST_CHECK(value_in_hex[4] == 0xDD);
	BOOST_CHECK(value_in_hex[5] == 0xCC);
	BOOST_CHECK(value_in_hex[6] == 0xBB);
	BOOST_CHECK(value_in_hex[7] == 0xFF);
}

BOOST_AUTO_TEST_CASE( UInt64ByteOrderingTest )
{
	uint64 value = 0xFFBBCCDD11223344;
	unsigned char* value_in_hex = (unsigned char*)&value;

	BOOST_CHECK(value_in_hex[0] == 0x44);
	BOOST_CHECK(value_in_hex[1] == 0x33);
	BOOST_CHECK(value_in_hex[2] == 0x22);
	BOOST_CHECK(value_in_hex[3] == 0x11);
	BOOST_CHECK(value_in_hex[4] == 0xDD);
	BOOST_CHECK(value_in_hex[5] == 0xCC);
	BOOST_CHECK(value_in_hex[6] == 0xBB);
	BOOST_CHECK(value_in_hex[7] == 0xFF);
}

BOOST_AUTO_TEST_CASE( SerializableBufferTest )
{
	std::map<std::string, Buffer> input;
	std::map<std::string, Buffer> output;
	Buffer a(32); int input_data_a = 1234; BOOST_REQUIRE_NO_THROW(a << input_data_a);
	Buffer b(32); float input_data_b = 23145.0f; BOOST_REQUIRE_NO_THROW(b << input_data_b);
	Buffer c(32); double input_data_c = 65655.0; BOOST_REQUIRE_NO_THROW(c << input_data_c);
	input.insert(std::make_pair("test_key_a", a));
	input.insert(std::make_pair("test_key_b", b));
	input.insert(std::make_pair("test_key_c", c));


	Buffer t(256);
	BOOST_REQUIRE_NO_THROW(t << input);
	BOOST_REQUIRE_NO_THROW(t >> output);

	int output_data_a; BOOST_REQUIRE_NO_THROW(output["test_key_a"] >> output_data_a); BOOST_CHECK(input_data_a == output_data_a);
	float output_data_b; BOOST_REQUIRE_NO_THROW(output["test_key_b"] >> output_data_b); BOOST_CHECK(input_data_b == output_data_b);
	double output_data_c; BOOST_REQUIRE_NO_THROW(output["test_key_c"] >> output_data_c); BOOST_CHECK(input_data_c == output_data_c);
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
BOOST_AUTO_TEST_CASE( BufferMoveSemanticsTest )
{
	Buffer a(128);
	Buffer b = a;

	byte* rptr_a = a.rptr();
	byte* rptr_b = b.rptr();
	BOOST_CHECK(rptr_a != rptr_b);

	Buffer c = std::move(a);
	byte* rptr_c = c.rptr();
	BOOST_CHECK(rptr_c == rptr_a);

	c = std::move(c);
	byte* rptr_c_copy_self = c.rptr();
	BOOST_CHECK(rptr_c == rptr_c_copy_self);

	Buffer d(256); d = std::move(c);
	BOOST_CHECK(d.allocatedSize() == 128);
}
#endif

struct ArbitraryContext
{
	template <typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & dummy1;
		ar & dummy2;
		ar & dummy3;
		ar & dummy4;
		ar & dummy5;
	}

	inline bool operator== (const ArbitraryContext& other) const
	{
		if(other.dummy1 != dummy1)	return false;
		if(other.dummy2 != dummy2)	return false;
		if(other.dummy3 != dummy3)	return false;
		if(other.dummy4 != dummy4)	return false;
		if(other.dummy5 != dummy5)	return false;
		return true;
	}

	int dummy1;
	float dummy2;
	double dummy3;
	bool dummy4;
	std::string dummy5;
};

BOOST_AUTO_TEST_CASE( BufferOnDemandResizingTest )
{
	ArbitraryContext input;
	input.dummy1 = 123;
	input.dummy2 = 456.0f;
	input.dummy3 = 789.0;
	input.dummy4 = false;
	input.dummy5 = "TEST!!!!";

	{
		Buffer b;
		b << input;

		BOOST_CHECK(b.dataSize() >= Buffer::probeSize(input));
		BOOST_CHECK(b.allocatedSize() >= Buffer::probeSize(input));

		ArbitraryContext output;
		b >> output;

		BOOST_CHECK(input == output);
	}

	{
		Buffer b;
		b << input << input;

		BOOST_CHECK(b.dataSize() >= Buffer::probeSize(input) * 2);
		BOOST_CHECK(b.allocatedSize() >= Buffer::probeSize(input) * 2);

		ArbitraryContext output;
		b >> output >> output;

		BOOST_CHECK(input == output);
	}
}

BOOST_AUTO_TEST_CASE( CircularBufferBasicTest1 )
{
	CircularBuffer b(12);
	BOOST_CHECK(b.freeSize() == 12);

	int x = 1;
	int y = 2;
	int z = 3;

	BOOST_CHECK_NO_THROW(b << x);
	BOOST_CHECK_NO_THROW(b << y);
	BOOST_CHECK_NO_THROW(b << z);

	BOOST_CHECK(b.dataSize() == 12);
	BOOST_CHECK(b.freeSize() == 0);

	int xx; b >> xx; BOOST_CHECK(x == xx);
	int yy; b >> yy; BOOST_CHECK(y == yy);
	int zz; b >> zz; BOOST_CHECK(z == zz);

	BOOST_CHECK(b.dataSize() == 0);
	BOOST_CHECK(b.freeSize() == 12);
}

BOOST_AUTO_TEST_CASE( CircularBufferBasicTest2 )
{
	CircularBuffer b(256);

	for(int iter = 0; iter = 256; ++iter)
	{
		BOOST_CHECK(b.freeSize() == 256);

		for(int i=0;i<64;++i)
		{
			BOOST_CHECK_NO_THROW(b << i);
		}

		BOOST_CHECK(b.freeSize() == 0);

		for(int i=0;i<64;++i)
		{
			int x;
			BOOST_CHECK_NO_THROW(b >> x);
			BOOST_CHECK(x == i);
		}

		BOOST_CHECK(b.freeSize() == 256);
	}
}


BOOST_AUTO_TEST_CASE( CircularBufferStrideTest )
{
	CircularBuffer b(257);

	for(int iter = 0; iter = 256; ++iter)
	{
		BOOST_CHECK(b.freeSize() == 257);

		for(int i=0;i<64;++i)
		{
			BOOST_CHECK_NO_THROW(b << i);
		}

		BOOST_CHECK(b.freeSize() == 1);

		for(int i=0;i<64;++i)
		{
			int x;
			BOOST_CHECK_NO_THROW(b >> x);
			BOOST_CHECK(x == i);
		}

		BOOST_CHECK(b.freeSize() == 257);
	}
}

BOOST_AUTO_TEST_SUITE_END()
