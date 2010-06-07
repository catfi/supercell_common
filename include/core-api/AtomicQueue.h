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

namespace zillians { namespace atomic {

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

public:
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

			next_node_prev = cur_nodeNext._ptr->_prev;
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

} }

#endif /* ATOMICQUEUE_H_ */
