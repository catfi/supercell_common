/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
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
 * @date Apr 13, 2011 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "core/AtomicBoundedQueue.h"
#include <iostream>
#include <string>
#include <limits>
#include <tbb/tick_count.h>
#include <tbb/concurrent_queue.h>

#define BOOST_TEST_MODULE AtomicBoundedQueueTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( AtomicBoundedQueueTest )

BOOST_AUTO_TEST_CASE( AtomicBoundedQueueTestCase1 )
{
	AtomicBoundedQueue<int> queue(64);
	for(int i=0;i<64;++i)
	{
		BOOST_CHECK(queue.push(i));
	}
	BOOST_CHECK(!queue.push(123));
	for(int i=0;i<64;++i)
	{
		int x;
		BOOST_CHECK(queue.pop(x));
		BOOST_CHECK(x == i);
	}
}

void producer_thread_proc(AtomicBoundedQueue<int>* q, int items_to_push)
{
    AtomicBoundedQueue<int>& queue = *q;

    while(items_to_push > 0)
    {
    	if(queue.push(items_to_push))
    	{
//    		printf("pushed %d\n", items_to_push);
    		--items_to_push;
    	}
    	else
    		boost::this_thread::yield();
    }

}

void consumer_thread_proc(AtomicBoundedQueue<int>* q, int items_to_pop)
{
    AtomicBoundedQueue<int>& queue = *q;

    while(items_to_pop > 0)
    {
    	int x;
    	if(queue.pop(x))
    	{
//    		printf("popped %d\n", x);
    		--items_to_pop;
    	}
    	else
    		boost::this_thread::yield();
    }
}

void producer_thread_proc_tbb(tbb::concurrent_bounded_queue<int>* q, int items_to_push)
{
	tbb::concurrent_bounded_queue<int>& queue = *q;

    while(items_to_push > 0)
    {
    	if(queue.try_push(items_to_push))
    	{
//    		printf("pushed %d\n", items_to_push);
    		--items_to_push;
    	}
    	else
    		boost::this_thread::yield();
    }

}

void consumer_thread_proc_tbb(tbb::concurrent_bounded_queue<int>* q, int items_to_pop)
{
	tbb::concurrent_bounded_queue<int>& queue = *q;

    while(items_to_pop > 0)
    {
    	int x;
    	if(queue.try_pop(x))
    	{
//    		printf("popped %d\n", x);
    		--items_to_pop;
    	}
    	else
    		boost::this_thread::yield();
    }
}

void test_spsc_push_pop(int test_queue_size, int test_iteration, int test_element_count)
{
	AtomicBoundedQueue<int> queue(test_queue_size);

	tbb::tick_count start = tbb::tick_count::now();
	printf("verifying single-producer-single-consumer scenario, pushing/poping %d elements, queue size = %d, iteration count = %d...", test_element_count, test_queue_size, test_iteration);
	for(int i=0;i<test_iteration;++i)
	{
		boost::thread producer(boost::bind(producer_thread_proc, &queue, test_element_count));
		boost::thread consumer(boost::bind(consumer_thread_proc, &queue, test_element_count));

		producer.join();
		consumer.join();
	}
	printf("passed, time = %f ms\n", (tbb::tick_count::now() - start).seconds() * 1000.0);
}

void test_spsc_push_pop_tbb(int test_queue_size, int test_iteration, int test_element_count)
{
	tbb::concurrent_bounded_queue<int> queue;

	tbb::tick_count start = tbb::tick_count::now();
	printf("[tbb] verifying single-producer-single-consumer scenario, pushing/poping %d elements, queue size = %d, iteration count = %d...", test_element_count, test_queue_size, test_iteration);
	for(int i=0;i<test_iteration;++i)
	{
		boost::thread producer(boost::bind(producer_thread_proc_tbb, &queue, test_element_count));
		boost::thread consumer(boost::bind(consumer_thread_proc_tbb, &queue, test_element_count));

		producer.join();
		consumer.join();
	}
	printf("passed, time = %f ms\n", (tbb::tick_count::now() - start).seconds() * 1000.0);
}

BOOST_AUTO_TEST_CASE( AtomicBoundedQueueTestCase2_SingleProducerSingleConsumer )
{
	test_spsc_push_pop(64, 100, 65536);
	test_spsc_push_pop(128, 100, 65536);
	test_spsc_push_pop(256, 100, 65536);
	test_spsc_push_pop(512, 100, 65536);
	test_spsc_push_pop(1024, 100, 65536);
}

BOOST_AUTO_TEST_CASE( AtomicBoundedQueueTestCase2_SingleProducerSingleConsumer_TBB )
{
	test_spsc_push_pop_tbb(64, 100, 65536);
}

void test_mpsc_push_pop(int test_queue_size, int test_iteration, int test_element_count, int producer_count)
{
	AtomicBoundedQueue<int> queue(test_queue_size);

	tbb::tick_count start = tbb::tick_count::now();
	printf("verifying %d-producer-single-consumer scenario, pushing/poping %d elements, queue size = %d, iteration count = %d...", producer_count, test_element_count, test_queue_size, test_iteration);
	for(int i=0;i<test_iteration;++i)
	{
		// create producer threads
		std::vector<boost::thread*> producers;
		for(int j=0;j<producer_count;++j)
		{
			producers.push_back(new boost::thread(boost::bind(producer_thread_proc, &queue, test_element_count/producer_count)));
		}

		// create consumer thread
		boost::thread consumer(boost::bind(consumer_thread_proc, &queue, test_element_count));

		for(int j=0;j<producer_count;++j)
		{
			producers[j]->join();
		}

		consumer.join();

		// cleanup
		for(int j=0;j<producer_count;++j)
		{
			delete producers[j];
		}
		producers.clear();
	}
	printf("passed, time = %f ms\n", (tbb::tick_count::now() - start).seconds() * 1000.0);
}

void test_mpsc_push_pop_tbb(int test_queue_size, int test_iteration, int test_element_count, int producer_count)
{
	tbb::concurrent_bounded_queue<int> queue;

	tbb::tick_count start = tbb::tick_count::now();
	printf("[tbb] verifying %d-producer-single-consumer scenario, pushing/poping %d elements, queue size = %d, iteration count = %d...", producer_count, test_element_count, test_queue_size, test_iteration);
	for(int i=0;i<test_iteration;++i)
	{
		// create producer threads
		std::vector<boost::thread*> producers;
		for(int j=0;j<producer_count;++j)
		{
			producers.push_back(new boost::thread(boost::bind(producer_thread_proc_tbb, &queue, test_element_count/producer_count)));
		}

		// create consumer thread
		boost::thread consumer(boost::bind(consumer_thread_proc_tbb, &queue, test_element_count));

		for(int j=0;j<producer_count;++j)
		{
			producers[j]->join();
		}

		consumer.join();

		// cleanup
		for(int j=0;j<producer_count;++j)
		{
			delete producers[j];
		}
		producers.clear();
	}
	printf("passed, time = %f ms\n", (tbb::tick_count::now() - start).seconds() * 1000.0);
}

BOOST_AUTO_TEST_CASE( AtomicBoundedQueueTestCase3_MultipleProducerSingleConsumer )
{
	test_mpsc_push_pop(64, 100, 65536, 2);
	test_mpsc_push_pop(128, 100, 65536, 2);
	test_mpsc_push_pop(256, 100, 65536, 2);
	test_mpsc_push_pop(512, 100, 65536, 2);
	test_mpsc_push_pop(1024, 100, 65536, 2);

	test_mpsc_push_pop(64, 100, 65536, 4);
	test_mpsc_push_pop(128, 100, 65536, 4);
	test_mpsc_push_pop(256, 100, 65536, 4);
	test_mpsc_push_pop(512, 100, 65536, 4);
	test_mpsc_push_pop(1024, 100, 65536, 4);
}

BOOST_AUTO_TEST_CASE( AtomicBoundedQueueTestCase3_MultipleProducerSingleConsumer_TBB )
{
	test_mpsc_push_pop_tbb(64, 100, 65536, 2);
	test_mpsc_push_pop_tbb(64, 100, 65536, 4);
}

void test_mpmc_push_pop(int test_queue_size, int test_iteration, int test_element_count, int producer_count, int consumer_count)
{
	AtomicBoundedQueue<int> queue(test_queue_size);

	tbb::tick_count start = tbb::tick_count::now();
	printf("verifying %d-producer-%d-consumer scenario, pushing/poping %d elements, queue size = %d, iteration count = %d...", producer_count, consumer_count, test_element_count, test_queue_size, test_iteration);
	for(int i=0;i<test_iteration;++i)
	{
		// create producer threads
		std::vector<boost::thread*> producers;
		for(int j=0;j<producer_count;++j)
		{
			producers.push_back(new boost::thread(boost::bind(producer_thread_proc, &queue, test_element_count/producer_count)));
		}

		// create consumer thread
		std::vector<boost::thread*> consumers;
		for(int j=0;j<consumer_count;++j)
		{
			consumers.push_back(new boost::thread(boost::bind(consumer_thread_proc, &queue, test_element_count/consumer_count)));
		}

		for(int j=0;j<producer_count;++j)
		{
			producers[j]->join();
		}

		for(int j=0;j<consumer_count;++j)
		{
			consumers[j]->join();
		}

		// cleanup
		for(int j=0;j<producer_count;++j)
		{
			delete producers[j];
		}
		producers.clear();

		for(int j=0;j<consumer_count;++j)
		{
			delete consumers[j];
		}
		consumers.clear();
	}
	printf("passed, time = %f ms\n", (tbb::tick_count::now() - start).seconds() * 1000.0);
}

void test_mpmc_push_pop_tbb(int test_queue_size, int test_iteration, int test_element_count, int producer_count, int consumer_count)
{
	tbb::concurrent_bounded_queue<int> queue;

	tbb::tick_count start = tbb::tick_count::now();
	printf("[tbb] verifying %d-producer-%d-consumer scenario, pushing/poping %d elements, queue size = %d, iteration count = %d...", producer_count, consumer_count, test_element_count, test_queue_size, test_iteration);
	for(int i=0;i<test_iteration;++i)
	{
		// create producer threads
		std::vector<boost::thread*> producers;
		for(int j=0;j<producer_count;++j)
		{
			producers.push_back(new boost::thread(boost::bind(producer_thread_proc_tbb, &queue, test_element_count/producer_count)));
		}

		// create consumer thread
		std::vector<boost::thread*> consumers;
		for(int j=0;j<consumer_count;++j)
		{
			consumers.push_back(new boost::thread(boost::bind(consumer_thread_proc_tbb, &queue, test_element_count/consumer_count)));
		}

		for(int j=0;j<producer_count;++j)
		{
			producers[j]->join();
		}

		for(int j=0;j<consumer_count;++j)
		{
			consumers[j]->join();
		}

		// cleanup
		for(int j=0;j<producer_count;++j)
		{
			delete producers[j];
		}
		producers.clear();

		for(int j=0;j<consumer_count;++j)
		{
			delete consumers[j];
		}
		consumers.clear();
	}
	printf("passed, time = %f ms\n", (tbb::tick_count::now() - start).seconds() * 1000.0);
}

BOOST_AUTO_TEST_CASE( AtomicBoundedQueueTestCase4_MultipleProducerMultipleConsumer )
{
	test_mpmc_push_pop(64, 100, 65536, 2, 2);
	test_mpmc_push_pop(128, 100, 65536, 2, 2);
	test_mpmc_push_pop(256, 100, 65536, 2, 2);
	test_mpmc_push_pop(512, 100, 65536, 2, 2);
	test_mpmc_push_pop(1024, 100, 65536, 2, 2);

	test_mpmc_push_pop(64, 100, 65536, 4, 4);
	test_mpmc_push_pop(128, 100, 65536, 4, 4);
	test_mpmc_push_pop(256, 100, 65536, 4, 4);
	test_mpmc_push_pop(512, 100, 65536, 4, 4);
	test_mpmc_push_pop(1024, 100, 65536, 4, 4);
}

BOOST_AUTO_TEST_CASE( AtomicBoundedQueueTestCase4_MultipleProducerMultipleConsumer_TBB )
{
	test_mpmc_push_pop_tbb(64, 100, 65536, 2, 2);
	test_mpmc_push_pop_tbb(64, 100, 65536, 4, 4);
}

BOOST_AUTO_TEST_SUITE_END()
