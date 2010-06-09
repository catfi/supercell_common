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
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/**
 * @file SharePtrCopyTest.cpp
 *
 * @date March 24, 2010 zac - Initial version created.
 */

#include "core/Prerequisite.h"
#include "tbb/tick_count.h"
#include "core/Buffer.h"
#include <iostream>
#include <vector>

#define BOOST_TEST_MODULE SharePtrCopyTest
//#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;


int loop = 1024 * 1024;
unsigned int bufferSize = 8;


BOOST_AUTO_TEST_SUITE( SharePtrCopyTest )


BOOST_AUTO_TEST_CASE ( SharePtrCopy )
{
	shared_ptr<Buffer> source(new Buffer(bufferSize));
	shared_ptr<Buffer> dest;
//	std::vector<shared_ptr<Buffer> > vec;

	tbb::tick_count start, end;
	start = tbb::tick_count::now();

	for (int i = 0; i < loop; i ++) {
		dest = source;
		dest;
	}

	end = tbb::tick_count::now();
	float total = (end - start).seconds()*1000.0;
	cout << "SharePtrCopy total:" << total << endl;
}

BOOST_AUTO_TEST_CASE ( VoidPtrCopy )
{
	Buffer* source = new Buffer(bufferSize);
	Buffer* dest;

	std::vector<Buffer*> vec;
	tbb::tick_count start, end;
	start = tbb::tick_count::now();

	for (int i = 0; i < loop; i++) {
		dest = source;
		//vec.push_back(source);
	}

	end = tbb::tick_count::now();
	float total = (end - start).seconds()*1000.0;
	cout << "VoidPtrCopy total:" << total << endl;
}

BOOST_AUTO_TEST_CASE ( SharePtrVectorInsert )
{
	shared_ptr<Buffer> source(new Buffer(bufferSize));
	shared_ptr<Buffer> dest;
	std::vector<shared_ptr<Buffer> > vec;

	tbb::tick_count start, end;
	start = tbb::tick_count::now();

	for (int i = 0; i < loop; i ++) {
		//dest = source;
		//dest;
		vec.push_back(source);
	}

	end = tbb::tick_count::now();
	float total = (end - start).seconds()*1000.0;
	cout << "SharePtrVectorInsert total:" << total << endl;
}

BOOST_AUTO_TEST_CASE ( VoidPtrVectorInsert )
{
	Buffer* source = new Buffer(bufferSize);
	Buffer* dest;

	std::vector<Buffer*> vec;
	tbb::tick_count start, end;
	start = tbb::tick_count::now();

	for (int i = 0; i < loop; i++) {
		//dest = source;
		vec.push_back(source);
	}

	end = tbb::tick_count::now();
	float total = (end - start).seconds()*1000.0;
	cout << "VoidPtrVectorInsert total:" << total << endl;
}

BOOST_AUTO_TEST_CASE ( VectorPairCopy )
{
	std::vector<std::pair<void*, size_t> > vec;
	tbb::tick_count start, end;
	start = tbb::tick_count::now();

	for (int i = 0; i < loop; i++) {
		//dest = source;
		vec.push_back(std::make_pair((void*)0, (std::size_t)0));
	}

	end = tbb::tick_count::now();
	float total = (end - start).seconds()*1000.0;
	cout << "VectorPairCopy total:" << total << endl;
}

BOOST_AUTO_TEST_SUITE_END()

