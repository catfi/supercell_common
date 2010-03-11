// 
// Zillians MMO
// Copyright (C) 2007-2008 Zillians.com, Inc.
// For more information see http://www.zillians.com
//
// Zillians MMO is the library and runtime for massive multiplayer online game
// development in utility computing model, which runs as a service for every 
// developer to build their virtual world running on our GPU-assisted machines
//
// This is a close source library intended to be used solely within Zillians.com
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
//
// Contact Information: info@zillians.com
//

#define TEST_STLPORT 0

#include "core-api/Types.h"

#if TEST_STLPORT
	#include "STLPortContainerPerformance.h"
#else
	#include "STDContainerPerformanceTest.h"
	#include "BoostContainerPerformanceTest.h"
	#include "TBBContainerPerformanceTest.h"
#endif

#define ITERATION_COUNT 1
#define ELEMENT_COUNT 20000
int main(int argc, char** argv)
{
	printf("ITERATION_COUNT = %d, ELEMENT_COUNT = %d\n", ITERATION_COUNT, ELEMENT_COUNT);
	
#if TEST_STLPORT	
	printf("[test_stlport_hash_map_insert_search_delete_in_same_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_stlport_hash_map_insert_search_delete_in_same_order(ELEMENT_COUNT);
	}
	
	printf("[test_stlport_hash_map_insert_search_delete_in_reverse_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_stlport_hash_map_insert_search_delete_in_reverse_order(ELEMENT_COUNT);
	}	
	
#else
	printf("[test_std_map_insert_delete_in_same_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_std_map_insert_search_delete_in_same_order(ELEMENT_COUNT);
	}
	
	printf("[test_std_map_insert_delete_in_reverse_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_std_map_insert_search_delete_in_reverse_order(ELEMENT_COUNT);
	}
	
	printf("[test_gnucxx_hash_map_insert_search_delete_in_same_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_gnucxx_hash_map_insert_search_delete_in_same_order(ELEMENT_COUNT);
	}
	
	printf("[test_gnucxx_hash_map_insert_search_delete_in_reverse_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_gnucxx_hash_map_insert_search_delete_in_reverse_order(ELEMENT_COUNT);
	}	
	
	printf("[test_gnucxx_hash_set_insert_search_delete_in_same_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_gnucxx_hash_set_insert_search_delete_in_same_order(ELEMENT_COUNT);
	}
	
	printf("[test_gnucxx_hash_set_insert_search_delete_in_reverse_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_gnucxx_hash_set_insert_search_delete_in_reverse_order(ELEMENT_COUNT);
	}	
	
	printf("[test_boost_unordered_map_insert_delete_in_same_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_boost_unordered_map_insert_search_delete_in_same_order(ELEMENT_COUNT);
	}
	
	printf("[test_boost_unordered_map_insert_delete_in_reverse_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_boost_unordered_map_insert_search_delete_in_reverse_order(ELEMENT_COUNT);
	}	
	
	printf("[test_boost_unordered_set_insert_delete_in_same_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_boost_unordered_set_insert_search_delete_in_same_order(ELEMENT_COUNT);
	}	

	printf("[test_boost_unordered_set_insert_delete_in_reverse_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_boost_unordered_set_insert_search_delete_in_reverse_order(ELEMENT_COUNT);
	}	
	
	printf("[test_tbb_concurrent_hash_map_insert_search_delete_in_same_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_tbb_concurrent_hash_map_insert_search_delete_in_same_order(ELEMENT_COUNT);
	}
	
	printf("[test_tbb_concurrent_hash_map_insert_search_delete_in_reverse_order]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_tbb_concurrent_hash_map_insert_search_delete_in_reverse_order(ELEMENT_COUNT);
	}
	
	printf("[test_concurrent_queue_push_pop]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_concurrent_queue_push_pop(ELEMENT_COUNT);
	}
	
#endif	
	/*
	printf("[test_my_concurrent_queue_push_pop]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_my_concurrent_queue_push_pop(ELEMENT_COUNT);
	}
	*/
	/*
	printf("[test_std_queue_push_pop_with_boost_mutex]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_std_queue_push_pop_with_boost_mutex(ELEMENT_COUNT);
	}
	
	printf("[test_std_queue_push_pop_with_mutex]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_std_queue_push_pop_with_mutex(ELEMENT_COUNT);
	}		
	
	printf("[test_std_queue_push_pop_with_spin_rw_mutex]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_std_queue_push_pop_with_spin_rw_mutex(ELEMENT_COUNT);
	}

	printf("[test_std_queue_push_pop_with_spin_mutex]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_std_queue_push_pop_with_spin_mutex(ELEMENT_COUNT);
	}
		

	
	printf("[test_std_queue_push_pop_with_recursive_mutex]\n");
	for(int i=0;i<ITERATION_COUNT;++i)
	{
		test_std_queue_push_pop_with_recursive_mutex(ELEMENT_COUNT);
	}	
	*/
	
	return 0;
}
