#ifndef STLPORTCONTAINERPERFORMANCE_H_
#define STLPORTCONTAINERPERFORMANCE_H_

#include <stlport/hash_map>
#include <tbb/tick_count.h>

// test stlport::hash_map insertion and deletion performance (in same order)
void test_stlport_hash_map_insert_search_delete_in_same_order(int iterations)
{
	std::hash_map<int,int> m;
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
			volatile std::hash_map<int,int>::iterator it = m.find(i);
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

void test_stlport_hash_map_insert_search_delete_in_reverse_order(int iterations)
{
	std::hash_map<int,int> m;
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
			volatile std::hash_map<int,int>::iterator it = m.find(i);
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


#endif /*STLPORTCONTAINERPERFORMANCE_H_*/
