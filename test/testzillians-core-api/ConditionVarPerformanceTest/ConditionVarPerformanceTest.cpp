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
 * @date Jun 10, 2009 rocet - Initial version created.
 */


#include "core-api/Prerequisite.h"
#include "eventcount.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/thread.hpp>
#include <tbb/spin_rw_mutex.h>
#include <tbb/tbb_thread.h>
#include <tbb/tick_count.h>
#include <tbb/concurrent_queue.h>
#include "core-api/ConditionVariable.h"

#define BOOST_TEST_MODULE ConditionVarPerformanceTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;


BOOST_AUTO_TEST_SUITE( ConditionVarPerformanceTest )

struct EventcountPingPongTestLocal
{
	void consumer()
	{
		consumer_ready = false;

		counter = 0;
		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			while(!consumer_ready)
			{
				consumer_ec.prepare_wait();
				consumer_ec.wait();
			}

			consumer_ready = false;
//			printf("consumer counter = %d\n", counter);
			BOOST_CHECK(counter % 2 == 1);
			++counter;

			producer_ready = true;
			producer_ec.notify_one();
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("[Eventcount] wait for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	void producer()
	{
		producer_ready = false;

		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			producer_ready = false;
//			printf("producer counter = %d\n", counter);
			BOOST_CHECK(counter % 2 == 0);
			++counter;

			consumer_ready = true;
			consumer_ec.notify_one();

			while(!producer_ready)
			{
				producer_ec.prepare_wait();
				producer_ec.wait();
			}
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("[Eventcount] notify for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	volatile uint32 counter;
	zillians::mutex m;
	//zillians::condition_variable var;
	zillians::eventcount consumer_ec;
	zillians::eventcount producer_ec;
	volatile bool consumer_ready;
	volatile bool producer_ready;
	const static uint32 iterations = 20000;
};

BOOST_AUTO_TEST_CASE( EventcountPingPongTestCase )
{
//	EventcountPingPongTestLocal obj;
//
//	tbb::tbb_thread t0(boost::bind(&EventcountPingPongTestLocal::consumer, &obj));
//	tbb::tbb_thread t1(boost::bind(&EventcountPingPongTestLocal::producer, &obj));
//
//	t0.join();
//	t1.join();
}

struct CondVarPingPongTestLocal
{
	void consumer()
	{
		consumer_ready = false;

		counter = 0;

		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			{
				boost::unique_lock<boost::mutex> lock(m);
				bool waited = false;
				while(!consumer_ready)
				{
					if(waited)
					{
						printf("spurious! i = %d\n", i);
					}
					consumer_cond.wait(lock);
					waited = true;
				}
			}

			consumer_ready = false;
//			if(counter % 1000 == 1)
//				printf("consumer counter = %d\n", counter);
			BOOST_CHECK(counter % 2 == 1);
			++counter;

			producer_ready = true;
			producer_cond.notify_one();
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("[Boost ConditionVar] wait for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	void producer()
	{
		producer_ready = false;
		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			producer_ready = false;
//			if(counter % 1000 == 0)
//				printf("producer counter = %d\n", counter);
			BOOST_CHECK(counter % 2 == 0);
			++counter;

			consumer_ready = true;
			consumer_cond.notify_one();

			boost::unique_lock<boost::mutex> lock(m);
			bool waited = false;
			while(!producer_ready)
			{
				if(waited)
				{
					printf("spurious! i = %d\n", i);
				}
				producer_cond.wait(lock);
				waited = true;
			}
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("[Boost ConditionVar] notify for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	volatile uint32 counter;
	boost::mutex m;
	boost::condition_variable consumer_cond;
	boost::condition_variable producer_cond;
	volatile bool consumer_ready;
	volatile bool producer_ready;
	const static uint32 iterations = 20000;
};

BOOST_AUTO_TEST_CASE( CondVarPingPongTestCase )
{
//	CondVarPingPongTestLocal obj;
//
//	tbb::tbb_thread t0(boost::bind(&CondVarPingPongTestLocal::consumer, &obj));
//	tbb::tbb_thread t1(boost::bind(&CondVarPingPongTestLocal::producer, &obj));
//
//	t0.join();
//	t1.join();
}

struct ConcurrentQueuePingPongTestLocal
{
	void consumer()
	{
		counter = 0;
		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			uint32 dummy = 0;
			consumer_q.pop(dummy);

			if(counter % 10000 == 0)
				printf("consumer counter = %d, dummy = %d\n", counter, dummy);
			BOOST_CHECK(counter % 2 == 0);
			dummy = ++counter;

			producer_q.push(dummy);
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("[ConcurrentQueue] wait for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	void producer()
	{
		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			uint32 dummy = counter;

			consumer_q.push(dummy);
			producer_q.pop(dummy);

			if(counter % 10000 == 1)
				printf("producer counter = %d, dummy = %d\n", counter, dummy);
			BOOST_CHECK(counter % 2 == 1);
			++counter;
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("[ConcurrentQueue] notify for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	volatile uint32 counter;
	tbb::concurrent_bounded_queue<uint32> consumer_q;
	tbb::concurrent_bounded_queue<uint32> producer_q;
	const static uint32 iterations = 200000;
};

BOOST_AUTO_TEST_CASE( ConcurrentQueuePingPongTestCase )
{
	ConcurrentQueuePingPongTestLocal obj;

	tbb::tbb_thread t0(boost::bind(&ConcurrentQueuePingPongTestLocal::consumer, &obj));
	tbb::tbb_thread t1(boost::bind(&ConcurrentQueuePingPongTestLocal::producer, &obj));

	t0.join();
	t1.join();
}

class AckQueue
{
public:
	typedef uint32 Key;

	class AckSlot
	{
	public:
		AckSlot()
		{ reset(); }

		~AckSlot()
		{ }

	public:
		void wait()
		{
			while(mCount == 0) tbb::this_tbb_thread::yield();
		}

		void signal()
		{
			mCount++;
		}
		void reset()
		{
			mCount = 0;
		}
	private:
		tbb::atomic<uint32> mCount;
	};

public:
	AckQueue()
	{ }

	void wait(Key key)
	{
		tbb::spin_rw_mutex::scoped_lock lock(mAckMap.lock, false);

		AckMap::iterator it = mAckMap.map.find(key);
		//BOOST_ASSERT( it != mAckMap.map.end() );// NOTE: Commented out because of Win32 compilation error
		/* Error
			error C2668: '_wassert' : 模稜兩可的呼叫多載函式	
			\zillians\projects\common\test\testzillians-core-api\ConditionVarPerformanceTest\ConditionVarPerformanceTest.cpp	307
		*/

		try
		{
			it->second->wait();
		}
		catch(...)
		{
			printf("exception catched while waiting\n");
		}

		while(!lock.upgrade_to_writer())
		{
			printf("failed to upgrade to writer\n");
		}

		SharedPtr<AckSlot> cond = it->second;
		cond->reset();

		mAckMap.map.erase(it);
		mAckSlotQueue.push(cond);
	}

	void signal(Key key)
	{
		tbb::spin_rw_mutex::scoped_lock lock(mAckMap.lock, false);

		AckMap::iterator it = mAckMap.map.find(key);
		//BOOST_ASSERT(it != mAckMap.map.end());// NOTE: Commented out because of Win32 compilation error

		it->second->signal();
	}

	Key next()
	{
		tbb::spin_rw_mutex::scoped_lock lock(mAckMap.lock, true);

		Key key = ++mCounter;

		SharedPtr<AckSlot> cond;
		if(!mAckSlotQueue.try_pop(cond))
		{
			cond = SharedPtr<AckSlot>(new AckSlot);
		}

		mAckMap.map[key] = cond;

		return key;
	}

private:
	tbb::atomic<Key> mCounter;

	typedef std::map<Key, SharedPtr<AckSlot> > AckMap;
	struct
	{
		AckMap map;
		tbb::spin_rw_mutex lock;
	} mAckMap;

	typedef tbb::concurrent_queue< SharedPtr<AckSlot> > AckSlotQueue;
	AckSlotQueue mAckSlotQueue;
};

struct AckQueuePingPongTestLocal
{
	AckQueuePingPongTestLocal()
	{
		key = q.next();
	}
	void consumer()
	{
		counter = 0;
		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			q.wait(key);

			printf("consumer counter = %d\n", counter);
			BOOST_CHECK(counter % 2 == 0);
			++counter;

			q.signal(key);
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("wait for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	void producer()
	{
		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			q.signal(key);
			q.wait(key);
			printf("producer counter = %d\n", counter);
			BOOST_CHECK(counter % 2 == 1);
			++counter;
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("notify for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	volatile uint32 counter;
	AckQueue q;
	AckQueue::Key key;
	const static uint32 iterations = 20;
};

BOOST_AUTO_TEST_CASE( AckQueuePingPongTestCase )
{
//	AckQueuePingPongTestLocal obj;
//
//	tbb::tbb_thread t0(boost::bind(&AckQueuePingPongTestLocal::consumer, &obj));
//	tbb::tbb_thread t1(boost::bind(&AckQueuePingPongTestLocal::producer, &obj));
//
//	t0.join();
//	t1.join();
}

//struct IoServicePingPongTestLocal
//{
//	IoServicePingPongTestLocal()
//	{
//	}
//
//	void consumer()
//	{
//		counter = 0;
//		tbb::tick_count s = tbb::tick_count::now();
//		for(int i=0;i<iterations;++i)
//		{
//			q.wait(key);
//
//			printf("consumer counter = %d\n", counter);
//			BOOST_CHECK(counter % 2 == 0);
//			++counter;
//
//			q.signal(key);
//		}
//		tbb::tick_count e = tbb::tick_count::now();
//		printf("wait for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
//	}
//
//	void producer()
//	{
//		tbb::tick_count s = tbb::tick_count::now();
//		for(int i=0;i<iterations;++i)
//		{
//			q.signal(key);
//			q.wait(key);
//			printf("producer counter = %d\n", counter);
//			BOOST_CHECK(counter % 2 == 1);
//			++counter;
//		}
//		tbb::tick_count e = tbb::tick_count::now();
//		printf("notify for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
//	}
//
//	volatile uint32 counter;
//	boost::asio::io_service io_consumer;
//	boost::asio::io_service io_producer;
//	const uint32 iterations = 20;
//};

BOOST_AUTO_TEST_CASE( IoServicePingPongTestCase )
{
//	IoServicePingPongTestLocal obj;
//
//	tbb::tbb_thread t0(boost::bind(&AckQueuePingPongTestLocal::consumer, &obj));
//	tbb::tbb_thread t1(boost::bind(&AckQueuePingPongTestLocal::producer, &obj));
//
//	t0.join();
//	t1.join();
}

struct TbbQueueCondVarPingPongTestCaseLocal
{
	void consumer()
	{
		counter = 0;
		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			uint32 dummy = 0;
			consumer_cond.wait(dummy);

			if(counter % 10000 == 0)
				printf("consumer counter = %d, dummy = %d\n", counter, dummy);
			BOOST_CHECK(counter % 2 == 0);
			dummy = ++counter;

			producer_cond.signal(dummy);
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("[zillians::ConditionVariable] wait for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	void producer()
	{
		tbb::tick_count s = tbb::tick_count::now();
		for(int i=0;i<iterations;++i)
		{
			uint32 dummy = counter;

			consumer_cond.signal(dummy);
			producer_cond.wait(dummy);

			if(counter % 10000 == 1)
				printf("producer counter = %d, dummy = %d\n", counter, dummy);
			BOOST_CHECK(counter % 2 == 1);
			++counter;
		}
		tbb::tick_count e = tbb::tick_count::now();
		printf("[zillians::ConditionVariable] notify for %d times takes %f ms\n", iterations, (e-s).seconds()*1000.0);
	}

	volatile uint32 counter;
	zillians::ConditionVariable<uint32> consumer_cond;
	zillians::ConditionVariable<uint32> producer_cond;
	const static uint32 iterations = 200000;
};

BOOST_AUTO_TEST_CASE( TbbQueueCondVarPingPongTestCase )
{
	TbbQueueCondVarPingPongTestCaseLocal obj;

	tbb::tbb_thread t0(boost::bind(&TbbQueueCondVarPingPongTestCaseLocal::consumer, &obj));
	tbb::tbb_thread t1(boost::bind(&TbbQueueCondVarPingPongTestCaseLocal::producer, &obj));

	t0.join();
	t1.join();
}

// KNOWN BUG: ConditionVariable cannot take bool as its template argument, due to a bug in tbb::concurrent_bounded_queue

//void signal_variable(zillians::ConditionVariable<bool>& var)
//{
//	var.signal(true);
//}
//
//BOOST_AUTO_TEST_CASE( TbbQueueCondVarPingPongTestCase2 )
//{
//	zillians::ConditionVariable<bool> xd;
//
//	tbb::tbb_thread t(boost::bind(signal_variable, xd));
//	bool dummy = false;
//	BOOST_CHECK_NO_THROW(xd.wait(dummy));
//}

BOOST_AUTO_TEST_SUITE_END()

