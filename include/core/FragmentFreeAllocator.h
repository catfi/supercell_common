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
 * @date Aug 30, 2009 sdk - Initial version created. Migrated from existing CUDA fragment-free allocator.
 */

#ifndef ZILLIANS_FRAGMENTFREEALLOCATOR_H_
#define ZILLIANS_FRAGMENTFREEALLOCATOR_H_

#include "core/Common.h"
#include "core/ObjectPool.h"

#include <tbb/mutex.h>
#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/bind/arg.hpp>

#define ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_OBJECT_POOL				1
#define ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION		1

namespace zillians {

/**
 * @brief (Internal Use) The pointer wrapper to facilitate defragment operator.
 *
 * Since we store the real data pointer inside FragmentPointer, upon
 * defragment, we can easily update the pointer location by changing the
 * data in the FragmentPointer.
 */
struct FragmentPointer
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_OBJECT_POOL
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
	: ConcurrentObjectPool<FragmentPointer>
#else
	: ObjectPool<FragmentPointer>
#endif
#endif
{
	FragmentPointer(byte* p) : data(p)
	{ }

	byte* data;
};

/**
 * @brief (Internal Use) The basic memory block used to allocate memory in
 * FragmentAllocator.
 *
 * FragmentBlock is used by FragmentAllocator to represent the used and
 * free memory block. Each memory allocation in FragmentAllocator creates
 * a FragmentPointer pointint to the actual memory, enclosed by a
 * corresponding FragmentBlock.
 */
class FragmentBlock
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_OBJECT_POOL
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
	: ConcurrentObjectPool<FragmentBlock>
#else
	: ObjectPool<FragmentBlock>
#endif
#endif
{
	friend class FragmentAllocator;
	friend class FragmentFreeOperator;

	FragmentBlock() :
		free(true),	offset(0), size(0),
		nextFree(NULL), prevFree(NULL),
		nextBlock(NULL), prevBlock(NULL),
		pointerReference(NULL)
	{ }

	~FragmentBlock()
	{ }

private:
	bool free;
	std::size_t offset;
	std::size_t size;

	FragmentBlock* nextFree;
	FragmentBlock* prevFree;

	FragmentBlock* nextBlock;
	FragmentBlock* prevBlock;

	FragmentPointer* pointerReference;
};

/**
 * @brief MutablePointer is a pointer wrapper to memory fragment, which can be
 * defraged by using FragmentFreeOperator.
 *
 * Given its private constructor, MutablePointer can be only created and
 * destroyed by FragmentAllocator::allocate() and deallocate()
 */
struct MutablePointer
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_OBJECT_POOL
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
	: ConcurrentObjectPool<MutablePointer>
#else
	: ObjectPool<MutablePointer>
#endif
#endif
{
//	friend class FragmentAllocator;
//	friend class FragmentFreeOperator;
//	friend class RuntimeApi;
//public:
	MutablePointer() :
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_OBJECT_POOL
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
		ConcurrentObjectPool<MutablePointer>(),
#else
		ObjectPool<MutablePointer>(),
#endif
#endif
		pointerReference(NULL), pointerOffset(0), pointerOwner(false)
	{ }

	MutablePointer(byte* pointer, std::size_t offset = 0) :
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_OBJECT_POOL
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
		ConcurrentObjectPool<MutablePointer>(),
#else
		ObjectPool<MutablePointer>(),
#endif
#endif
		pointerReference(new FragmentPointer(pointer)), pointerOffset(offset), pointerOwner(true)
	{ }

	MutablePointer(FragmentPointer* pointer, std::size_t offset = 0) :
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_OBJECT_POOL
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
		ConcurrentObjectPool<MutablePointer>(),
#else
		ObjectPool<MutablePointer>(),
#endif
#endif
		pointerReference(pointer), pointerOffset(offset), pointerOwner(true)
	{ }

	MutablePointer(MutablePointer& ref, std::size_t offset = 0) :
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_OBJECT_POOL
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
		ConcurrentObjectPool<MutablePointer>(),
#else
		ObjectPool<MutablePointer>(),
#endif
#endif
		pointerReference(ref.pointerReference), pointerOffset(ref.pointerOffset), pointerOwner(false)
	{ }

	~MutablePointer()
	{ if(pointerOwner) { SAFE_DELETE(pointerReference); } }

	byte* data() { return pointerReference->data + pointerOffset; }

//private:
	FragmentPointer* pointerReference;
	std::size_t pointerOffset;
	bool pointerOwner;
};

/**
 *
 */
class FragmentAllocator
{
	friend class FragmentFreeOperator;
public:
	FragmentAllocator(byte* pool, std::size_t size);
	~FragmentAllocator();

	bool allocate(MutablePointer **pointer, std::size_t size);
	bool deallocate(MutablePointer  *pointer);

	inline size_t total();
	inline size_t used();
	inline size_t available();

	bool isValid(MutablePointer *ptr);
	size_t infoSize(MutablePointer *ptr);

	void debug();

private:
	typedef std::map<FragmentPointer*, FragmentBlock*> PointerToBlockMap;

	size_t mConfiguredChunkSize;
	size_t mConfiguredNumChunks;

	size_t mAllocatedSize;
	byte* mDeviceBasePointer;

	FragmentBlock* mFragmentBlockHeadFree;
	FragmentBlock* mFragmentBlockHead;

	PointerToBlockMap   mPointerToBlockMap;

#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
	tbb::mutex mAllocationLock;
#endif
};

class FragmentFreeOperator
{
public:
	struct placeholders
	{
		static inline boost::arg<1> dst()
		{
			return boost::arg<1>();
		}

		static inline boost::arg<2> src()
		{
			return boost::arg<2>();
		}

		static inline boost::arg<3> size()
		{
			return boost::arg<3>();
		}
	};

	FragmentFreeOperator(boost::function< void(void*,void*,std::size_t) > copyFunctor);

	bool operator() (FragmentAllocator& allocator);

private:
	boost::function< void(void*,void*,std::size_t) > mCopyFunctor;
};

}

#endif/*FRAGMENTFREEALLOCATOR_H_*/
