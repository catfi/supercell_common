#ifndef TBBCONTAINERPERFORMANCETEST_H_
#define TBBCONTAINERPERFORMANCETEST_H_

#include <tbb/concurrent_hash_map.h>
#include <tbb/tick_count.h>

struct MyHashCompare {
    static size_t hash( const int& x ) {
        // return x % 100; // SLOWER
    	return x; // FASTER
    }
    //! True if strings are equal
    static bool equal( const int& x, const int& y ) {
        return x==y;
    }
};


// test tbb::concurrent_hash_map insertion and deleltion performance (in same order)
void test_tbb_concurrent_hash_map_insert_search_delete_in_same_order(int iterations)
{
	tbb::concurrent_hash_map<int,int,MyHashCompare> m;
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		tbb::concurrent_hash_map<int,int,MyHashCompare>::accessor a;
		for(int i=0;i<iterations;++i)
		{
			m.insert(a, i);
			a->second = i;
		}
	}
	end = tbb::tick_count::now();
	printf("\tinsertion takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		//tbb::concurrent_hash_map<int,int,MyHashCompare>::const_accessor a;
		tbb::concurrent_hash_map<int,int,MyHashCompare>::accessor a;
		for(int i=0;i<iterations;++i)
		{
			m.find(a, i);
			a;
		}
	}
	end = tbb::tick_count::now();
	printf("\tsearch takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		tbb::concurrent_hash_map<int,int,MyHashCompare>::accessor a;
		for(int i=0;i<iterations;++i)
		{
			//m.erase(i); // FASTER
			if(m.find(a, i)) m.erase(a); // SLOWER
		}
	}
	end = tbb::tick_count::now();
	printf("\tdeletion takes %lf ms\n", (end - start).seconds()*1000.0);
}

// test tbb::concurrent_hash_map insertion and deleltion performance (in reversed order)
void test_tbb_concurrent_hash_map_insert_search_delete_in_reverse_order(int iterations)
{
	tbb::concurrent_hash_map<int,int,MyHashCompare> m;
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		tbb::concurrent_hash_map<int,int,MyHashCompare>::accessor a;
		for(int i=0;i<iterations;++i)
		{
			m.insert(a, i);
			a->second = i;
		}
	}
	end = tbb::tick_count::now();
	printf("\tinsertion takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		//tbb::concurrent_hash_map<int,int,MyHashCompare>::const_accessor a;
		tbb::concurrent_hash_map<int,int,MyHashCompare>::accessor a;
		for(int i=iterations-1;i>=0;--i)
		{
			m.find(a, i);
			a;
		}
	}
	end = tbb::tick_count::now();
	printf("\tsearch takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		tbb::concurrent_hash_map<int,int,MyHashCompare>::accessor a;
		for(int i=iterations-1;i>=0;--i)
		{
			//m.erase(i); // FASTER
			if(m.find(a, i)) m.erase(a); // SLOWER
		}
	}
	end = tbb::tick_count::now();
	printf("\tdeletion takes %lf ms\n", (end - start).seconds()*1000.0);
}

// test tbb::concurrent_queue concurrent push/pop performance
#include <boost/bind.hpp>
#include <tbb/tbb_thread.h>
#include <tbb/concurrent_queue.h>

class test_concurrent_queue
{
public:
	test_concurrent_queue(int _iterations)
	{
		iterations = _iterations;
	}
	test_concurrent_queue(const test_concurrent_queue &obj) 
	{ 
		iterations = obj.iterations;
	}
	~test_concurrent_queue()
	{
		q.clear();
	}
	
public:
	int iterations;
	tbb::concurrent_queue<int> q;
	tbb::tick_count start, end;
public:
	void push_worker()
	{
		start = tbb::tick_count::now();
		int iter = iterations;
		for(int i=0;i<iter;++i)
		{
			q.push(i);
		}
	}
	
	void pop_worker()
	{
		int iter = iterations;
		for(int i=0;i<iter;++i)
		{
			int result;
			q.pop(result);
		}
		end = tbb::tick_count::now();
		printf("\tpush/pop takes %lf ms\n", (end - start).seconds()*1000.0);		
	}
};

void test_concurrent_queue_push_pop(int iterations)
{
	test_concurrent_queue base(iterations);
	tbb::tbb_thread push_worker(boost::bind(&test_concurrent_queue::push_worker, &base));
	tbb::tbb_thread pop_worker(boost::bind(&test_concurrent_queue::pop_worker, &base));
	push_worker.join();
	pop_worker.join();
}


/*
#include "core-api/ConcurrentQueue.h"

class test_my_concurrent_queue
{
public:
	test_my_concurrent_queue(int _iterations)
	{
		iterations = _iterations;
	}
	test_my_concurrent_queue(const test_concurrent_queue &obj) 
	{ 
		iterations = obj.iterations;
	}
	~test_my_concurrent_queue()
	{
	}
	
public:
	int iterations;
	ConcurrentQueue<int> q;
	tbb::tick_count start, end;
public:
	void push_worker()
	{
		start = tbb::tick_count::now();
		int iter = iterations;
		for(int i=0;i<iter;++i)
		{
			q.push(i);
		}
	}
	
	void pop_worker()
	{
		int iter = iterations;
		for(int i=0;i<iter;++i)
		{
			int result;
			q.wait_and_pop(result);
		}
		end = tbb::tick_count::now();
		printf("\tpush/pop takes %lf ms\n", (end - start).seconds()*1000.0);		
	}
};

void test_my_concurrent_queue_push_pop(int iterations)
{
	test_my_concurrent_queue base(iterations);
	tbb::tbb_thread push_worker(boost::bind(&test_my_concurrent_queue::push_worker, &base));
	tbb::tbb_thread pop_worker(boost::bind(&test_my_concurrent_queue::pop_worker, &base));
	push_worker.join();
	pop_worker.join();
}
*/

#endif /*TBBCONTAINERPERFORMANCETEST_H_*/
