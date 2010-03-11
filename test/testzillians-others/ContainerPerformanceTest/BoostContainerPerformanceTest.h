#ifndef BOOSTCONTAINERPERFORMANCETEST_H_
#define BOOSTCONTAINERPERFORMANCETEST_H_

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <tbb/tick_count.h>

// test boost::unordered_map insertion and deletion performance (in same order)
void test_boost_unordered_map_insert_search_delete_in_same_order(int iterations)
{
	boost::unordered_map<int,int> m;
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
			volatile boost::unordered_map<int,int>::iterator it = m.find(i);
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

// test boost::unordered_map insertion and deletion performance (in reversed order)
void test_boost_unordered_map_insert_search_delete_in_reverse_order(int iterations)
{
	boost::unordered_map<int,int> m;
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
			volatile boost::unordered_map<int,int>::iterator it = m.find(i);
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

// test boost::unordered_set insertion and deletion performance (in same order)
void test_boost_unordered_set_insert_search_delete_in_same_order(int iterations)
{
	boost::unordered_set<int> m;
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
			volatile boost::unordered_set<int>::iterator it = m.find(i);
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

// test boost::unordered_set insertion and deletion performance (in reversed order)
void test_boost_unordered_set_insert_search_delete_in_reverse_order(int iterations)
{
	boost::unordered_set<int> m;
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
			volatile boost::unordered_set<int>::iterator it = m.find(i);
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

#endif /*BOOSTCONTAINERPERFORMANCETEST_H_*/
