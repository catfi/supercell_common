#include <boost/pool/object_pool.hpp>
#include <tbb/tick_count.h>
#include <list>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "tbb/tbb_thread.h"
#include <boost/bind.hpp>

//#include <core-api/SharedCount.h>
//#include <core-api/ObjectPool.h>

//using namespace zillians;

class DummyMessage
{
public:
	DummyMessage() { }
	~DummyMessage() { }
	
public:
	int data;
};

void testNaiveAllocationSingle(int iterations)
{
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		DummyMessage *obj = new DummyMessage();
		delete obj;
	}
	end = tbb::tick_count::now();
	printf("\tnative allocate/delete (single) takes %lf ms\n", (end - start).seconds()*1000.0);
}

void testNaiveAllocationArray(int iterations)
{
	tbb::tick_count start, end;
	
	DummyMessage** objlist = new DummyMessage*[iterations];
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			objlist[i] = new DummyMessage();
		}
		for(int j=0;j<iterations;++j)
		{
			delete objlist[j];
		}
	}
	end = tbb::tick_count::now();
	printf("\tnative allocate/delete (array) takes %lf ms\n", (end - start).seconds()*1000.0);
	
	delete[] objlist;
}

void testNaiveReplacementAllocationArray(int iterations)
{
	tbb::tick_count start, end;
	
	char* buffer = new char[sizeof(DummyMessage)*iterations];
	DummyMessage** objlist = new DummyMessage*[iterations];
	
	start = tbb::tick_count::now();
	{
		size_t offset = 0;
		for(int i=0;i<iterations;++i)
		{
			objlist[i] = new (buffer + offset) DummyMessage();
			offset+=sizeof(DummyMessage);
		}
		for(int j=0;j<iterations;++j)
		{
			objlist[j]->~DummyMessage();
		}
	}
	end = tbb::tick_count::now();
	printf("\tnative placement allocate/delete (array) takes %lf ms\n", (end - start).seconds()*1000.0);
	
	delete[] objlist;
	delete[] buffer;
}
/*
class DummyBoostPooledMessage
{
public:
	DummyBoostPooledMessage() { }
	~DummyBoostPooledMessage() { }
	
public:
	int data;
};

boost::object_pool<DummyBoostPooledMessage> pool;

void testBoostObjectPoolSingle(int iterations)
{
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		DummyBoostPooledMessage *obj = pool.malloc();
		pool.destroy(obj);
	}
	end = tbb::tick_count::now();
	printf("\tallocate/delete on boost object pool (single) takes %lf ms\n", (end - start).seconds()*1000.0);
	
}

void testBoostObjectPoolArraySameOrder(int iterations)
{
	tbb::tick_count start, end;
	
	DummyBoostPooledMessage** objlist = new DummyBoostPooledMessage*[iterations];
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			objlist[i] = pool.malloc();
		}
		for(int j=0;j<iterations;++j)
		{
			pool.destroy(objlist[j]);
		}
	}
	end = tbb::tick_count::now();
	printf("\tallocate/delete on boost object pool (array) (same order) takes %lf ms\n", (end - start).seconds()*1000.0);
	delete [] objlist;
}

void testBoostObjectPoolArrayReversedOrder(int iterations)
{
	tbb::tick_count start, end;
	
	DummyBoostPooledMessage** objlist = new DummyBoostPooledMessage*[iterations];
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			objlist[i] = pool.malloc();
		}
		for(int j=iterations-1;j>=0;--j)
		{
			pool.destroy(objlist[j]);
		}
	}
	end = tbb::tick_count::now();
	printf("\tallocate/delete on boost object pool (array) (reversed order) takes %lf ms\n", (end - start).seconds()*1000.0);
	delete [] objlist;
}
*/
/*
class DummyMyPooledMessage : public ObjectPool<DummyMyPooledMessage>
{
public:
	DummyMyPooledMessage() { }
	~DummyMyPooledMessage() { }
	
public:
	int data;
};

void testMyObjectPoolSameOrder(int iterations)
{
	tbb::tick_count start, end;
	
	DummyMyPooledMessage** objlist = new DummyMyPooledMessage*[iterations];
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			objlist[i] = new DummyMyPooledMessage();
		}
		for(int j=0;j<iterations;++j)
		{
			delete objlist[j];
			//ZN_SAFE_RELEASE(objlist[j]);
		}
	}
	end = tbb::tick_count::now();
	printf("\tallocate/delete on my object pool (reversed order) takes %lf ms\n", (end - start).seconds()*1000.0);
	delete [] objlist;	
}

void testMyObjectPoolReversedOrder(int iterations)
{
	tbb::tick_count start, end;
	
	DummyMyPooledMessage** objlist = new DummyMyPooledMessage*[iterations];
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			objlist[i] = new DummyMyPooledMessage();
		}
		for(int j=iterations-1;j>=0;--j)
		{
			delete objlist[j];
			//ZN_SAFE_RELEASE(objlist[j]);
		}
	}
	end = tbb::tick_count::now();
	printf("\tallocate/delete on my object pool (reversed order) takes %lf ms\n", (end - start).seconds()*1000.0);
	delete [] objlist;	
}
*/
/*
#define NS_BOOST_MEMORY_BEGIN   namespace std {
#define NS_BOOST_MEMORY_END     }
#define NS_BOOST_MEMORY         std
#define NS_BOOST_MEMORY_POLICY_BEGIN
#define NS_BOOST_MEMORY_POLICY_END
#define NS_BOOST_MEMORY_POLICY  std

#ifndef NS_BOOST_DETAIL_BEGIN
#define NS_BOOST_DETAIL_BEGIN   namespace std {
#define NS_BOOST_DETAIL_END     }
#define NS_BOOST_DETAIL         std
#endif

#define winx_call 
#include <boost/memory.hpp>

void testBoostSandboxMemorySameOrder(int iterations)
{
	tbb::tick_count start, end;
	
	DummyBoostPooledMessage** objlist = new DummyBoostPooledMessage*[iterations];
	
	start = tbb::tick_count::now();
	{
		boost::scoped_alloc mScopedAlloc;
		for(int i=0;i<iterations;++i)
		{
			objlist[i] = BOOST_MEMORY_NEW(mScopedAlloc, DummyBoostPooledMessage);
		}
	}
	end = tbb::tick_count::now();
	printf("\tallocate/delete on boost object pool (array) (reversed order) takes %lf ms\n", (end - start).seconds()*1000.0);
	delete [] objlist;
}
*/


#define ITERATION_COUNT 2
#define ELEMENT_COUNT 20000
int main(int argc, char** argv)
{
	for(int i=0;i<ITERATION_COUNT;++i)
		testNaiveAllocationSingle(ELEMENT_COUNT);
	
	for(int i=0;i<ITERATION_COUNT;++i)
		testNaiveAllocationArray(ELEMENT_COUNT);
		
	for(int i=0;i<ITERATION_COUNT;++i)
		testNaiveReplacementAllocationArray(ELEMENT_COUNT);
	
/*	for(int i=0;i<ITERATION_COUNT;++i)
		testBoostObjectPoolSingle(ELEMENT_COUNT);
	
	for(int i=0;i<ITERATION_COUNT;++i)
		testBoostObjectPoolArraySameOrder(ELEMENT_COUNT);
	
	for(int i=0;i<ITERATION_COUNT;++i)
		testBoostObjectPoolArrayReversedOrder(ELEMENT_COUNT);
*/	
	//for(int i=0;i<ITERATION_COUNT;++i)
	//	testMyObjectPoolSameOrder(ELEMENT_COUNT);
	
	//for(int i=0;i<ITERATION_COUNT;++i)
	//	testMyObjectPoolReversedOrder(ELEMENT_COUNT);
	//for(int i=0;i<ITERATION_COUNT;++i)
	//	testBoostSandboxMemorySameOrder(ELEMENT_COUNT);
	
	return 0;
}

