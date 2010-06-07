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

#ifndef ZILLIANS_ATOMIC_ATOMICSTACK_H_
#define ZILLIANS_ATOMIC_ATOMICSTACK_H_

#include "core-api/Atomic.h"

namespace zillians { namespace atomic {

struct stack_node
{
	stack_node() : _nexts(0)
	{ }

	stack_node* volatile _nexts;
};

/**
 * Simple Atomic Stack
 */
template<class T>
class stack
{
private:
	struct ptr_t
	{
		ptr_t() : _ptr(0), _pops(0)
		{ }

		ptr_t(stack_node* const ptr, const uint32 pops) : _ptr(ptr), _pops(pops)
		{ }

		union
		{
			struct
			{
				stack_node* volatile _ptr;
				volatile uint32 _pops;
			};
			volatile int64 _data;
		};
	};

	ptr_t _head;

public:
	stack() : _head()
	{ }

	void push(T * item)
	{
		stack_node* node = item;

		while(true)
		{
			node->_nexts = _head._ptr;
			if(b_cas_ptr(reinterpret_cast<void* volatile*> (&_head._ptr), node, node->_nexts))
				break;
		}
	}

	T* pop()
	{
		while(true)
		{
			const ptr_t head = _head;

			if(head._ptr == 0)
				return 0;

			const ptr_t next(head._ptr->_nexts, head._pops + 1);

			if(b_cas(&_head._data, next._data, head._data))
			{
				return static_cast<T*> (head._ptr);
			}
		}
	}
};

} }

#endif /* ZILLIANS_ATOMIC_ATOMICSTACK_H_ */
