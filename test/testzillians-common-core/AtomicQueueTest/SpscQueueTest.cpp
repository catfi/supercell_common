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
 * @date Jun 8, 2010 zac - Initial version created.
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <tbb/tick_count.h>
#include <tbb/tbb_thread.h>
//#include <boost/thread.hpp>
//#include <boost/timer.hpp>
#include <boost/assert.hpp>
#include "core/Prerequisite.h"

// load with 'consume' (data-dependent) memory ordering
	template<typename T>
T load_consume(T const* addr)
{
	// hardware fence is implicit on x86
	T v = *const_cast<T const volatile*>(addr);
	//__memory_barrier(); // compiler fence
	__sync_synchronize();
	return v;
}

// store with 'release' memory ordering
	template<typename T>
void store_release(T* addr, T v)
{
	// hardware fence is implicit on x86
	//__memory_barrier(); // compiler fence
	__sync_synchronize();
	*const_cast<T volatile*>(addr) = v;
}

// cache line size on modern x86 processors (in bytes)
const std::size_t cache_line_size = 64;

// single-producer/single-consumer queue
template<typename T>
class spsc_queue
{
	public:
		spsc_queue()
		{
			node* n = new node;
			n->next_ = 0;
			tail_ = head_ = first_= tail_copy_ = n;
		}

		~spsc_queue()
		{
			node* n = first_;
			do
			{
				node* next = n->next_;
				delete n;
				n = next;
			}
			while (n);
		}

		void enqueue(T v)
		{
			node* n = alloc_node();
			n->next_ = 0;
			n->value_ = v;
			store_release(&head_->next_, n);
			head_ = n;
		}

		// returns 'false' if queue is empty
		bool dequeue(T& v)
		{
			if (load_consume(&tail_->next_))
			{
				v = tail_->next_->value_;
				store_release(&tail_, tail_->next_);
				return true;
			}
			else
			{
				return false;
			}
		}

	private:
		// internal node structure
		struct node
		{
			node* next_;
			T value_;
		};

		// consumer part
		// accessed mainly by consumer, infrequently be producer
		node* tail_; // tail of the queue

		// delimiter between consumer part and producer part,
		// so that they situated on different cache lines
		char cache_line_pad_ [cache_line_size];

		// producer part
		// accessed only by producer
		node* head_; // head of the queue
		node* first_; // last unused node (tail of node cache)
		node* tail_copy_; // helper (points somewhere between first_ and tail_)

		node* alloc_node()
		{
			// first tries to allocate node from internal node cache,
			// if attempt fails, allocates node via ::operator new()

			if (first_ != tail_copy_)
			{
				node* n = first_;
				first_ = first_->next_;
				return n;
			}
			tail_copy_ = load_consume(&tail_);
			if (first_ != tail_copy_)
			{
				node* n = first_;
				first_ = first_->next_;
				return n;
			}
			node* n = new node;
			return n;
		}

		spsc_queue(spsc_queue const&);
		spsc_queue& operator = (spsc_queue const&);
};

void writer(spsc_queue<int>* q, int n)
{
	for(int i=0;i<n;++i)
	{
		q->enqueue(i);
	}
}

void reader(spsc_queue<int>* q, int n)
{
	int v;
	for(int i=0;i<n;++i)
	{
		while(!q->dequeue(v));
		BOOST_ASSERT(v == i);
	}
}

// usage example
int main(int argc, char** argv)
{
	int n = atoi(argv[1]);
	spsc_queue<int> q;

	tbb::tick_count start, end;
	start = tbb::tick_count::now();
//	boost::timer timer;
//	timer.restart();
//	boost::thread tr(boost::bind(reader, &q, n));
//	boost::thread tw(boost::bind(, &q, n));
	tbb::tbb_thread tw(boost::bind(&writer, &q, n));
	tbb::tbb_thread tr(boost::bind(&reader, &q, n));

	tr.join();
	tw.join();

	end = tbb::tick_count::now();
	float total = (end - start).seconds()*1000.0;

//	double elapsed = timer.elapsed();
	printf("enqueue/dequeue %d elements in %f ms\n", n, total);

	return 0;
}
