/**
 * Zillians MMO
 * Copyright (C) 2007-2009 Zillians.com, Inc.
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

#ifndef ZILLIANS_OBJECTPOOL_H_
#define ZILLIANS_OBJECTPOOL_H_

#include "core/Common.h"
#include <tbb/concurrent_queue.h>

#define ZILLIANS_OBJPOOL_PREFERABLE_POOL_SIZE     0
#define ZILLIANS_OBJPOOL_ENABLE_OBJ_POOL_COUNTER  0

namespace zillians {

/**
 * @brief ObjectPool is a simple object pooling template.
 *
 * @note ObjectPool does not support concurrent new/delete on the same
 * type of object, use ConcurrentObjectPool for that case.
 *
 * @see ConcurrentObjectPool
 */
template <typename T>
class ObjectPool
{
public:
	ObjectPool()
	{ }

	virtual ~ObjectPool()
	{ }

public:
 	static void* operator new(size_t size)
	{
#if ZILLIANS_OBJPOOL_ENABLE_OBJ_POOL_COUNTER
		++mPool.allocationCount;
#endif
 		if(mPool.allocations.empty())
 		{
 			return ::operator new(size);
 		}
 		else
 		{
 	 		void* obj = mPool.allocations.front();
 	 		mPool.allocations.pop();
 	 		return obj;
 		}
	}

	static void operator delete(void* p)
	{
#if ZILLIANS_OBJPOOL_ENABLE_OBJ_POOL_COUNTER
		--mPool.allocationCount;
#endif
		if(ZILLIANS_OBJPOOL_PREFERABLE_POOL_SIZE > 0 && mPool.allocations.size() > ZILLIANS_OBJPOOL_PREFERABLE_POOL_SIZE)
		{
			::operator delete(p);
		}
		else
		{
			mPool.allocations.push(p);
		}
	}

	static void purge()
	{
		while(!mPool.allocations.empty())
		{
			void* obj = mPool.allocations.front();
			::operator delete(obj);
			mPool.allocations.pop();
#if ZILLIANS_OBJPOOL_ENABLE_OBJ_POOL_COUNTER
			--mPool.allocationCount;
#endif
		}
	}

protected:
	/**
	 * This is used to ensure the ObjectPool<T>::purge() is called before
	 * destroying the internal object pool container.
	 */
	struct AutoPoolImpl
	{
		AutoPoolImpl() { }
		~AutoPoolImpl() { ObjectPool<T>::purge(); }
#if ZILLIANS_OBJPOOL_ENABLE_OBJ_POOL_COUNTER
		tbb::atomic<long> allocationCount;
#endif
		std::queue<void*> allocations;
	};
	static AutoPoolImpl mPool;
};

template<typename T> typename ObjectPool<T>::AutoPoolImpl ObjectPool<T>::mPool;

/**
 * ConcurrentObjectPool is a simple object pooling template supporting
 * concurrent allocations and deallocations.
 *
 * @note Since ConcurrentObjectPool does support concurrent new/delete on
 * the same type of object, it also brings some overhead while managing
 * the internal queue. Use it with performance caution.
 *
 * @see ObjectPool
 */
template <typename T>
class ConcurrentObjectPool
{
public:
	ConcurrentObjectPool()
	{ }

	virtual ~ConcurrentObjectPool()
	{ }

public:
 	static void* operator new(size_t size)
	{
#if ZILLIANS_OBJPOOL_ENABLE_OBJ_POOL_COUNTER
		++mPool.allocationCount;
#endif
 		void* obj = NULL;
 		mPool.allocations.try_pop(obj);

		if(obj)
		{
			return obj;
		}
		else
		{
			return ::operator new(size);
		}
	}

	static void operator delete(void* p)
	{
#if ZILLIANS_OBJPOOL_ENABLE_OBJ_POOL_COUNTER
		--mPool.allocationCount;
#endif
		if(ZILLIANS_OBJPOOL_PREFERABLE_POOL_SIZE > 0 && mPool.allocations.size() > ZILLIANS_OBJPOOL_PREFERABLE_POOL_SIZE)
		{
			::operator delete(p);
		}
		else
		{
			mPool.allocations.push(p);
		}
	}

	static void purge()
	{
		while(!mPool.allocations.empty())
		{
			void* obj = NULL;
			if(mPool.allocations.try_pop(obj))
			{
				::operator delete(obj);
#if ZILLIANS_OBJPOOL_ENABLE_OBJ_POOL_COUNTER
				--mPool.allocationCount;
#endif
			}
		}
	}

protected:
	/**
	 * This is used to ensure the ObjectPool<T>::purge() is called before
	 * destroying the internal object pool container.
	 */
	struct AutoPoolImpl
	{
		AutoPoolImpl() { }
		~AutoPoolImpl() { ConcurrentObjectPool<T>::purge(); }
#if ZILLIANS_OBJPOOL_ENABLE_OBJ_POOL_COUNTER
		tbb::atomic<long> allocationCount;
#endif
		tbb::concurrent_bounded_queue<void*> allocations;
	};
	static AutoPoolImpl mPool;
};

template<typename T> typename ConcurrentObjectPool<T>::AutoPoolImpl ConcurrentObjectPool<T>::mPool;

}

#endif/*ZILLIANS_OBJECTPOOL_H_*/
