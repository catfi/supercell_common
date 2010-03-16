//============================================================================
// Name        : MemoryManagementTest.cpp
// Author      : Michael Kuo
// Version     :
// Copyright   :
// Description :
//============================================================================
/**
 * - Test case
 *   1. Allocate 256MB of memory for allocator
 *   2. Generate an allocation/deallocation sequence for testing
 *   3. Spawn 8 threads
 *   4. In each thread, allocate/deallocate objects according to the sequence
 *   5. Time the allocation/deallocation sequence
 *   6. Repeat 3 - 5 for each allocator
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include "core-api/Types.h"
#include "core-api/ScalablePoolAllocator.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "tbb/tbb_thread.h"
#include "tbb/tick_count.h"
#include "tbb/atomic.h"


using std::cout;
using std::endl;
using std::setw;
using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace zillians;
//using zillians::ScalablePoolAllocator;

const size_t TEST_SIZE = 1048576 * 1024;//100MB
const size_t THREAD_COUNT = 1;
const size_t MINIMUM_OBJECT_SIZE = 8;
const size_t MAXIMUM_OBJECT_SIZE = 16384;//8192;//20000;
const size_t ALLOC_COUNT = 10000;
const size_t REPEAT_COUNT = 50;

struct TAllocation
{
	size_t sz;
	byte* data;
	int data_hash;
	tbb::atomic<bool> allocated;
	int idx;

	TAllocation(size_t size)
	{
		sz = size;
		data = 0;
		data_hash = 0;
		allocated = false;
	}
};
enum EAction
{
	Allocate = 0,
	Deallocate
};
struct TAction
{
	EAction action;
	TAllocation *target;
};
std::vector<TAllocation*> gAllocationList;
std::vector< std::list<TAction> > gThreadActionSeq;

LoggerPtr logger(Logger::getLogger("MMTest"));

void runTestSuite(zillians::ScalablePoolAllocator* alloc);
double runTest(zillians::ScalablePoolAllocator* alloc);
int runTestThread(int idx, zillians::ScalablePoolAllocator* palloc);		// simple alloc/dealloc test
int genSequences(size_t totalCount);

class bindT
{
public:
	int(*mFunc)(int,ScalablePoolAllocator*);
	int mV;
	ScalablePoolAllocator* mAlloc;
	bindT(int(*func)(int,ScalablePoolAllocator*), int v, ScalablePoolAllocator* palloc)
	{
		mFunc = func;
		mV = v;
		mAlloc = palloc;
	}
	void operator() ()
	{
		mFunc(mV, mAlloc);
	}
};

int main() {
	ScalablePoolAllocator* palloc;
	byte* mem;
	int ret = 0;

	// Init logger
    try
    {
        // Set up a simple configuration that logs on the console.
        BasicConfigurator::configure();
    }
    catch(Exception&)
    {
            ret = EXIT_FAILURE;
    }
    LOG4CXX_INFO(logger, "Entering application.");
    cout<<"* THREAD_COUNT        = "<<THREAD_COUNT<<endl;
    cout<<"* MINIMUM_OBJECT_SIZE = "<<MINIMUM_OBJECT_SIZE<<endl;
    cout<<"* ALLOCATION COUNT    = "<<ALLOC_COUNT * REPEAT_COUNT<<endl;
    cout<<"* POOL SIZE           = "<<TEST_SIZE<<endl;

    LOG4CXX_INFO(logger, "Generating allocation sequence. # of allocation = "<<ALLOC_COUNT);
    if( 0 != (ret = genSequences(ALLOC_COUNT)) ) { return ret; }
    LOG4CXX_INFO(logger, "Generating allocation sequence complete");

	cout<<"Allocator performance test"<<endl;
	mem = new byte[TEST_SIZE];

	/// TEST
	cout<<"ScalableAllocator       "<<endl;
	palloc = new ScalablePoolAllocator(mem, TEST_SIZE);
	runTestSuite(palloc);
	delete palloc;



	// Clean up
	for(size_t i = 0; i < ALLOC_COUNT; ++i)
	{
		delete gAllocationList[i];
	}

	delete[] mem;
	mem = 0;
    LOG4CXX_INFO(logger, "Exiting application.");
	return ret;
}

int genSequences(size_t totalCount)
{
	while(gAllocationList.size() > 0)
	{
		delete *(gAllocationList.rbegin());
		gAllocationList.pop_back();
	}
	for(size_t i = 0; i < ALLOC_COUNT; ++i)
	{
		TAllocation *ta = new TAllocation(rand() % (MAXIMUM_OBJECT_SIZE - MINIMUM_OBJECT_SIZE) + MINIMUM_OBJECT_SIZE);
		ta->idx = i;
		gAllocationList.push_back(ta);
	}

	gThreadActionSeq.clear();
	for(size_t i = 0; i < THREAD_COUNT; ++i)
	{
		std::list<TAction> l;
		gThreadActionSeq.push_back(l);
	}

	for(size_t i = 0; i < ALLOC_COUNT; ++i)
	{
		int idx = rand() % THREAD_COUNT;
		TAction act;
		act.action = Allocate;
		act.target = gAllocationList[i];
		gThreadActionSeq[idx].push_back(act);
		act.action = Deallocate;
		idx = rand() % THREAD_COUNT;
		gThreadActionSeq[idx].push_back(act);
	}


	// Print allocation/deallocation sequence
//	for(size_t idx = 0; idx < THREAD_COUNT; ++idx)
//	{
//		cout<<"T"<<idx<<" - ";
//		for(std::list<TAction>::iterator i = gThreadActionSeq[idx].begin(); i != gThreadActionSeq[idx].end(); i++)
//		{
//			if( i->action == Allocate )
//			{
//				cout<<"+"<<i->target->idx;
//				cout.flush();
//			}
//			else if( i->action == Deallocate )
//			{
//				cout<<"-"<<i->target->idx;;
//				cout.flush();
//			}
//		}
//		cout<<endl;
//	}

	return 0;
}

void showStat(ScalablePoolAllocator* alloc)
{
	ScalablePoolAllocator::AllocatorStat stat = alloc->getAllocatorStat();

	if(stat.StatAvailable == false)
	{
		LOG4CXX_INFO(logger, "Statistics unavailable.");
		return;
	}
	cout<<endl;
	cout<<"=========================================="<<endl;
	cout.setf(std::ios::right);
	cout<<" TotalAllocations         "<<setw(12)<<stat.TotalAllocations<<endl;
	cout<<" TotalDeallocations       "<<setw(12)<<stat.TotalDeallocations<<endl;
	cout<<" TotalSmallAllocations    "<<setw(12)<<stat.TotalSmallAllocations<<endl;
	cout<<" TotalSmallDeallocations  "<<setw(12)<<stat.TotalSmallDeallocations<<endl;
	cout<<" TotalLargeAllocations    "<<setw(12)<<stat.TotalLargeAllocations<<endl;
	cout<<" TotalLargeDeallocations  "<<setw(12)<<stat.TotalLargeDeallocations<<endl;
	cout<<" ChunksInUse              "<<setw(12)<<stat.ChunksInUse<<endl;
	cout<<" BlocksInUse              "<<setw(12)<<stat.BlocksInUse<<endl;
	cout<<" BlocksInFreeBlockStack   "<<setw(12)<<stat.BlocksInFreeBlockStack<<endl;
	cout<<" LargeChunkInFreeList     "<<setw(12)<<stat.LargeChunkInFreeList<<endl;
	cout<<" Underruns                "<<setw(12)<<stat.Underruns<<endl;
	cout<<" AllocationRecursion[err] "<<setw(12)<<stat.AllocationRecursion<<endl;
	cout<<" GarbageCollection        "<<setw(12)<<stat.GarbageCollection<<endl;
	cout<<"=========================================="<<endl;
	cout.flush();
}


void runTestSuite(ScalablePoolAllocator* alloc)
{
	ScalablePoolAllocator *sa = (ScalablePoolAllocator*)alloc;
	double sec = 0;
	unsigned long cnt = 0;
	cout<<"Simple alloc/dealloc test        ";
	for(int i = 0; i < REPEAT_COUNT; ++i)
	{
		sec += runTest(alloc);
		cnt += ALLOC_COUNT;
		cout<<".";
		if( (i+1) % 20 == 0 )
		{
			showStat(sa);
			cout<<sec<<" sec."<<endl;
			cout<<sec * 1000000.0 / double(cnt)<<" us per allocation"<<endl;
		}
		cout.flush();
	}
	cout<<endl;
	cout<<sec<<" sec."<<endl;
	cout<<sec * 1000000.0 / double(ALLOC_COUNT * REPEAT_COUNT)<<" us per allocation"<<endl;
	showStat(sa);
}


double runTest(ScalablePoolAllocator* alloc)
{
    tbb::tbb_thread *t[THREAD_COUNT] = {0};
    tbb::tick_count tBeg, tEnd;

    tBeg = tbb::tick_count::now();
    for(size_t i = 0; i < THREAD_COUNT; ++i)
    {
    	t[i] = new tbb::tbb_thread(bindT(runTestThread, (int)i, alloc));
    }
    for(size_t i = 0; i < THREAD_COUNT; ++i)
    {
    	t[i]->join();
    }
    tEnd = tbb::tick_count::now();
    for(size_t i = 0; i < THREAD_COUNT; ++i)
    {
    	delete t[i];
    }
	return (tEnd - tBeg).seconds();
}

int runTestThread(int idx, ScalablePoolAllocator* palloc)
{
	for(std::list<TAction>::iterator i = gThreadActionSeq[idx].begin(); i != gThreadActionSeq[idx].end(); i++)
	{
		if( i->action == Allocate )
		{
//			cout<<"+"<<i->target->idx;
//			cout.flush();
			i->target->data = palloc->allocate(i->target->sz);
			if(i->target->data)
			{
				i->target->allocated = true;

			}
			else
			{
				//ERROR
				i->target->allocated = true;
			}
		}
		else if( i->action == Deallocate )
		{
//			cout<<"-"<<i->target->idx;;
//			cout.flush();
			while( !(i->target->allocated) )
			{
				tbb::this_tbb_thread::yield();
			}
			palloc->deallocate(i->target->data);
			i->target->allocated = false;
		}
	}
	return 0;
}

