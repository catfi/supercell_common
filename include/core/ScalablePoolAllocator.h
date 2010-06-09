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
 * @date Feb 15, 2009 sdk - Initial version created.
 * @date Feb 16, 2009 nothing - Implemented first-fit allocation for testing purposes.
 * @date Mar 1, 2009  nothing - New allocator in place, implements the algorithm used by McRT/Intel's TBB
 */

#ifndef ZILLIANS_SCALABLEPOOLALLOCATOR_H_
#define ZILLIANS_SCALABLEPOOLALLOCATOR_H_

#include "core/Prerequisite.h"
#include "tbb/spin_mutex.h"// for synchronization
#include "tbb/atomic.h"
#include "boost/thread.hpp"


#define ZILLIANS_SCALABLEALLOCATOR_STATISTICS ///< Enable statistics for debugging purposes.

namespace zillians {

/**
 * @brief ScalablePoolAllocator is an allocator which allocates memory from the given memory pool
 *
 * ScalablePoolAllocator can be used in other allocator, in which we may create the memory pool,
 * associate the memory pool with some context, and create buffer object instances wrapping the
 * memory pointer.
 *
 * ScalablePoolAllocator is implemented in the same way as TBB scalable allocator, which optimize
 * for small object allocation among multiple execution contexts (i.e. threads) to maximize the
 * throughput of allocation/deallocation.
 *
 * Inside ScalablePoolAllocator we have a global free memory block list, multiple bins within which
 * we have a list of memory blocks.
 */
class ScalablePoolAllocator
{
public:// Interface
	ScalablePoolAllocator(byte* pMemory, size_t size, size_t* binSizes = 0, size_t binCount = 0);
	virtual ~ScalablePoolAllocator();

	virtual byte* allocate(size_t sz);
	virtual void deallocate(byte* mem);

private:// Types and forward declaration
	typedef size_t ThreadID;
protected:
	class FreeChunk;
	class Block;
	class Bin;

private:// Method act on pool
	byte* allocateLarge(size_t sz);
	void deallocateLarge(byte* mem);
	bool isLargeChunk(byte* mem);

	bool allocateBlocks();// Add more blocks to free block stack (mallocBigBlock)

private:

	template<bool asIndex>
	inline size_t getIndexOrChunkSize(size_t sz);

	size_t getIndex(size_t sz);
	size_t getChunkSize(size_t sz);

private:
	void initEmptyBlock(Block* block, size_t sz);
	Block* getEmptyBlock(size_t chunkSize);
	void returnEmptyBlock(Block* block);
	Block* getPartialBlock(Bin* bin, size_t sz);
	void returnPartialBlock(Bin* bin, Block* block);
	void restoreBumpPtr(Block* block);

private:// public freelist operations
	tbb::spin_mutex mPublicFreeListLock;
	Block* getPublicFreeListBlock(Bin* bin);
	void privatizePublicFreeList(Block* block);
	void freePublicChunk(Block* block, FreeChunk* chunk);

private:// Per block operations
	byte* allocateFromBlock(Block* block);
	byte* allocateFromFreeList(Block* block);
	byte* allocateFromBumpPtr(Block* block);
	bool emptyEnoughToUse(Block* block);
	void removeBlock(Bin* bin, Block* block);
	void prependBlock(Bin* bin, Block* block);
	void moveBlockToBinFront(Block* block);
	void processLessUsedBlock(Block* block);

private:
	Bin* allocateTLS(size_t sz);//bootstrapMalloc(tls size)
	void deallocateTLS(Bin* p);//bootstrapFree

protected:// Internal classes
	class FreeChunk
	{
	public:
		FreeChunk*	mNext;	///< Pointer to next free chunk
	};//FreeChunk

	class Block
	{
	public:
		Block*		mNext;				///< Pointer to next block, it MUST be the first field of the Block class
		ThreadID	mOwnerID;			///< The thread ID of the thread owning this block
		Block*		mPrev;				///< Pointer to the previous block in the bin
		size_t		mChunkSize;			///< The chunk size allocated in this block
		FreeChunk*	mBumpPtr;			///< Bump pointer, points to next available chunk address (Used_End - ChunkSize)
										/// For an empty block, bump pointer should point right after the end of the block
		FreeChunk*	mFreeList;			///< Pointer to private freelist
		size_t		mAllocationCount;	///< # of objects allocated within the block
		bool		mIsFull;			///< Indicate whether the block is ready for more allocation
		FreeChunk*	mPublicFreeList;	///< FreeChunk's returned by threads other than owning thread.
		Block*		mNextPrivatizable;//?
	};//Block

	class Bin
	{
	public:
		Block*			mActiveBlock;	///< Points to the active block
		Block*			mMailBox;		///<
		tbb::spin_mutex	mMailBoxLock;	///< Lock to mail box

		inline Block* getActiveBlock() { return mActiveBlock; }
		inline void setActiveBlock(Block* block) { mActiveBlock = block; }
		inline Block* setPreviousBlockActive();
	};//Bin

	class LargeChunk
	{
	public:
		LargeChunk*	mNext;
		size_t		mSize;	///< The size available for allocation. (excluding the size header when allocated)
		LargeChunk*	mPrev;
	};

	class Stack// Helper class
	{
	public:
		inline Stack();
		inline void push(void** p);
		inline void* pop();
	private:
		void* mTop;
		tbb::spin_mutex mLock;
	};

private:// Constants
	const size_t BLOCK_SIZE;			///< Size of block, it affects the largest chunk you can store, should be a power of two.
										///  Memory is aligned to block size
	const size_t BIG_BLOCK_BLOCK_COUNT;	///< Number of blocks within a Big Block
	const size_t BIG_BLOCK_SIZE;		///< BLOCK_SIZE * BIG_BLOCK_BLOCK_COUNT, for allocating Block's
	const size_t MIN_LARGE_CHUNK_SIZE;	///< Minimum size of large chunks
	const uintptr_t INVALID;			///< Indicate this data is invalid(unusable)
	const size_t BIN_COUNT;				///< Number of bins
	const size_t TLS_SIZE;				///< Size of TLS, depends on number of Bins
	const float EMPTY_ENOUGH_THRESHOLD;	///< The amount of which the usage in a block when it can be called "empty enough". Valid values are floats between [0, 1]
	const size_t OWNER_ID_NULL;			///< Represent a null ID  (uint max)

private:// Utility methods
	inline uintptr_t alignUp(uintptr_t ptr, uintptr_t alignTo)
	{
		return ((ptr + (alignTo - 1)) & ~(alignTo - 1));
	}

	inline uintptr_t alignDown(uintptr_t ptr, uintptr_t alignTo)
	{
		return (ptr) & ~(alignTo - 1);
	}

	inline bool isInvalid(uintptr_t ptr) { return ptr == INVALID; };

	/**
	 * @brief Check if pointer is valid
	 *
	 * @param ptr Pointer to check
	 *
	 * @return True if pointer is not INVALID nor 0x0, otherwise false
	 */
	inline bool isValid(uintptr_t ptr) { return (INVALID | ptr) != INVALID; };

private:// Pool variables
	byte* mPool;	///< Pointer to memory-aligned pool
	byte* mPoolEnd;	///< Pointer to the end of pool
	tbb::spin_mutex mPoolLock;

	byte* mBlockAllocPtr;	///< Points to free space start point at bottom address
	Stack mFreeBlockStack;	///< A stack-ful of blocks ready to be allocated (freeBlockList)

	byte* mBumpPtr;	///< Points to free space start point at top address. NOTE: This bump has different meaning to Block::mBumpPtr
	/// @remarks While ScalableAllocator::mBumpPtr points to the end of the space, Block::mBumpPtr points to (UsedAddress - ChunkSize)

	LargeChunk* mLargeFreeList;	///< Freelist of large chunks, in ascending size order

private:// Bins
	typedef std::pair<size_t, size_t> SizePair;
	size_t *mBinSizes;

	Stack* mGlobalBins;	///< Contains all blocks returned from threads (globalSizedBins)


private:// Thread local storage control
	static void ThreadIDTLSCleanUpFunction(ThreadID* t);
	static void BinTLSCleanUpFunction(Bin* bins);

	ThreadID	getThreadID();
	boost::thread_specific_ptr<ThreadID>	mThreadID;///< TLS storing current thread's ID
	tbb::atomic<ThreadID> mThreadCount;

	Bin* getBin(size_t sz);
	boost::thread_specific_ptr<Bin>			mBins;///< TLS storing sized bins
	tbb::spin_mutex	mTLSAllocationLock;	///< Lock used for alloc/dealloc of TLS(bins)
	FreeChunk*		mTLSChunkList;	///< Chunks freed and used for next TLS allocation (bootStrapObjectList)
	Block*			mTLSUsedBlocks;	///< Blocks used for TLS allocation (bootStrapBlockUsed)
	Block*			mTLSAllocBlock;	///< Block used for next TLS allocation (bootStrapBlock)

private:// Logging
	static log4cxx::LoggerPtr mLogger;
public:// Statistics
	struct AllocatorStat
	{
		bool StatAvailable;
		tbb::atomic<size_t> TotalAllocations;
		tbb::atomic<size_t> TotalLargeAllocations;
		tbb::atomic<size_t> TotalSmallAllocations;
		tbb::atomic<size_t> TotalDeallocations;
		tbb::atomic<size_t> TotalLargeDeallocations;
		tbb::atomic<size_t> TotalSmallDeallocations;

		tbb::atomic<size_t> ChunksInUse;
		tbb::atomic<size_t> BlocksInUse;
		tbb::atomic<size_t> BlocksInFreeBlockStack;
		tbb::atomic<size_t> LargeChunkInFreeList;

		tbb::atomic<size_t> Underruns;	///< Number of times on subtraction when the value is already zero, which should not happen.
							///  Under correct execution, this should always be 0
		tbb::atomic<size_t> AllocationRecursion;
		tbb::atomic<size_t> GarbageCollection;

		AllocatorStat()
		{
			ChunksInUse = 0;
			BlocksInUse = 0;
			BlocksInFreeBlockStack = 0;
			LargeChunkInFreeList = 0;
			Underruns = 0;
			AllocationRecursion = 0;
			GarbageCollection = 0;
		}
	};
#ifdef ZILLIANS_SCALABLEALLOCATOR_STATISTICS
private:
	AllocatorStat mStatistics;
public:
	inline AllocatorStat getAllocatorStat() { return mStatistics; }
	void resetAllocatorStat();

#define STAT_ADD(v) (v)++;
#define STAT_ADDV(v, x) (v)+=x;
#define STAT_SUB(v) if(v == 0) { mStatistics.Underruns++; }; (v)--;
#define STAT_RESET() resetAllocatorStat()
#else// no statistics
#define STAT_ADD(v)
#define STAT_ADDV(v, x)
#define STAT_SUB(v)
#define STAT_RESET()
inline AllocatorStat getAllocatorStat() { AllocatorStat a; a.StatAvailable = false; return a; }
#endif//ZILLIANS_SCALABLEALLOCATOR_STATISTICS
};

}

#endif/*ZILLIANS_SCALABLEPOOLALLOCATOR_H_*/
