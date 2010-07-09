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
#include <tbb/tick_count.h>

#define BOOST_TEST_MODULE MemoryCopyPerformanceTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace zillians;

BOOST_AUTO_TEST_SUITE( MemoryCopyPerformanceTestCase )

template<typename T>
double test_memcpy_single_thread(int size, int count)
{
	T* input = new T[size * count];
	T* output = new T[size * count];

	T* it_input = input;
	T* it_output = output;

	tbb::tick_count start, stop;
	start = tbb::tick_count::now();
	for(int i=0;i<count;++i)
	{
		memcpy((void*)it_output, (void*)it_input, size * sizeof(T));
		it_input += size;
		it_output += size;
	}
	stop = tbb::tick_count::now();

	delete[] input;
	delete[] output;

	return (stop-start).seconds() * 1000.0;
}

template<typename T>
double test_strncpy_single_thread(int size, int count)
{
	T* input = new T[size * count];
	T* output = new T[size * count];

	T* it_input = input;
	T* it_output = output;

	tbb::tick_count start, stop;
	start = tbb::tick_count::now();
	for(int i=0;i<count;++i)
	{
		strncpy((char*)it_output, (char*)it_input, size * sizeof(T));
		it_input += size;
		it_output += size;
	}
	stop = tbb::tick_count::now();

	delete[] input;
	delete[] output;

	return (stop-start).seconds() * 1000.0;
}

template<typename T>
double test_forloop_single_thread(int size, int count)
{
	T* input = new T[size * count];
	T* output = new T[size * count];

	T* it_input = input;
	T* it_output = output;

	tbb::tick_count start, stop;
	start = tbb::tick_count::now();
	for(int i=0;i<count;++i)
	{
		for(int j=0;j<size;++j)
		{
			*it_output = *it_input;
			++it_output; ++it_input;
		}
	}
	stop = tbb::tick_count::now();

	delete[] input;
	delete[] output;

	return (stop-start).seconds() * 1000.0;
}

BOOST_AUTO_TEST_CASE( MemoryCopyPerformanceTestCase1 )
{
	//for(int size = 1; size <= 8192; size *= 2)
	std::size_t size = 64*1024*1024;
	{
		//for(int count = 1024; count <= 32768; count *= 2)
		int count = 1;
		{
			double t_memcpy = 0.0;
			double t_strncpy = 0.0;
			double t_forloop = 0.0;
			for(int iter = 0; iter < 10; ++iter)
			{
				t_memcpy += test_memcpy_single_thread<int>(size, count);
				t_strncpy += test_strncpy_single_thread<int>(size, count);
				t_forloop += test_forloop_single_thread<int>(size, count);
			}
			t_memcpy /= 10.0;
			t_strncpy /= 10.0;
			t_forloop /= 10.0;

			//printf("size = %4d, count = %5d, total bytes = %9ld KB, memcpy time = %5.6f ms, strncpy time = %5.6f ms", size, count, size * count * sizeof(int) / 1024, t_memcpy, t_strncpy);
			cout << "size = " << setw(4) << size <<
					", count = " << setw(5) << count <<
					", total bytes = " << setw(7) << size * count * sizeof(int) / 1024 << " KB" <<
					", memcpy time = " << setprecision(5) << setw(10) << t_memcpy << " ms" <<
					", strncpy time = " << setprecision(5) << setw(10) << t_strncpy << " ms" <<
					", forloop time = " << setprecision(5) << setw(10) << t_forloop << " ms";
			if(t_memcpy < t_strncpy)
			{
				cout << ", winner: " << "memcpy  is " << t_strncpy / t_memcpy << " times faster" << endl;
			}
			else
			{
				cout << ", winner: " << "strncpy is " << t_memcpy / t_strncpy << " times faster" << endl;
			}
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
