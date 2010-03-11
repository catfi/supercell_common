#ifndef STDCONTAINERPERFORMANCETEST_H_
#define STDCONTAINERPERFORMANCETEST_H_

#include <map>
#include <ext/hash_map>
#include <ext/hash_set>
#include <tbb/tick_count.h>

// test std::map insertion and deletion performance (in same order)
void test_std_map_insert_search_delete_in_same_order(int iterations)
{
	std::map<int,int> m;
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			m[i] = i;
		}
	}
	end = tbb::tick_count::now();
	printf("\tinsertion takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			volatile std::map<int,int>::iterator it = m.find(i);
			it;
		}
	}
	end = tbb::tick_count::now();
	printf("\tsearch takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			m.erase(m.find(i));
		}
	}
	end = tbb::tick_count::now();
	printf("\tdeletion takes %lf ms\n", (end - start).seconds()*1000.0);
}

// test std::map insertion and deletion performance (in reversed order)
void test_std_map_insert_search_delete_in_reverse_order(int iterations)
{
	std::map<int,int> m;
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			m[i] = i;
		}
	}
	end = tbb::tick_count::now();
	printf("\tinsertion takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=iterations-1;i>=0;--i)
		{
			volatile std::map<int,int>::iterator it = m.find(i);
			it;
		}
	}
	end = tbb::tick_count::now();
	printf("\tsearch takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=iterations-1;i>=0;--i)
		{
			m.erase(m.find(i));
		}
	}
	end = tbb::tick_count::now();
	printf("\tdeletion takes %lf ms\n", (end - start).seconds()*1000.0);
}

// test __gnu_cxx::hash_map insertion and deletion performance (in same order)
void test_gnucxx_hash_map_insert_search_delete_in_same_order(int iterations)
{
	__gnu_cxx::hash_map<int,int> m;
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			m[i] = i;
		}
	}
	end = tbb::tick_count::now();
	printf("\tinsertion takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			volatile __gnu_cxx::hash_map<int,int>::iterator it = m.find(i);
			it;
		}
	}
	end = tbb::tick_count::now();
	printf("\tsearch takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			m.erase(m.find(i));
		}
	}
	end = tbb::tick_count::now();
	printf("\tdeletion takes %lf ms\n", (end - start).seconds()*1000.0);
}

// test __gnu_cxx::hash_map insertion and deletion performance (in reverse order)
void test_gnucxx_hash_map_insert_search_delete_in_reverse_order(int iterations)
{
	__gnu_cxx::hash_map<int,int> m;
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			m[i] = i;
		}
	}
	end = tbb::tick_count::now();
	printf("\tinsertion takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=iterations-1;i>=0;--i)
		{
			volatile __gnu_cxx::hash_map<int,int>::iterator it = m.find(i);
			it;
		}
	}
	end = tbb::tick_count::now();
	printf("\tsearch takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=iterations-1;i>=0;--i)
		{
			m.erase(m.find(i));
		}
	}
	end = tbb::tick_count::now();
	printf("\tdeletion takes %lf ms\n", (end - start).seconds()*1000.0);
}

// test __gnu_cxx::hash_set insertion and deletion performance (in same order)
void test_gnucxx_hash_set_insert_search_delete_in_same_order(int iterations)
{
	__gnu_cxx::hash_set<int> m;
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			m.insert(i);
		}
	}
	end = tbb::tick_count::now();
	printf("\tinsertion takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			volatile __gnu_cxx::hash_set<int>::iterator it = m.find(i);
			it;
		}
	}
	end = tbb::tick_count::now();
	printf("\tsearch takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			//m.erase(m.find(i));
			m.erase(i);
		}
	}
	end = tbb::tick_count::now();
	printf("\tdeletion takes %lf ms\n", (end - start).seconds()*1000.0);
}

// test __gnu_cxx::hash_set insertion and deletion performance (in reverse order)
void test_gnucxx_hash_set_insert_search_delete_in_reverse_order(int iterations)
{
	__gnu_cxx::hash_set<int> m;
	tbb::tick_count start, end;
	
	start = tbb::tick_count::now();
	{
		for(int i=0;i<iterations;++i)
		{
			m.insert(i);
		}
	}
	end = tbb::tick_count::now();
	printf("\tinsertion takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=iterations-1;i>=0;--i)
		{
			volatile __gnu_cxx::hash_set<int>::iterator it = m.find(i);
			it;
		}
	}
	end = tbb::tick_count::now();
	printf("\tsearch takes %lf ms\n", (end - start).seconds()*1000.0);
	
	start = tbb::tick_count::now();
	{
		for(int i=iterations-1;i>=0;--i)
		{
			//m.erase(m.find(i));
			m.erase(i);
		}
	}
	end = tbb::tick_count::now();
	printf("\tdeletion takes %lf ms\n", (end - start).seconds()*1000.0);
}

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <tbb/tbb_thread.h>
#include <tbb/spin_mutex.h>
#include <tbb/spin_rw_mutex.h>
#include <tbb/mutex.h>
#include <tbb/recursive_mutex.h>
#include <queue>

// test concurrent queue push/pop performance with spin_rw_mutex/spin_mutex/
template <typename mutex_type>
class test_std_queue
{
public:
	test_std_queue(int _iterations)
	{
		iterations = _iterations;
	}
	test_std_queue(const test_std_queue &obj) 
	{ 
		iterations = obj.iterations;
	}
	~test_std_queue()
	{
	}
	
public:
	int iterations;
	std::queue<int> q;
	tbb::tick_count start, end;
	mutex_type m;
public:
	void push_worker()
	{
		start = tbb::tick_count::now();
		int iter = iterations;
		for(int i=0;i<iter;++i)
		{
			typename mutex_type::scoped_lock(m);
			q.push(i);
		}
	}
	
	void pop_worker()
	{
		int iter = iterations;
		for(int i=0;i<iter;++i)
		{
			typename mutex_type::scoped_lock(m);
			if(!q.empty())
			{
				int result = q.front();
				q.pop();
			}
			else
			{
				--i;
			}
		}
		end = tbb::tick_count::now();
		printf("\tpush/pop takes %lf ms\n", (end - start).seconds()*1000.0);		
	}
};

void test_std_queue_push_pop_with_spin_rw_mutex(int iterations)
{
	test_std_queue<tbb::spin_rw_mutex> base(iterations);
	tbb::tbb_thread push_worker(boost::bind(&test_std_queue<tbb::spin_rw_mutex>::push_worker, &base));
	tbb::tbb_thread pop_worker(boost::bind(&test_std_queue<tbb::spin_rw_mutex>::pop_worker, &base));
	push_worker.join();
	pop_worker.join();
}

void test_std_queue_push_pop_with_spin_mutex(int iterations)
{
	test_std_queue<tbb::recursive_mutex> base(iterations);
	tbb::tbb_thread push_worker(boost::bind(&test_std_queue<tbb::recursive_mutex>::push_worker, &base));
	tbb::tbb_thread pop_worker(boost::bind(&test_std_queue<tbb::recursive_mutex>::pop_worker, &base));
	push_worker.join();
	pop_worker.join();
}

void test_std_queue_push_pop_with_mutex(int iterations)
{
	test_std_queue<tbb::mutex> base(iterations);
	tbb::tbb_thread push_worker(boost::bind(&test_std_queue<tbb::mutex>::push_worker, &base));
	tbb::tbb_thread pop_worker(boost::bind(&test_std_queue<tbb::mutex>::pop_worker, &base));
	push_worker.join();
	pop_worker.join();
}

void test_std_queue_push_pop_with_recursive_mutex(int iterations)
{
	test_std_queue<tbb::recursive_mutex> base(iterations);
	tbb::tbb_thread push_worker(boost::bind(&test_std_queue<tbb::recursive_mutex>::push_worker, &base));
	tbb::tbb_thread pop_worker(boost::bind(&test_std_queue<tbb::recursive_mutex>::pop_worker, &base));
	push_worker.join();
	pop_worker.join();
}

void test_std_queue_push_pop_with_boost_mutex(int iterations)
{
	test_std_queue<boost::mutex> base(iterations);
	tbb::tbb_thread push_worker(boost::bind(&test_std_queue<boost::mutex>::push_worker, &base));
	tbb::tbb_thread pop_worker(boost::bind(&test_std_queue<boost::mutex>::pop_worker, &base));
	push_worker.join();
	pop_worker.join();
}

#endif /*STDCONTAINERPERFORMANCETEST_H_*/
