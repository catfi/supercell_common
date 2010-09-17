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
 * @date Feb 16, 2009 nothing - Added a naive memory manager, using first-fit algorithm.
 * @date Mar 1, 2009 nothing - New ScalablePoolAllocator in place.
 */

#include "core/ScalablePoolAllocator.h"
#include "tbb/tbb_thread.h"

namespace zillians {

log4cxx::LoggerPtr ScalablePoolAllocator::mLogger(log4cxx::Logger::getLogger("zillians.common.core.ScalablePoolAllocator"));

ScalablePoolAllocator::ScalablePoolAllocator(byte* pMemory, size_t size, size_t* binSizes, size_t binCount)
: BLOCK_SIZE(16384)// Default to 16K blocks
, BIG_BLOCK_BLOCK_COUNT(16)// Allocate 16 new blocks at a time whenever there's not enough blocks to go around
, BIG_BLOCK_SIZE(BLOCK_SIZE * BIG_BLOCK_BLOCK_COUNT)
, MIN_LARGE_CHUNK_SIZE(binSizes?binSizes[binCount-1]+1:8193)
, INVALID(0x1)
, BIN_COUNT(binCount?binCount:32)// Default to 32 bins between 4 to 8K bytes
, TLS_SIZE(sizeof(Bin) * (BIN_COUNT + 1))// Bin size times number of bins, 1 for additional space to store "this" pointer
// TODO: above line needs more explaination.
, EMPTY_ENOUGH_THRESHOLD((BLOCK_SIZE - sizeof(Block)) * 0.75f)// 75% full is empty enough
, OWNER_ID_NULL(static_cast<size_t>(-1))
, mThreadID(ThreadIDTLSCleanUpFunction)
, mBins(BinTLSCleanUpFunction)
{
	// Only use memory aligned part
	memset(pMemory, 0, size);
	mPool = reinterpret_cast<byte*>( alignUp(reinterpret_cast<uintptr_t>(pMemory), BLOCK_SIZE) );
	mPoolEnd = pMemory + size;

	// Create TLS variables
	// 20090302 nothing - They are now created on heap
	//mThreadID = new boost::thread_specific_ptr<ThreadID>(ThreadIDTLSCleanUpFunction);
	//mBins = new boost::thread_specific_ptr<Bin>(BinTLSCleanUpFunction);

	mTLSChunkList = NULL;
	mTLSUsedBlocks = NULL;
	mTLSAllocBlock = NULL;
	mThreadCount = 0;

	// Initialize pool
	mBlockAllocPtr = mPool;

	// Large Allocation
	mBumpPtr = mPoolEnd;
	mLargeFreeList = NULL;

	// Initialize bin settings
	// 20090302 nothing - Allocate mBinSizes in mPool, and align to 128 byte boundary for cache coherence.
	mBumpPtr = reinterpret_cast<byte*>(alignDown(reinterpret_cast<uintptr_t>(mBumpPtr - (sizeof(size_t) * BIN_COUNT)), 128));
	mBinSizes = reinterpret_cast<size_t*>(mBumpPtr);//new size_t[BIN_COUNT];
	if(binSizes)
	{
		for(size_t i = 0; i < BIN_COUNT; ++i)
		{
			mBinSizes[i] = binSizes[i];
		}
	}
	else// Default bin sizes
	{
		mBinSizes[0] = 8;		mBinSizes[1] = 16;		mBinSizes[2] = 32;		mBinSizes[3] = 64;
		mBinSizes[4] = 80;		mBinSizes[5] = 96;		mBinSizes[6] = 112;		mBinSizes[7] = 128;
		mBinSizes[8] = 160;		mBinSizes[9] = 192;		mBinSizes[10] = 224;	mBinSizes[11] = 256;
		mBinSizes[12] = 320;	mBinSizes[13] = 384;	mBinSizes[14] = 448;	mBinSizes[15] = 512;
		mBinSizes[16] = 640;	mBinSizes[17] = 768;	mBinSizes[18] = 896;	mBinSizes[19] = 1024;
		mBinSizes[20] = 1280;	mBinSizes[21] = 1536;	mBinSizes[22] = 1792;	mBinSizes[23] = 2048;
		mBinSizes[24] = 2560;	mBinSizes[25] = 3072;	mBinSizes[26] = 3584;	mBinSizes[27] = 4096;
		mBinSizes[28] = 5120;	mBinSizes[29] = 6144;	mBinSizes[30] = 7168;	mBinSizes[31] = 8192;
	}
	mGlobalBins = new Stack[BIN_COUNT];

	STAT_RESET();

}//c'tor

ScalablePoolAllocator::~ScalablePoolAllocator()
{
	//
	SAFE_DELETE_ARRAY(mGlobalBins);

	//delete[] mBinSizes;// 20090302 nothing - mBinSizes now allocated on mPool

	// Destroy TLS variables// 20090302 nothing - allocated on stack
	//delete mBins;
	//delete mThreadID;
}

byte* ScalablePoolAllocator::allocate(size_t sz)//done
{
	STAT_ADD(mStatistics.TotalAllocations);

	if( sz >= MIN_LARGE_CHUNK_SIZE )
	{
		return allocateLarge(sz);
	}

	STAT_ADD(mStatistics.TotalSmallAllocations);
	STAT_ADD(mStatistics.ChunksInUse);

	Bin* bin = getBin(sz);
	byte* ret = NULL;
	Block* block = bin->getActiveBlock();

	if(block)
	{
		do
		{
			if( (ret = allocateFromBlock(block)) )
			{
				return ret;
			}
		} while( (block = bin->setPreviousBlockActive()) );
	}

	block = getPublicFreeListBlock(bin);
	if(block)
	{
		if(emptyEnoughToUse(block))
		{
			removeBlock(bin, block);
			prependBlock(bin, block);
		}
		if( (ret = allocateFromFreeList(block)) )
		{
			return ret;
		}
		STAT_ADD(mStatistics.AllocationRecursion);
		return allocate(sz);// Code should not reach this line
	}

	block = getPartialBlock(bin, sz);
	while(block)
	{
		prependBlock(bin, block);
		bin->setActiveBlock(block);
		if( (ret = allocateFromBlock(block)) )
		{
			return ret;
		}
		block = getPartialBlock(bin, sz);
	}

	block = getEmptyBlock(sz);
	if(block)
	{
		prependBlock(bin, block);
		bin->setActiveBlock(block);
		if( (ret = allocateFromBlock(block)) )
		{
			return ret;
		}
		STAT_ADD(mStatistics.AllocationRecursion);
		return allocate(sz);// Code should not reach this line
	}

	STAT_SUB(mStatistics.ChunksInUse);
	return NULL;// Out of memory
}

void ScalablePoolAllocator::deallocate(byte* mem)//done
{
	if(!mem) { return; }

	if((mem < mPool)||(mem > mPoolEnd))
	{
		// NOTE: This should never ever happen, it's user's fault if this happens
		LOG4CXX_ERROR(mLogger, "mem is not inside the pool. User's fault!");
		assert(0);
		return;
	}
	STAT_ADD(mStatistics.TotalDeallocations);

	if(isLargeChunk(mem))
	{
		deallocateLarge(mem);
		return;
	}
	STAT_ADD(mStatistics.TotalSmallDeallocations);
	STAT_SUB(mStatistics.ChunksInUse);

	FreeChunk* chunk = reinterpret_cast<FreeChunk*>(mem);
	ThreadID tid = getThreadID();
	Block* block = reinterpret_cast<Block*>( alignDown(reinterpret_cast<uintptr_t>(mem), BLOCK_SIZE) );

	if(tid == block->mOwnerID)
	{
		chunk->mNext = block->mFreeList;
		block->mFreeList = chunk;
		block->mAllocationCount--;

		if(block->mIsFull)
		{
			if(emptyEnoughToUse(block))
			{
				moveBlockToBinFront(block);
			}
		}
		else
		{
			if( (block->mAllocationCount == 0) && (block->mPublicFreeList == NULL) )
			{
				processLessUsedBlock(block);
			}
		}
	}
	else
	{
		freePublicChunk(block, chunk);
	}
}


byte* ScalablePoolAllocator::allocateLarge(size_t sz)
{
	tbb::spin_mutex::scoped_lock lock(mPoolLock);
	STAT_ADD(mStatistics.TotalLargeAllocations);

	LargeChunk* chunk = mLargeFreeList;
	size_t* psz = NULL;

	// 1. Search for a FreeChunk that's large enough for allocation
	while(chunk)
	{
		if(chunk->mSize == sz)// Exact fit
		{
			// Detach chunk from list
			if(chunk->mPrev)
			{
				chunk->mPrev->mNext = chunk->mNext;
			}
			else
			{
				mLargeFreeList = chunk->mNext;
			}
			if(chunk->mNext)
			{
				chunk->mNext->mPrev = chunk->mPrev;
			}

			psz = reinterpret_cast<size_t*>(chunk);
			*psz = sz;
			return reinterpret_cast<byte*>(psz) + sizeof(size_t);
		}
		else if(chunk->mSize > sz)// Fit
		{
			if(chunk->mSize - sz > sizeof(LargeChunk))// add a new LargeChunk
			{
				LargeChunk* newchunk = reinterpret_cast<LargeChunk*>( reinterpret_cast<byte*>(chunk) + sz + sizeof(size_t) );
				newchunk->mNext = chunk->mNext;
				newchunk->mPrev = chunk->mPrev;
				newchunk->mSize = chunk->mSize - sz -sizeof(size_t);
				if(newchunk->mPrev)
				{
					newchunk->mPrev->mNext = newchunk;
				}
				else
				{
					mLargeFreeList = newchunk;
				}
				if(newchunk->mNext)
				{
					newchunk->mNext->mPrev = newchunk;
				}
				psz = reinterpret_cast<size_t*>(chunk);
				*psz = sz;
				return reinterpret_cast<byte*>(psz) + sizeof(size_t);
			}
			else// remain space cannot fit a new LargeChunk, give entire LargeChunk to user.
			{
				// Detach chunk from list
				if(chunk->mPrev)
				{
					chunk->mPrev->mNext = chunk->mNext;
				}
				else
				{
					mLargeFreeList = chunk->mNext;
				}
				if(chunk->mNext)
				{
					chunk->mNext->mPrev = chunk->mPrev;
				}

				psz = reinterpret_cast<size_t*>(chunk);
				*psz = sz;
				return reinterpret_cast<byte*>(psz) + sizeof(size_t);
			}
		}
		chunk = chunk->mNext;
	}//while(chunk)

	// 2. Allocate from mBumpPtr
	mBumpPtr = mBumpPtr - sz - sizeof(size_t);
	if(mBumpPtr < mBlockAllocPtr)
	{
		// Out of memory!
		mBumpPtr = mBumpPtr + sz + sizeof(size_t);
		STAT_ADD(mStatistics.GarbageCollection);// TODO: Need some sort of garbage collection
		return NULL;
	}
	psz = reinterpret_cast<size_t*>(mBumpPtr);
	*psz = sz;

	return reinterpret_cast<byte*>(psz) + sizeof(size_t);

}//allocateLarge(size_t sz)


void ScalablePoolAllocator::deallocateLarge(byte* mem)
{
	tbb::spin_mutex::scoped_lock lock(mPoolLock);
	STAT_ADD(mStatistics.TotalLargeDeallocations);

	size_t* psz = (reinterpret_cast<size_t*>(mem - sizeof(size_t)));
	if(mem == mBumpPtr + sizeof(size_t))// At front, move the bump pointer back
	{
		mBumpPtr = mBumpPtr + (*psz) + sizeof(size_t);
		return;
	}
	else
	{
		LargeChunk* lc = reinterpret_cast<LargeChunk*>(psz);
		lc->mSize = *psz;
		LargeChunk* list = mLargeFreeList;

		if(list == NULL)// The only item in freelist
		{
			lc->mPrev = lc->mNext = NULL;
			mLargeFreeList = lc;
			STAT_ADD(mStatistics.LargeChunkInFreeList);
			return;
		}

		// Find position in the list
		while((list->mNext != NULL) && (list < lc))
		{
			list = list->mNext;
		}
		if(list->mNext == NULL)// last item
		{
			if(list < lc)// Append to the end of freelist
			{
				lc->mNext = NULL;
				lc->mPrev = list;
				list->mNext = lc;
			}
			else// Insert before last item
			{
				lc->mNext = list;
				lc->mPrev = list->mPrev;
				list->mPrev = lc;
				if(lc->mPrev)
				{
					lc->mPrev->mNext = lc;
				}
				else//lc is head
				{
					mLargeFreeList = lc;
				}
			}
		}
		else// list > lc
		{
			lc->mNext = list;
			lc->mPrev = list->mPrev;
			list->mPrev = lc;
			if(lc->mPrev)
			{
				lc->mPrev->mNext = lc;
			}
			else
			{
				mLargeFreeList = lc;
			}
		}
		STAT_ADD(mStatistics.LargeChunkInFreeList);

		// Merge next/prev blocks if they are neighbours
		if((lc->mPrev) && (reinterpret_cast<byte*>(lc->mPrev) + lc->mPrev->mSize + sizeof(size_t) == reinterpret_cast<byte*>(lc)))
		{
			lc->mPrev->mSize += (lc->mSize + sizeof(size_t));
			lc->mPrev->mNext = lc->mNext;
			if(lc->mNext)
			{
				lc->mNext->mPrev = lc->mPrev;
			}
			lc = lc->mPrev;
			STAT_SUB(mStatistics.LargeChunkInFreeList);
		}
		if((lc->mNext) && (reinterpret_cast<byte*>(lc) + lc->mSize + sizeof(size_t) == reinterpret_cast<byte*>(lc->mNext)))
		{
			lc->mSize += (lc->mNext->mSize + sizeof(size_t));
			lc->mNext = lc->mNext->mNext;
			if(lc->mNext)
			{
				lc->mNext->mPrev = lc;
			}
			STAT_SUB(mStatistics.LargeChunkInFreeList);
		}
	}
}//deallocateLarge(byte* mem)


bool ScalablePoolAllocator::isLargeChunk(byte* mem)// done
{
	return (mem > mBlockAllocPtr);
}


ScalablePoolAllocator::ThreadID ScalablePoolAllocator::getThreadID()
{
	ThreadID ret = reinterpret_cast<ThreadID>(mThreadID.get());
	if( ret == 0 )
	{
		ret = ++mThreadCount;// Atomic operation
		mThreadID.reset(reinterpret_cast<ThreadID*>(ret));
	}
	return ret;
}


ScalablePoolAllocator::Bin* ScalablePoolAllocator::getBin(size_t sz)//done
{
	Bin* bins = mBins.get();

	if(bins == NULL)// First time used, create bins
	{
		bins = allocateTLS(TLS_SIZE);
		uintptr_t* sa = reinterpret_cast<uintptr_t*>(bins);
		*sa = reinterpret_cast<uintptr_t>(this);
		bins = bins + 1;
		mBins.reset(bins);
	}
	return bins + getIndex(sz);
}


ScalablePoolAllocator::Bin* ScalablePoolAllocator::allocateTLS(size_t sz)// done
{
	Bin* ret;

	{
		tbb::spin_mutex::scoped_lock lock(mTLSAllocationLock);

		if(mTLSChunkList)// Free chunk ready to use
		{
			ret = reinterpret_cast<Bin*>(mTLSChunkList);
			mTLSChunkList = mTLSChunkList->mNext;
		}
		else
		{
			if( !mTLSAllocBlock )
			{
				mTLSAllocBlock = getEmptyBlock(sz);
			}
			ret = reinterpret_cast<Bin*>(mTLSAllocBlock->mBumpPtr);
			mTLSAllocBlock->mBumpPtr = reinterpret_cast<FreeChunk*>( reinterpret_cast<uintptr_t>(mTLSAllocBlock->mBumpPtr) - mTLSAllocBlock->mChunkSize );
			if( reinterpret_cast<uintptr_t>(mTLSAllocBlock->mBumpPtr) < reinterpret_cast<uintptr_t>(mTLSAllocBlock) + sizeof(Block))
			{
				mTLSAllocBlock->mBumpPtr = NULL;
				mTLSAllocBlock->mNext = mTLSUsedBlocks;
				mTLSUsedBlocks = mTLSAllocBlock;
				mTLSAllocBlock = NULL;
			}
		}
	}//unlock

	memset(ret, 0, sz);
	return ret;
}


void ScalablePoolAllocator::deallocateTLS(Bin* p)//done (bootStrapFree)
{
	{// lock
		tbb::spin_mutex::scoped_lock lock(mTLSAllocationLock);
		reinterpret_cast<FreeChunk*>(p)->mNext = mTLSChunkList;
		mTLSChunkList = reinterpret_cast<FreeChunk*>(p);
	}// unlock
}


bool ScalablePoolAllocator::allocateBlocks()//done (mallocBigBlock)
{
	tbb::spin_mutex::scoped_lock lock(mPoolLock);// NOTE: can be replace with atomic addition to mBlockAllocPtr
	Block* blk = reinterpret_cast<Block*>(mBlockAllocPtr);
	mBlockAllocPtr += BIG_BLOCK_SIZE;

	if(mBlockAllocPtr > mBumpPtr) { return false; }// Out of memory!

	blk->mBumpPtr = reinterpret_cast<FreeChunk*>( reinterpret_cast<uintptr_t>(blk) + BIG_BLOCK_SIZE );
	mFreeBlockStack.push(reinterpret_cast<void**>(blk));

	STAT_ADDV(mStatistics.BlocksInFreeBlockStack, BIG_BLOCK_BLOCK_COUNT);
	return true;
}


template<bool asIndex>
size_t ScalablePoolAllocator::getIndexOrChunkSize(size_t sz)
{
	if( BIN_COUNT < 17 || sz <= mBinSizes[15] )	{// [0-15]
		if( BIN_COUNT < 9 || sz <= mBinSizes[7] )		{// [0-7]
			if( BIN_COUNT < 5 || sz <= mBinSizes[3] )		{//[0-3]
				if( BIN_COUNT < 3 || sz <= mBinSizes[1] )		{
					if( sz <= mBinSizes[0] )	{	return asIndex?0:mBinSizes[0];	}
					else						{	return asIndex?1:mBinSizes[1];	}
				}else{
					if( sz <= mBinSizes[2] )	{	return asIndex?2:mBinSizes[2];	}
					else						{	return asIndex?3:mBinSizes[3];	}
				}
			}else{//[4-7]
				if( BIN_COUNT < 7 || sz <= mBinSizes[5] )		{
					if( sz <= mBinSizes[4] )	{	return asIndex?4:mBinSizes[4];	}
					else						{	return asIndex?5:mBinSizes[5];	}
				}else{
					if( sz <= mBinSizes[6] )	{	return asIndex?6:mBinSizes[6];	}
					else						{	return asIndex?7:mBinSizes[7];	}
				}
			}
		}else{// [8 - 15]
			if( BIN_COUNT < 13 || sz <= mBinSizes[11] )		{//[8-11]
				if( BIN_COUNT < 11 || sz <= mBinSizes[9] )		{
					if( sz <= mBinSizes[8] )	{	return asIndex?8:mBinSizes[8];	}
					else						{	return asIndex?9:mBinSizes[9];	}
				}else{
					if( sz <= mBinSizes[10])	{	return asIndex?10:mBinSizes[10];	}
					else						{	return asIndex?11:mBinSizes[11];	}
				}
			}else{//[12-15]
				if( BIN_COUNT < 15 || sz <= mBinSizes[13] )	{
					if( sz <= mBinSizes[12] )	{	return asIndex?12:mBinSizes[12];	}
					else						{	return asIndex?13:mBinSizes[13];	}
				}else{
					if( sz <= mBinSizes[14] )	{	return asIndex?14:mBinSizes[14];	}
					else						{	return asIndex?15:mBinSizes[15];	}
		}	}	}
	}else{// [16-31]
		if( BIN_COUNT < 25 || sz <= mBinSizes[23] )		{// [16-23]
			if( BIN_COUNT < 21 || sz <= mBinSizes[19] )		{//[16-19]
				if( BIN_COUNT < 19 || sz <= mBinSizes[17] )		{
					if( sz <= mBinSizes[16] )	{	return asIndex?16:mBinSizes[16];	}
					else						{	return asIndex?17:mBinSizes[17];	}
				}else{
					if( sz <= mBinSizes[18] )	{	return asIndex?18:mBinSizes[18];	}
					else						{	return asIndex?19:mBinSizes[19];	}
				}
			}else{//[20-23]
				if( BIN_COUNT < 23 || sz <= mBinSizes[21] )		{
					if( sz <= mBinSizes[20] )	{	return asIndex?20:mBinSizes[20];	}
					else						{	return asIndex?21:mBinSizes[21];	}
				}else{
					if( sz <= mBinSizes[22] )	{	return asIndex?22:mBinSizes[22];	}
					else						{	return asIndex?23:mBinSizes[23];	}
				}
			}
		}else{// [24 - 31]
			if( BIN_COUNT < 29 || sz <= mBinSizes[27] )		{//[24-27]
				if( BIN_COUNT < 27 || sz <= mBinSizes[25] )	{
					if( sz <= mBinSizes[24] )	{	return asIndex?24:mBinSizes[24];	}
					else						{	return asIndex?25:mBinSizes[25];	}
				}else{
					if( sz <= mBinSizes[26] )	{	return asIndex?26:mBinSizes[26];	}
					else						{	return asIndex?27:mBinSizes[27];	}
				}
			}else{//[28-31]
				if( BIN_COUNT < 31 || sz <= mBinSizes[29] )	{
					if( sz <= mBinSizes[28] )	{	return asIndex?28:mBinSizes[28];	}
					else						{	return asIndex?29:mBinSizes[29];	}
				}else{
					if( sz <= mBinSizes[30] )	{	return asIndex?30:mBinSizes[30];	}
					else						{	return asIndex?31:mBinSizes[31];	}
		}	}	}
	}
}//getIndexOrChunkSize()

size_t ScalablePoolAllocator::getIndex(size_t sz)//done
{
	return getIndexOrChunkSize<true>(sz);
}
size_t ScalablePoolAllocator::getChunkSize(size_t sz)//done
{
	return getIndexOrChunkSize<false>(sz);
}


void ScalablePoolAllocator::initEmptyBlock(Block* block, size_t sz)//done
{
	size_t idx = getIndex(sz);
	size_t chunkSize = getChunkSize(sz);
	Bin* tls = mBins.get();

	block->mNext = NULL;
	block->mPrev = NULL;
	block->mChunkSize = chunkSize;
	block->mOwnerID = getThreadID();
	block->mBumpPtr = reinterpret_cast<FreeChunk*>( reinterpret_cast<uintptr_t>(block) + BLOCK_SIZE - chunkSize );
	block->mFreeList = NULL;
	block->mPublicFreeList = NULL;
	block->mAllocationCount = 0;
	block->mIsFull = false;
	block->mNextPrivatizable = tls ? reinterpret_cast<Block*>(tls + idx) : NULL;
}


ScalablePoolAllocator::Block* ScalablePoolAllocator::getEmptyBlock(size_t chunkSize)//done
{
	Block* ret = NULL;
	Block* bigBlock;

	bigBlock = reinterpret_cast<Block*>(mFreeBlockStack.pop());
	while( !bigBlock )
	{
		if( !allocateBlocks() )// Failed to allocate, most likely out of memory
		{
			return NULL;
		}
		bigBlock = reinterpret_cast<Block*>(mFreeBlockStack.pop());
	}

	bigBlock->mBumpPtr = reinterpret_cast<FreeChunk*>( reinterpret_cast<uintptr_t>(bigBlock->mBumpPtr) - BLOCK_SIZE);
	ret = reinterpret_cast<Block*>(bigBlock->mBumpPtr);
	if(ret != bigBlock)
	{
		mFreeBlockStack.push( reinterpret_cast<void**>(bigBlock) );
	}
	initEmptyBlock(ret, chunkSize);

	STAT_SUB(mStatistics.BlocksInFreeBlockStack);
	STAT_ADD(mStatistics.BlocksInUse);
	return ret;
}


void ScalablePoolAllocator::returnEmptyBlock(Block* block)//done
{
	block->mPublicFreeList = NULL;
	block->mNextPrivatizable = NULL;

	block->mNext = NULL;
	block->mPrev = NULL;
	block->mChunkSize = 0;
	block->mOwnerID = OWNER_ID_NULL;

	// for an empty block, bump pointer should point right after the end of the block
	block->mBumpPtr = reinterpret_cast<FreeChunk*>( reinterpret_cast<uintptr_t>(block) + BLOCK_SIZE );
	block->mFreeList = NULL;
	block->mAllocationCount = 0;
	block->mIsFull = false;

	STAT_SUB(mStatistics.BlocksInUse);
	STAT_ADD(mStatistics.BlocksInFreeBlockStack);
	mFreeBlockStack.push( reinterpret_cast<void**>(block) );
}


ScalablePoolAllocator::Block* ScalablePoolAllocator::getPartialBlock(Bin* bin, size_t sz)//done
{
	Block* ret;
	size_t idx = getIndex(sz);
	ret = reinterpret_cast<Block*>(mGlobalBins[idx].pop());
	if(ret)
	{
		ret->mNext = NULL;
		ret->mPrev = NULL;
		ret->mOwnerID = getThreadID();
		ret->mNextPrivatizable = reinterpret_cast<Block*>(bin);
		privatizePublicFreeList(ret);
		if(ret->mAllocationCount)
		{
			emptyEnoughToUse(ret);
		}
		else
		{
			restoreBumpPtr(ret);
		}
	}
	return ret;
}


void ScalablePoolAllocator::returnPartialBlock(Bin* bin, Block* block)//done
{
	size_t idx = getIndex(block->mChunkSize);
	if( reinterpret_cast<uintptr_t>(block->mNextPrivatizable) == reinterpret_cast<uintptr_t>(bin) )
	{
		void* oldval = NULL;
		{//lock
			tbb::spin_mutex::scoped_lock lock(mPublicFreeListLock);
			if( (oldval = reinterpret_cast<void*>(block->mPublicFreeList)) == NULL)
			{
				block->mPublicFreeList = reinterpret_cast<FreeChunk*>(INVALID);
			}
		}//unlock
		if(oldval != NULL)
		{
			// need to wait for the other thread finishes freeing the object
			int waitcnt = 256;
			while( reinterpret_cast<uintptr_t>(const_cast<Block* volatile &>(block->mNextPrivatizable)) == reinterpret_cast<uintptr_t>(bin) )
			{
				if(--waitcnt == 0)
				{
					tbb::this_tbb_thread::yield();
					waitcnt = 256;
				}
			}
		}
	}

	block->mPrev = NULL;
	block->mOwnerID = 0;

	block->mNextPrivatizable = reinterpret_cast<Block*>(INVALID);
	mGlobalBins[idx].push(reinterpret_cast<void**>(block));
}


void ScalablePoolAllocator::restoreBumpPtr(Block* block)//done
{
	// This will make the block empty, and it assumes there're no allocations within the block
	block->mBumpPtr = reinterpret_cast<FreeChunk*>( reinterpret_cast<uintptr_t>(block) + BLOCK_SIZE - block->mChunkSize );
	block->mFreeList = NULL;
	block->mIsFull = false;
}



byte* ScalablePoolAllocator::allocateFromBlock(Block* block)//done
{
	byte* ret;
	if( (ret = allocateFromFreeList(block)) )
	{
		return ret;
	}
	if( (ret = allocateFromBumpPtr(block)) )
	{
		return ret;
	}
	block->mIsFull = true;
	return NULL;
}


byte* ScalablePoolAllocator::allocateFromFreeList(Block* block)//done
{
	byte* ret;
	if( !block->mFreeList )
	{
		return NULL;
	}
	ret = reinterpret_cast<byte*>(block->mFreeList);
	block->mFreeList = block->mFreeList->mNext;
	block->mAllocationCount++;
	return ret;
}


byte* ScalablePoolAllocator::allocateFromBumpPtr(Block* block)//done
{
	byte* ret = reinterpret_cast<byte*>(block->mBumpPtr);
	if(ret)
	{
		block->mBumpPtr = reinterpret_cast<FreeChunk*>( reinterpret_cast<uintptr_t>(block->mBumpPtr) - block->mChunkSize );
		if( reinterpret_cast<uintptr_t>(block->mBumpPtr) < reinterpret_cast<uintptr_t>(block) + sizeof(Block) )
		{
			block->mBumpPtr = NULL;
		}
		block->mAllocationCount++;
	}
	return ret;
}


ScalablePoolAllocator::Block* ScalablePoolAllocator::getPublicFreeListBlock(Bin* bin)//done
{
	Block* ret;
	{//lock
		tbb::spin_mutex::scoped_lock lock(bin->mMailBoxLock);
		ret = bin->mMailBox;
		if(ret)
		{
			bin->mMailBox = ret->mNextPrivatizable;// ? when is this property a block, when is it a bin?
			ret->mNextPrivatizable = reinterpret_cast<Block*>(bin);
		}
	}//unlock
	if(ret)
	{
		privatizePublicFreeList(ret);
	}
	return ret;
}


void ScalablePoolAllocator::privatizePublicFreeList(Block* block)//done
{
	FreeChunk* tmp, *publicFreeList;

	///{ public freelist locking scheme
	{
		tbb::spin_mutex::scoped_lock lock(mPublicFreeListLock);
		publicFreeList = block->mPublicFreeList;
		block->mPublicFreeList = NULL;
	}
	tmp = publicFreeList;
	///}
	if( !isInvalid( reinterpret_cast<uintptr_t>(tmp) ) )// return/getPartialBlock could set it to INVALID
	{
		block->mAllocationCount--;
		while( isValid( reinterpret_cast<uintptr_t>(tmp->mNext) ) )// the list will end with either NULL or UNUSABLE
		{
			tmp = tmp->mNext;
			block->mAllocationCount--;
		}
		// prepend to private freelist
		tmp->mNext = block->mFreeList;
		block->mFreeList = publicFreeList;
	}
}


void ScalablePoolAllocator::freePublicChunk(Block* block, FreeChunk* chunk)//done
{
	Bin* bin;
	FreeChunk* publicFreeList;
	{//lock
		tbb::spin_mutex::scoped_lock lock(mPublicFreeListLock);
		publicFreeList = chunk->mNext = block->mPublicFreeList;
		block->mPublicFreeList = chunk;
	}//unlock

	if( publicFreeList == NULL )
	{
		if( !isInvalid( reinterpret_cast<uintptr_t>(block->mNextPrivatizable) ) )
		{
			bin = reinterpret_cast<Bin*>(block->mNextPrivatizable);
			tbb::spin_mutex::scoped_lock maillock(bin->mMailBoxLock);
			block->mNextPrivatizable = bin->mMailBox;
			bin->mMailBox = block;
		}
		else
		{
			// MALLOC_ASSERT( block->owner==0, ASSERT_TEXT );
		}
	}
}


bool ScalablePoolAllocator::emptyEnoughToUse(Block* block)//done
{
	if(block->mBumpPtr)
	{
		block->mIsFull = false;
		return true;
	}
	block->mIsFull = (block->mAllocationCount * block->mChunkSize > EMPTY_ENOUGH_THRESHOLD);
	return !(block->mIsFull);
}


void ScalablePoolAllocator::removeBlock(Bin* bin, Block* block)//done
{
	//size_t sz = block->mChunkSize;
	if(block == bin->mActiveBlock)
	{
		bin->mActiveBlock = block->mPrev ? block->mPrev : block->mNext;
	}
	if(block->mPrev)
	{
		block->mPrev->mNext = block->mNext;
	}
	if(block->mNext)
	{
		block->mNext->mPrev = block->mPrev;
	}
	block->mNext = NULL;
	block->mPrev = NULL;
}


void ScalablePoolAllocator::prependBlock(Bin* bin, Block* block)//done
{
	//size_t sz = block->mChunkSize;
	Block* active;
	active = bin->mActiveBlock;
	block->mNext = active;
	if(active)
	{
		block->mPrev = active->mPrev;
		active->mPrev = block;
		if(block->mPrev)
		{
			block->mPrev->mNext = block;
		}
	}
	else
	{
		bin->mActiveBlock = block;
	}
}


void ScalablePoolAllocator::moveBlockToBinFront(Block* block)// done
{
	Bin* bin = getBin(block->mChunkSize);
	removeBlock(bin, block);
	prependBlock(bin, block);
}


void ScalablePoolAllocator::processLessUsedBlock(Block* block)//done
{
	Bin* bin = getBin(block->mChunkSize);
	if( (block != bin->getActiveBlock()) && (block != bin->getActiveBlock()->mPrev) )
	{
		removeBlock(bin, block);
		returnEmptyBlock(block);
	}
	else
	{
		restoreBumpPtr(block);
	}
}

/// Bin
///{
ScalablePoolAllocator::Block* ScalablePoolAllocator::Bin::setPreviousBlockActive()//done
{
	Block* tmp = mActiveBlock->mPrev;
	if(tmp)
	{
		mActiveBlock = tmp;
	}
	return tmp;
}

///}

/// Stack
///{
ScalablePoolAllocator::Stack::Stack() : mTop(NULL)
{}
void ScalablePoolAllocator::Stack::push(void** p)
{
	tbb::spin_mutex::scoped_lock lock(mLock);
	*p = mTop;
	mTop = reinterpret_cast<void*>(p);
}
void* ScalablePoolAllocator::Stack::pop()
{
	void** ret = NULL;
	{
		tbb::spin_mutex::scoped_lock lock(mLock);
		if( !mTop ) { return ret; }
		ret = reinterpret_cast<void**>(mTop);
		mTop = *ret;
	}
	*ret = NULL;
	return ret;
}
///}

/// TLS Clean up functions
///{
void ScalablePoolAllocator::ThreadIDTLSCleanUpFunction(ThreadID* t)
{
	t = NULL;
	/*do nothing*/
}
void ScalablePoolAllocator::BinTLSCleanUpFunction(Bin* bins)
{
	// This is needed due to unavailability of "this" pointer inside a static member function.
	// The pointer is obtained from TLS that had stored "this" pointer during initial getBin() method.
	ScalablePoolAllocator* _this = reinterpret_cast<ScalablePoolAllocator*>( *((uintptr_t*)((byte*)bins - sizeof(Bin))) );
	Block* threadlessBlock;
	Block* threadBlock;

	if(bins)
	{
		for(size_t idx = 0; idx < _this->BIN_COUNT; idx++)
		{
			if(bins[idx].mActiveBlock == NULL) { continue; }

			threadlessBlock = bins[idx].mActiveBlock->mPrev;
			while(threadlessBlock)
			{
				threadBlock = threadlessBlock->mPrev;
				if((threadlessBlock->mAllocationCount == 0) && (threadlessBlock->mPublicFreeList == NULL))
				{
					_this->returnEmptyBlock(threadlessBlock);
				}
				else
				{
					_this->returnPartialBlock(bins + idx, threadlessBlock);
				}
				threadlessBlock = threadBlock;
			}
			threadlessBlock = bins[idx].mActiveBlock;
			while(threadlessBlock)
			{
				threadBlock = threadlessBlock->mNext;
				if((threadlessBlock->mAllocationCount == 0) && (threadlessBlock->mPublicFreeList == NULL))
				{
					_this->returnEmptyBlock(threadlessBlock);
				}
				else
				{
					_this->returnPartialBlock(bins + idx, threadlessBlock);
				}
				threadlessBlock = threadBlock;
			}
			bins[idx].mActiveBlock = NULL;
		}
		_this->deallocateTLS(bins - 1);// Including the "this" pointer upon deallocation
		bins = NULL;
	}

	bins = NULL;
}
///}


/// Statistics
///{
#ifdef ZILLIANS_SCALABLEALLOCATOR_STATISTICS
void ScalablePoolAllocator::resetAllocatorStat()
{
	mStatistics.StatAvailable = true;
	mStatistics.TotalAllocations = 0;
	mStatistics.TotalDeallocations = 0;
	mStatistics.TotalSmallAllocations = 0;
	mStatistics.TotalSmallDeallocations = 0;
	mStatistics.TotalLargeAllocations = 0;
	mStatistics.TotalLargeDeallocations = 0;
	mStatistics.ChunksInUse = 0;
	mStatistics.Underruns = 0;
	mStatistics.AllocationRecursion = 0;
	mStatistics.GarbageCollection = 0;
}
#endif
///}

}
