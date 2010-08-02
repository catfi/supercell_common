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

#ifndef ZILLIANS_CONTEXTHUB_H_
#define ZILLIANS_CONTEXTHUB_H_

#include "core/Common.h"
#include "core/SharedPtr.h"
#include <tr1/unordered_map>
#include <tbb/atomic.h>

/**
 * By allowing arbitrary context placement for different ContextHub instance,
 * the atomic context indexer can be stored as a member variable in ContextHub
 * to avoid link dependency (so we have a pure header class.) However this makes
 * it harder to debug and make the ContextHub object larger (memory space waste.)
 */
#define ZILLIANS_SERVICEHUB_ALLOW_ARBITRARY_CONTEXT_PLACEMENT_FOR_DIFFERENT_INSTANCE  0

namespace zillians {

struct ContextOwnership
{
	enum type
	{
		keep		= 0,
		transfer	= 1
	};
};

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
template<ContextOwnership::type TransferOwnershipDefault = ContextOwnership::transfer>
class ContextHub
{
private:
	struct NullDeleter
	{
	    void operator() (void const *) const
	    { }
	};

public:
	ContextHub()
	{ }

	virtual ~ContextHub()
	{ }

public:
	/**
	 * Save an object of type T into the universal storage.
	 *
	 * @note If the TransferOwnership template parameter is set, the ownership of the given object is transferred to this ContextHub instance.
	 *
	 * @param ctx The given object of type T
	 */
	template <typename T, ContextOwnership::type TransferOwnership = TransferOwnershipDefault>
	inline void set(T* ctx)
	{
		if(TransferOwnership == ContextOwnership::transfer)
		{
			refSharedContext<T>() = shared_ptr<T>(ctx);
		}
		else
		{
			refSharedContext<T>() = shared_ptr<T>(ctx, NullDeleter());
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
		return static_pointer_cast<T>(refSharedContext<T>()).get();
	}

	/**
	 * Remove the previously stored object instance of type T.
	 *
	 * @note If the TransferOwnership template parameter is set, ContextHub will automatically destroy the object; otherwise
	 */
	template <typename T>
	inline void reset()
	{
		refSharedContext<T>().reset();
	}

private:
	/**
	 * The magic trick to store and access context object by using static
	 * initialization to identify the index of a specific type.
	 *
	 * @return The reference to the shared pointer
	 */
	template <typename T>
	inline std::vector< shared_ptr<void> >::reference refSharedContext()
	{
		static uint32 index = msContextIndexer++;
		if(UNLIKELY(index >= mSharedContextObjects.size()))
		{
			while(index >= mSharedContextObjects.size())
			{
				mSharedContextObjects.push_back(shared_ptr<void>());
			}
		}

		BOOST_ASSERT(index < mSharedContextObjects.size());
		return mSharedContextObjects[index];
	}

	std::vector< shared_ptr<void> > mSharedContextObjects;
#if ZILLIANS_SERVICEHUB_ALLOW_ARBITRARY_CONTEXT_PLACEMENT_FOR_DIFFERENT_INSTANCE
	tbb::atomic<uint32> msContextIndexer;
#else
	static tbb::atomic<uint32> msContextIndexer;
#endif
};

template<ContextOwnership::type TransferOwnershipDefault> tbb::atomic<uint32> ContextHub<TransferOwnershipDefault>::msContextIndexer;


/**
 * NamedContextHub provides name to context pointer mapping.
 *
 * NamedContextHub is an alternative version of ContexHub in which
 * an optional key (string) can be provided as the identifier of the
 * context pointer. By default, the key is the name (typeid) of the
 * given type object.
 */
template<ContextOwnership::type TransferOwnershipDefault>
class NamedContextHub
{
private:
	struct NullDeleter
	{
	    void operator() (void const *) const
	    { }
	};

public:
	NamedContextHub()
	{ }

	virtual ~NamedContextHub()
	{ }

public:
	/**
	 * Save an object of type T into the universal storage.
	 *
	 * @note If the TransferOwnership template parameter is set, the ownership of the given object is transferred to this ContextHub instance.
	 *
	 * @param ctx The given object of type T
	 */
	template <typename T, ContextOwnership::type TransferOwnership = TransferOwnershipDefault>
	inline void set(T* ctx, const std::string& name = typeid(T).name())
	{
		if(TransferOwnership == ContextOwnership::transfer)
		{
			refSharedContext<T>(name) = shared_ptr<T>(ctx);
		}
		else
		{
			refSharedContext<T>(name) = shared_ptr<T>(ctx, NullDeleter());
		}
	}

	/**
	 * Retrieve the object according to the given type T.
	 *
	 * @return The pointer to the stored object. Return null pointer if it's not set previously.
	 */
	template <typename T>
	inline T* get(const std::string& name = typeid(T).name())
	{
		return static_pointer_cast<T>(refSharedContext<T>(name)).get();
	}

	/**
	 * Remove the previously stored object instance of type T.
	 *
	 * @note If the TransferOwnership template parameter is set, ContextHub will automatically destroy the object; otherwise
	 */
	template <typename T>
	inline void reset(const std::string& name = typeid(T).name())
	{
		refSharedContext<T>(name).reset();
	}

	inline std::size_t size()
	{
		return mSharedContextObjects.size();
	}

private:
	/**
	 * The magic trick to store and access context object by using static
	 * initialization to identify the index of a specific type.
	 *
	 * @return The reference to the shared pointer
	 */
	template <typename T>
	inline shared_ptr<void>& refSharedContext(const std::string& name)
	{
		std::map< std::string, shared_ptr<void> >::iterator it = mSharedContextObjects.find(name);
		if(UNLIKELY(it == mSharedContextObjects.end()))
		{
			//mSharedContextObjects[name] = shared_ptr<void>();
			mSharedContextObjects.insert(std::make_pair(name, shared_ptr<void>()));
			return mSharedContextObjects[name];
		}
		else
		{
			return it->second;
		}
	}

	std::map< std::string, shared_ptr<void> > mSharedContextObjects;
};

/**
 * HashedContextHub provides index to context pointer mapping.
 *
 * IndexedContextHub is an alternative version of ContexHub in which
 * an key can be provided as the identifier of the context pointer.
 * By default, the key is the name (typeid) of the given type object.
 */
template<ContextOwnership::type TransferOwnershipDefault = ContextOwnership::transfer, typename KeyType = uint32>
class HashedContextHub
{
private:
	struct NullDeleter
	{
	    void operator() (void const *) const
	    { }
	};

public:
	HashedContextHub()
	{ }

	virtual ~HashedContextHub()
	{ }

public:
	/**
	 * Save an object of type T into the universal storage.
	 *
	 * @note If the TransferOwnership template parameter is set, the ownership of the given object is transferred to this ContextHub instance.
	 *
	 * @param ctx The given object of type T
	 */
	template <typename T, ContextOwnership::type TransferOwnership = TransferOwnershipDefault>
	inline void set(T* ctx, const KeyType& key)
	{
		if(TransferOwnership == ContextOwnership::transfer)
		{
			refSharedContext<T>(key) = shared_ptr<T>(ctx);
		}
		else
		{
			refSharedContext<T>(key) = shared_ptr<T>(ctx, NullDeleter());
		}
	}

	/**
	 * Retrieve the object according to the given type T.
	 *
	 * @return The pointer to the stored object. Return null pointer if it's not set previously.
	 */
	template <typename T>
	inline T* get(const KeyType& key)
	{
		return static_pointer_cast<T>(refSharedContext<T>(key)).get();
	}

	/**
	 * Remove the previously stored object instance of type T.
	 *
	 * @note If the TransferOwnership template parameter is set, ContextHub will automatically destroy the object; otherwise
	 */
	template <typename T>
	inline void reset(const KeyType& key)
	{
		refSharedContext<T>(key).reset();
	}

	inline std::size_t size()
	{
		return mSharedContextObjects.size();
	}

	template<typename F>
	void foreach(F functor)
	{
		std::for_each(mSharedContextObjects.begin(), mSharedContextObjects.end(), functor);
	}

private:
	/**
	 * The magic trick to store and access context object by using static
	 * initialization to identify the index of a specific type.
	 *
	 * @return The reference to the shared pointer
	 */
	template <typename T>
	inline shared_ptr<void>& refSharedContext(const KeyType& name)
	{
		typename std::tr1::unordered_map<KeyType, shared_ptr<void> >::iterator it = mSharedContextObjects.find(name);
		if(UNLIKELY(it == mSharedContextObjects.end()))
		{
			mSharedContextObjects[name] = shared_ptr<void>();
			return mSharedContextObjects[name];
		}
		else
		{
			return it->second;
		}
	}

	std::tr1::unordered_map<KeyType, shared_ptr<void> > mSharedContextObjects;
};

}
#endif/*ZILLIANS_CONTEXTHUB_H_*/
