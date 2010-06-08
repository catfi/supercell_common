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
 * @date Jun 4, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_ATOMIC_ATOMICQUEUE_H_
#define ZILLIANS_ATOMIC_ATOMICQUEUE_H_

#include "core-api/Atomic.h"
#include "core-api/AtomicStack.h"
#include <tbb/atomic.h>

namespace zillians { namespace atomic {

#if 0
struct node;

struct node_ptr_t
{
	explicit node_ptr_t(node* const p) : _ptr(p), _tag(0)
	{ }

	node_ptr_t(node* const p, const int32 t) : _ptr(p), _tag(t)
	{ }

	node_ptr_t() : _ptr(0), _tag(0)
	{ }

	inline void set(node* const p, const int32 t)
	{
		_ptr = p;
		_tag = t;
	}

	inline bool operator == (const node_ptr_t & p)
	{
		return (_ptr == p._ptr) && (_tag == p._tag);
	}

	inline bool operator != (const node_ptr_t & p)
	{
		return !(*this == (p));
	}

	union
	{
		struct
		{
			node * volatile _ptr;
			volatile unsigned int _tag;
		};
		volatile int64 _data;
	};
};

struct node
{
	node() : _next(0), _prev(0), _dummy(false)
	{ }

	node(const bool d) : _next(0), _prev(0), _dummy(d)
	{ }

	node_ptr_t _next;
	node_ptr_t _prev;
	bool _dummy;
};

struct dummy_node: public node, public stack_node
{
	dummy_node() : node(true)
	{ }
};

typedef node node_t;

/**
 * Atomic Double Linked List FIFO Queue
 *
 * Implementation of Edya Ladan Mozes's paper titled "An Optimistic Approach to Lock-Free
 * FIFO Queues", published on Distributed Computing, Volume 20, number 5, February 2008.
 *
 * As far as we know, this is the fastest concurrent queue implementation using spin wait
 * in human being history.
 *
 * Reference: http://people.csail.mit.edu/edya/publications/OptimisticFIFOQueue-journal.pdf
 */
template<class T>
class queue
{
public:
	node_ptr_t _tail;
	node_ptr_t _head;

protected:
	stack<dummy_node> _dpool;

protected:
	inline void fix_list(node_ptr_t& tail, node_ptr_t& head)
	{
		node_ptr_t cur_node, cur_node_next, next_node_prev;

		cur_node = tail;

		while((head == _head) && (cur_node != head))
		{
			cur_node_next = cur_node._ptr->_next;

			if(cur_node_next._tag != cur_node._tag)
				return;

			next_node_prev = cur_node_next._ptr->_prev;
			if(next_node_prev != node_ptr_t(cur_node._ptr, cur_node._tag - 1))
				cur_node_next._ptr->_prev.set(cur_node._ptr, cur_node._tag - 1);

			cur_node.set(cur_node_next._ptr, cur_node._tag - 1);
		};
	}

public:
	queue() : _tail(new dummy_node()), _head(_tail._ptr), _dpool()
	{ }

	~queue()
	{
		node_t* p;
		while ((p = _dpool.pop()) != 0)
			delete p;
	}

	void push(T * const item)
	{
		node_ptr_t tail;
		node_t* const new_node = item;

		new_node->_prev.set(0, 0);

		while(true)
		{
			tail = _tail;
			new_node->_next.set(tail._ptr, tail._tag + 1);
			if(b_cas(&_tail._data, node_ptr_t(new_node, tail._tag + 1)._data, tail._data))
			{
				tail._ptr->_prev.set(new_node, tail._tag);
				return;
			}
		}
	}

	T* pop()
	{
		node_ptr_t head, tail;
		dummy_node* dummy;

		while(true)
		{
			head = _head;
			tail = _tail;
			if(head == _head)
			{
				if(!head._ptr->_dummy)
				{
					if(tail != head)
					{
						if(head._ptr->_prev._tag != head._tag)
						{
							fix_list(tail, head);
							continue;
						}
					}
					else
					{
						dummy = _dpool.pop();

						if(dummy == NULL)
						{
							dummy = new dummy_node();
						}

						dummy->_next.set(tail._ptr, tail._tag + 1);

						if(b_cas(&_tail._data, node_ptr_t(dummy, tail._tag + 1)._data, tail._data))
						{
							head._ptr->_prev.set(dummy, tail._tag);
						}
						else
						{
							_dpool.push(dummy);
						}

						continue;
					}

					if(b_cas(&_head._data, node_ptr_t(head._ptr->_prev._ptr, head._tag + 1)._data, head._data))
					{
						return static_cast<T*> (head._ptr);
					}
				}
				else
				{
					if(tail._ptr == head._ptr)
					{
						return NULL;
					}
					else
					{
						if(head._ptr->_prev._tag != head._tag)
						{
							fix_list(tail, head);
							continue;
						}
						if(b_cas(&_head._data, node_ptr_t(head._ptr->_prev._ptr, head._tag + 1)._data, head._data))
						{
							_dpool.push(static_cast<dummy_node*> (head._ptr));
						}
					}
				}
			}
		}
	}
};

#else

/**
 * @date Jun 8, 2010 zac - Initial version created.
 *
 * AtomicQueue is based on yqueue.
 */
//  yqueue is an efficient queue implementation. The main goal is
//  to minimise number of allocations/deallocations needed. Thus yqueue
//  allocates/deallocates elements in batches of N.
//
//  yqueue allows one thread to use push/back function and another one
//  to use pop/front functions. However, user must ensure that there's no
//  pop on the empty queue and that both threads don't access the same
//  element in unsynchronised manner.
//
//  T is the type of the object in the queue.
//  N is granularity of the queue (how many pushes have to be done till
//  actual memory allocation is required).

template <typename T, int N> class AtomicQueue
{
public:

    //  Create the queue.
    inline AtomicQueue ()
    {
         begin_chunk = (chunk_t*) malloc (sizeof (chunk_t));
         BOOST_ASSERT (begin_chunk);
         begin_pos = 0;
         back_chunk = NULL;
         back_pos = 0;
         end_chunk = begin_chunk;
         end_pos = 0;
         spare_chunk = NULL;
    }

    //  Destroy the queue.
    inline ~AtomicQueue ()
    {
        while (true) {
            if (begin_chunk == end_chunk) {
                free (begin_chunk);
                break;
            }
            chunk_t *o = begin_chunk;
            begin_chunk = begin_chunk->next;
            free (o);
        }

//        chunk_t *sc = spare_chunk.xchg (NULL);
        chunk_t *sc = spare_chunk.fetch_and_store(NULL);
        if (sc)
            free (sc);
    }

    //  Returns reference to the front element of the queue.
    //  If the queue is empty, behaviour is undefined.
    inline T &front ()
    {
         return begin_chunk->values [begin_pos];
    }

    //  Returns reference to the back element of the queue.
    //  If the queue is empty, behaviour is undefined.
    inline T &back ()
    {
        return back_chunk->values [back_pos];
    }

    //  Adds an element to the back end of the queue.
    inline void push ()
    {
        back_chunk = end_chunk;
        back_pos = end_pos;

        if (++end_pos != N)
            return;

//        chunk_t *sc = spare_chunk.xchg (NULL);
        chunk_t *sc = spare_chunk.fetch_and_store(NULL);
        if (sc) {
            end_chunk->next = sc;
            sc->prev = end_chunk;
        } else {
            end_chunk->next = (chunk_t*) malloc (sizeof (chunk_t));
            BOOST_ASSERT (end_chunk->next);
            end_chunk->next->prev = end_chunk;
        }
        end_chunk = end_chunk->next;
        end_pos = 0;
    }

    //  Removes element from the back end of the queue. In other words
    //  it rollbacks last push to the queue. Take care: Caller is
    //  responsible for destroying the object being unpushed.
    //  The caller must also guarantee that the queue isn't empty when
    //  unpush is called. It cannot be done automatically as the read
    //  side of the queue can be managed by different, completely
    //  unsynchronised thread.
    inline void unpush ()
    {
        //  First, move 'back' one position backwards.
        if (back_pos)
            --back_pos;
        else {
            back_pos = N - 1;
            back_chunk = back_chunk->prev;
        }

        //  Now, move 'end' position backwards. Note that obsolete end chunk
        //  is not used as a spare chunk. The analysis shows that doing so
        //  would require free and atomic operation per chunk deallocated
        //  instead of a simple free.
        if (end_pos)
            --end_pos;
        else {
            end_pos = N - 1;
            end_chunk = end_chunk->prev;
            free (end_chunk->next);
            end_chunk->next = NULL;
        }
    }

    //  Removes an element from the front end of the queue.
    inline void pop ()
    {
        if (++ begin_pos == N) {
            chunk_t *o = begin_chunk;
            begin_chunk = begin_chunk->next;
            begin_chunk->prev = NULL;
            begin_pos = 0;

            //  'o' has been more recently used than spare_chunk,
            //  so for cache reasons we'll get rid of the spare and
            //  use 'o' as the spare.
//            chunk_t *cs = spare_chunk.xchg (o);
            chunk_t *cs = spare_chunk.fetch_and_store(o);
            if (cs)
                free (cs);
        }
    }

private:

    //  Individual memory chunk to hold N elements.
    struct chunk_t
    {
         T values [N];
         chunk_t *prev;
         chunk_t *next;
    };

    //  Back position may point to invalid memory if the queue is empty,
    //  while begin & end positions are always valid. Begin position is
    //  accessed exclusively be queue reader (front/pop), while back and
    //  end positions are accessed exclusively by queue writer (back/push).
    chunk_t *begin_chunk;
    int begin_pos;
    chunk_t *back_chunk;
    int back_pos;
    chunk_t *end_chunk;
    int end_pos;

    //  People are likely to produce and consume at similar rates.  In
    //  this scenario holding onto the most recently freed chunk saves
    //  us from having to call malloc/free.
//    atomic_ptr_t<chunk_t> spare_chunk;
    tbb::atomic<chunk_t*> spare_chunk;

    //  Disable copying of yqueue.
    AtomicQueue (const AtomicQueue&);
    void operator = (const AtomicQueue&);
};


/**
 * AtomicPipe
 */
//  Lock-free queue implementation.
//  Only a single thread can read from the pipe at any specific moment.
//  Only a single thread can write to the pipe at any specific moment.
//  T is the type of the object in the queue.
//  N is granularity of the pipe, i.e. how many items are needed to
//  perform next memory allocation.

template <typename T, int N> class AtomicPipe
{
public:

    //  Initialises the pipe.
    inline AtomicPipe ()
    {
        //  Insert terminator element into the queue.
        queue.push ();

        //  Let all the pointers to point to the terminator.
        //  (unless pipe is dead, in which case c is set to NULL).
        r = w = f = &queue.back ();
//        c.set (&queue.back ());
        c = &queue.back();
    }

    //  Following function (write) deliberately copies uninitialised data
    //  when used with zmq_msg. Initialising the VSM body for
    //  non-VSM messages won't be good for performance.

//#ifdef ZMQ_HAVE_OPENVMS
//#pragma message save
//#pragma message disable(UNINIT)
//#endif

    //  Write an item to the pipe.  Don't flush it yet. If incomplete is
    //  set to true the item is assumed to be continued by items
    //  subsequently written to the pipe. Incomplete items are never
    //  flushed down the stream.
    inline void write (const T &value_, bool incomplete_)
    {
        //  Place the value to the queue, add new terminator element.
        queue.back () = value_;
        queue.push ();

        //  Move the "flush up to here" poiter.
        if (!incomplete_)
            f = &queue.back ();
    }

//#ifdef ZMQ_HAVE_OPENVMS
//#pragma message restore
//#endif

    //  Pop an incomplete item from the pipe. Returns true is such
    //  item exists, false otherwise.
    inline bool unwrite (T *value_)
    {
        if (f == &queue.back ())
            return false;
        queue.unpush ();
        *value_ = queue.back ();
        return true;
    }

    //  Flush all the completed items into the pipe. Returns false if
    //  the reader thread is sleeping. In that case, caller is obliged to
    //  wake the reader up before using the pipe again.
    inline bool flush ()
    {
        //  If there are no un-flushed items, do nothing.
        if (w == f)
            return true;

        //  Try to set 'c' to 'f'.
//        if (c.cas (w, f) != w) {
        if(c.compare_and_swap(w, f) != w) {

            //  Compare-and-swap was unseccessful because 'c' is NULL.
            //  This means that the reader is asleep. Therefore we don't
            //  care about thread-safeness and update c in non-atomic
            //  manner. We'll return false to let the caller know
            //  that reader is sleeping.
//            c.set (f);
        	c = f;
            w = f;
            return false;
        }

        //  Reader is alive. Nothing special to do now. Just move
        //  the 'first un-flushed item' pointer to 'f'.
        w = f;
        return true;
    }

    //  Check whether item is available for reading.
    inline bool check_read ()
    {
        //  Was the value prefetched already? If so, return.
        if (&queue.front () != r)
             return true;

        //  There's no prefetched value, so let us prefetch more values.
        //  Prefetching is to simply retrieve the
        //  pointer from c in atomic fashion. If there are no
        //  items to prefetch, set c to NULL (using compare-and-swap).
//        r = c.cas (&queue.front (), NULL);
        r = c.compare_and_swap(&queue.front(), NULL);

        //  If there are no elements prefetched, exit.
        //  During pipe's lifetime r should never be NULL, however,
        //  it can happen during pipe shutdown when items
        //  are being deallocated.
        if (&queue.front () == r || !r)
            return false;

        //  There was at least one value prefetched.
        return true;
    }

    //  Reads an item from the pipe. Returns false if there is no value.
    //  available.
    inline bool read (T *value_)
    {
        //  Try to prefetch a value.
        if (!check_read ())
            return false;

        //  There was at least one value prefetched.
        //  Return it to the caller.
        *value_ = queue.front ();
        queue.pop ();
        return true;
    }

protected:

    //  Allocation-efficient queue to store pipe items.
    //  Front of the queue points to the first prefetched item, back of
    //  the pipe points to last un-flushed item. Front is used only by
    //  reader thread, while back is used only by writer thread.
    AtomicQueue <T, N> queue;

    //  Points to the first un-flushed item. This variable is used
    //  exclusively by writer thread.
    T *w;

    //  Points to the first un-prefetched item. This variable is used
    //  exclusively by reader thread.
    T *r;

    //  Points to the first item to be flushed in the future.
    T *f;

    //  The single point of contention between writer and reader thread.
    //  Points past the last flushed item. If it is NULL,
    //  reader is asleep. This pointer should be always accessed using
    //  atomic operations.
//    atomic_ptr_t <T> c;
    tbb::atomic<T*> c;

    //  Disable copying of ypipe object.
    AtomicPipe (const AtomicPipe&);
    void operator = (const AtomicPipe&);
};

#endif

} }

#endif /* ATOMICQUEUE_H_ */
