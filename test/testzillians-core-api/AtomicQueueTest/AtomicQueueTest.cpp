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
 * @date Jun 7, 2010 zac - Initial version created.
 */

#include "core-api/Prerequisite.h"
#include "core-api/AtomicQueue.h"
#include <tbb/tick_count.h>
#include <tbb/tbb_thread.h>
//#include <boost/thread/thread.hpp>

using std::cout;
using std::endl;
using namespace zillians;

#if 0
const int numData = 10000;

struct msgTest : atomic::node_t
{
	int size;
};

void ThreadReader(atomic::queue<msgTest>* queue)
{
	cout << "reader thread begin" << endl;

	msgTest *message;

	tbb::tick_count start, end;
	start = tbb::tick_count::now();

	for(int i = 0; i < numData; ++i)
	{
		message = queue->pop();
	}

	end = tbb::tick_count::now();
	float total = (end - start).seconds()*1000.0;

	cout << "througput: " << total / numData << endl;
	cout << "time: " << total << endl;
}

void ThreadWriter(atomic::queue<msgTest>* queue)
{
	cout << "writer thread begin" << endl;

	msgTest* message = new msgTest();

	for(int i = 0; i < numData; ++i)
	{
		queue->push(message);
	}
}

int main()
{
	atomic::queue<msgTest> atomicQueue;

	boost::thread threadWriter(boost::bind(&ThreadWriter, &atomicQueue));
	boost::thread threadReader(boost::bind(&ThreadReader, &atomicQueue));

	threadWriter.join();
	threadReader.join();

	return 0;
}
#else

#define numElements 256

const int numData = 102400000;

struct TestMsg
{
	int size;
};

void ThreadReader(atomic::AtomicPipe<int, numElements>* pipe)
{
	cout << "reader" << endl;

	int ret;
	for(int i = 0; i < numData; ++i)
	{
		pipe->read(&ret);
		BOOST_ASSERT(ret == i);
	}
}

void ThreadWriter(atomic::AtomicPipe<int, numElements>* pipe)
{
	cout << "writer" << endl;

	//TestMsg* message = new TestMsg;

	for(int i = 0; i < numData; ++i)
	{
		pipe->write(i, true);
	}


}

int main()
{
	atomic::AtomicPipe<int, numElements> atomicPipe;
//	atomic::AtomicQueue<int, numElements> atomicQueue;

	tbb::tick_count start, end;
	start = tbb::tick_count::now();

	tbb::tbb_thread threadWriter(boost::bind(&ThreadWriter, &atomicPipe));
	tbb::tbb_thread threadReader(boost::bind(&ThreadReader, &atomicPipe));

//	tbb::tbb_thread threadWriter(boost::bind(&ThreadWriter));
//	tbb::tbb_thread threadReader(boost::bind(&ThreadReader));

//	if(threadWriter.joinable())
		threadWriter.join();

//	if(threadReader.joinable())
		threadReader.join();

	end = tbb::tick_count::now();
	float total = (end - start).seconds()*1000.0;

	//	cout << "througput: " << total / numData << endl;
	//	cout << "time: " << total << endl;
	cout << "enqueue/dequeue: " << numData << " elements in " << total << " ms" << endl;
}

#endif
