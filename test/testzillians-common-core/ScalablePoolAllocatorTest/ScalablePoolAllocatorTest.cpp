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
 * @file ScalablePoolAllocator.cpp
 * @todo Put file description here.
 *
 * @date Mar 1, 2009 nothing - Initial version created.
 */

/**
 * Arguments
 * @li -perf	Performance test
 * @li -intg	Integrity test
 */

#include <iostream>
#include <cstring>
#include "core/ScalablePoolAllocator.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

using namespace std;

enum TestMode
{
	Unknown = 0,
	Performance = 1,
	Integrity
};

TestMode testMode = Unknown;
log4cxx::LoggerPtr logg(log4cxx::Logger::getLogger("zillians.common.core.ScalablePoolAllocatorTest"));

int testPerformance();
int testIntegrity();

int main(int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure();

	// Parse argv
	for(int i = 1; i < argc; ++i)
	{
		if(strcmp(argv[i], "-perf") == 0)
		{
			testMode = Performance;
		}
		else if(strcmp(argv[i], "-intg") == 0)
		{
			testMode = Integrity;
		}
	}

	// No test mode selected, error
	if(testMode == Unknown)
	{
		LOG4CXX_ERROR(logg, "No test selected. Use -perf for performance test, -intg for integrity test.");
		return -1;
	}

	int ret = 0;
	if(testMode == Performance)
	{
		ret = testPerformance();
	}
	else if(testMode == Integrity)
	{
		ret = testIntegrity();
	}

	// Finish
	return ret;
}


struct Allocation
{
	size_t sz;
	byte* data;
};


/**
 * Test allocation and deallocation speed by allocate/deallocate
 * 1000000 times.
 *
 * Default settings:
 * @li 4 threads
 * @li Random allocation between 4 and 16384 bytes. Making half of them in bins, half of them in large allocation.
 *
 * @return 0 for success, fail otherwise
 */
int testPerformance()
{
	const size_t sz = 100 * 1048576;
	byte* buf = new byte[sz];// 100MB
	zillians::ScalablePoolAllocator allocator(buf, sz);



	delete[] buf;
	return -1;
}


int testIntegrity()
{
	return -1;
}
