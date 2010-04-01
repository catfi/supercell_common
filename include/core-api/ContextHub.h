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
/**
 * @date Aug 27, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_SERVICEHUB_H_
#define ZILLIANS_SERVICEHUB_H_

#include "core-api/Common.h"
#include "core-api/SharedPtr.h"

#include <tbb/atomic.h>

/**
 * By allowing arbitrary context placement for different ContextHub instance,
 * the atomic context indexer can be stored as a member variable in ContextHub
 * to avoid link dependency (so we have a pure header class.) However this makes
 * it harder to debug and make the ContextHub object larger (memory space waste.)
 */
#define ZILLIANS_SERVICEHUB_ALLOW_ARBITRARY_CONTEXT_PLACEMENT_FOR_DIFFERENT_INSTANCE  0

namespace zillians {

/**
 * ContextHub is a universal storage for arbitrary class with constant access time.
 *
 * ContextHub utilized static template method trick to give different template type
 * a different class index, which will be used to locate the stored instance. The
 * access time is basically a small constant, since we don't use map or hash but a
 * dead simple dynamic array (std::vector).
 *
 * @note There's only one slot for each different type of object. Suppose you have
 * three different types named A, B, and C. You can only save one instance of A into
 * one instance of ContextHub. Same for B and C.
 */
template<bool TransferOwnership>
class ContextHub
{
public:
	ContextHub()
	{ }

public:
	/**
	 * Save an object of type T into the universal storage.
	 *
	 * @note If the TransferOwnership template parameter is set, the ownership of the given object is transferred to this ContextHub instance.
	 *
	 * @param ctx The given object of type T
	 */
	template <typename T>
	inline void set(T* ctx)
	{
		if(TransferOwnership)
		{
			refSharedContext<T>() = boost::shared_ptr<T>(ctx);
		}
		else
		{
			refContext<T>() = (void*)ctx;
		}
	}

	/**
	 * Retrieve the object according to the given type T.
	 *
	 * @return The pointer to the stored object. Return null pointer if it's not set previously.
	 */
	template <typename T>
	inline T* get()
	{
		if(TransferOwnership)
		{
			return boost::static_pointer_cast<T>(refSharedContext<T>()).get();
		}
		else
		{
			return (T*)(refContext<T>());
		}
	}

	/**
	 * Remove the previously stored object instance of type T.
	 *
	 * @note If the TransferOwnership template parameter is set, ContextHub will automatically destroy the object; otherwise
	 */
	template <typename T>
	inline void reset()
	{
		if(TransferOwnership)
		{
			refSharedContext<T>().reset();
		}
		else
		{
			refContext<T>() = NULL;
		}
	}

private:
	/**
	 * The magic trick to store and access context object by using static
	 * initialization to identify the index of a specific type.
	 *
	 * @return The reference to the shared pointer
	 */
	template <typename T>
	inline std::vector< boost::shared_ptr<void> >::reference refSharedContext()
	{
		static uint32 index = msContextIndexer++;
		if(UNLIKELY(index >= mSharedContextObjects.size()))
		{
			while(index >= mSharedContextObjects.size())
			{
				mSharedContextObjects.push_back(boost::shared_ptr<void>());
			}
		}

		BOOST_ASSERT(index < mSharedContextObjects.size());
		return mSharedContextObjects[index];
	}

	/**
	 * The magic trick to store and access context object by using static
	 * initialization to identify the index of a specific type.
	 *
	 * @return The reference to the shared pointer
	 */
	template <typename T>
	inline std::vector< void* >::reference refContext()
	{
		static uint32 index = msContextIndexer++;
		if(UNLIKELY(index >= mContextObjects.size()))
		{
			while(index >= mContextObjects.size())
			{
				mContextObjects.push_back(NULL);
			}
		}

		BOOST_ASSERT(index < mContextObjects.size());
		return mContextObjects[index];
	}

	std::vector< boost::shared_ptr<void> > mSharedContextObjects;
	std::vector< void* > mContextObjects;
#if ZILLIANS_SERVICEHUB_ALLOW_ARBITRARY_CONTEXT_PLACEMENT_FOR_DIFFERENT_INSTANCE
	tbb::atomic<uint32> msContextIndexer;
#else
	static tbb::atomic<uint32> msContextIndexer;
#endif
};

template<bool TransferOwnership> tbb::atomic<uint32> ContextHub<TransferOwnership>::msContextIndexer;

}
#endif/*ZILLIANS_SERVICEHUB_H_*/
