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
 * @date Aug 31, 2009 sdk - Initial version created.
 */

#include "core-api/FragmentFreeAllocator.h"

namespace zillians {

FragmentAllocator::FragmentAllocator(byte* pool, std::size_t size) :
	mConfiguredChunkSize(0),
	mConfiguredNumChunks(0),
	mAllocatedSize(0),
	mDeviceBasePointer(NULL),
	mFragmentBlockHeadFree(NULL),
	mFragmentBlockHead(NULL)
{
	BOOST_ASSERT(pool != NULL);
	BOOST_ASSERT(size > 0);

	// set the configuration
	mConfiguredChunkSize = 1024;
	mConfiguredNumChunks = size / mConfiguredChunkSize;

	printf("trying to reserve %ld bytes (%ld KB) (%ld MB), with chunk size %ld bytes (%ld KB) (%ld MB)\n", size, size/1024, size/(1024*1024), mConfiguredChunkSize, mConfiguredChunkSize/1024, mConfiguredChunkSize/(1024*1024));

	// save the base pointer
	mAllocatedSize = 0;
	mDeviceBasePointer = pool;

	// initialize allocation list
	mFragmentBlockHead = new FragmentBlock;

	mFragmentBlockHead->free = true;
	mFragmentBlockHead->offset = 0;
	mFragmentBlockHead->size = size;

	mFragmentBlockHead->nextFree = mFragmentBlockHead->prevFree = NULL;
	mFragmentBlockHead->nextBlock = mFragmentBlockHead->prevBlock = NULL;

	mFragmentBlockHeadFree = mFragmentBlockHead;
}

FragmentAllocator::~FragmentAllocator()
{
	BOOST_ASSERT(mDeviceBasePointer != NULL);

	BOOST_ASSERT(mAllocatedSize == 0);

	mDeviceBasePointer = NULL;

	mConfiguredChunkSize = 0;
	mConfiguredNumChunks = 0;

	// clean up memory block
	FragmentBlock* currentBlock = mFragmentBlockHead;
	while(currentBlock)
	{
		FragmentBlock* next = currentBlock->nextBlock;
		delete currentBlock;
		currentBlock = next;
	}

	mFragmentBlockHeadFree = NULL;
	mFragmentBlockHead = NULL;

	// clean up allocation map
	// note that the map should be empty
	BOOST_ASSERT(mPointerToBlockMap.empty());
	for(PointerToBlockMap::iterator it = mPointerToBlockMap.begin(); it != mPointerToBlockMap.end(); ++it)
	{
		//SAFE_DELETE(it->first);
		delete it->first;
	}
	mPointerToBlockMap.clear();
}

bool FragmentAllocator::allocate(MutablePointer **pointer, std::size_t size)
{
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
	tbb::mutex::scoped_lock lock(mAllocationLock);
#endif

	printf("trying to allocate %ld bytes (%ld KB) (%ld MB)\n", size, size/1024, size/(1024*1024));

	if(!mFragmentBlockHeadFree)
		return false;

	// round the requested size to multiple of chunk size
	if(size % mConfiguredChunkSize != 0)
		size = ((size / mConfiguredChunkSize) + 1) * mConfiguredChunkSize;

	// find the free block with enough free space
	FragmentBlock* currentBlock = mFragmentBlockHeadFree;
	bool found = false;
	{
		while (currentBlock)
		{
			// current block has enough size, end search
			if (currentBlock->size >= size)
			{
				found = true;
				break;
			}

			// no remaining free blocks, end search
			if (!currentBlock->nextFree)
			{
				found = false;
				break;
			}

			// continue on next free block
			currentBlock = currentBlock->nextFree;
		}

		if (!found)
		{
			printf("no available free block\n");
			return false;
		}
	}

	// as we found the free block,...
	// insert new allocated block on the left of current block
	FragmentBlock* newBlock = NULL;
	{
		newBlock = new FragmentBlock;
		newBlock->free = false;
		newBlock->offset = currentBlock->offset;
		newBlock->size = size;

		// Nullize the free link
		newBlock->prevFree = NULL;
		newBlock->nextFree = NULL;

		// update the block link between new block and prev block
		newBlock->prevBlock = currentBlock->prevBlock;
		if(currentBlock->prevBlock)
			currentBlock->prevBlock->nextBlock = newBlock;

		// update the block link between  new block and current block
		currentBlock->prevBlock = newBlock;
		newBlock->nextBlock = currentBlock;
	}

	// check if current block, which is going to be delete, is the head of all blocks
	if(currentBlock == mFragmentBlockHead)
		mFragmentBlockHead = currentBlock->prevBlock;

	// modify the existing free block
	{
		if (currentBlock->size == size)
		{
			// maintain the double linked-list
			if (currentBlock->prevFree)
				currentBlock->prevFree->nextFree = currentBlock->nextFree;

			if (currentBlock->nextFree)
				currentBlock->nextFree->prevFree = currentBlock->prevFree;

			if (currentBlock->prevBlock)
				currentBlock->prevBlock->nextBlock = currentBlock->nextBlock;

			if (currentBlock->nextBlock)
				currentBlock->nextBlock->prevBlock = currentBlock->prevBlock;

			// check if current block, which is going to be delete, is the head of free blocks
			if (currentBlock == mFragmentBlockHeadFree)
				mFragmentBlockHeadFree = currentBlock->nextFree;

			SAFE_DELETE(currentBlock);
		}
		else
		{
			currentBlock->offset += size;
			currentBlock->size   -= size;
		}
	}

	// create the mutable pointer
	*pointer = new MutablePointer(mDeviceBasePointer + newBlock->offset);

	// set the reference pointer
	newBlock->pointerReference = (*pointer)->pointerReference;

	// insert new entry in allocation map
	mPointerToBlockMap[(*pointer)->pointerReference] = newBlock;

	// bookkeeping the allocated size
	mAllocatedSize += size;

	std::size_t availmem = available();
	printf("allocated %ld bytes (%ld KB) (%ld MB), free memory %ld bytes (%ld KB) (%ld MB)\n", size, size/1024, size/(1024*1024), availmem, availmem/1024, availmem/(1024*1024));
	return true;
}

bool FragmentAllocator::deallocate(MutablePointer *pointer)
{
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
	tbb::mutex::scoped_lock lock(mAllocationLock);
#endif

	if(!pointer)
		return false;

	if(!pointer->pointerReference)
		return false;

	// find the memory block from allocation map
	PointerToBlockMap::iterator it = mPointerToBlockMap.find(pointer->pointerReference);

	// if we cannot find the allocation, return fail
	if(it == mPointerToBlockMap.end())
		return false;

	// we have the corresponding memory block now
	FragmentBlock* currentBlock = it->second;

	// delete the entry in allocation map
	mPointerToBlockMap.erase(it);

	// free the mutable pointer object
	SAFE_DELETE(pointer);

	// bookkeeping the allocated size
	mAllocatedSize -= currentBlock->size;

	// four cases: merge left, merge right, merge both left and right, and no merge
	FragmentBlock* prevBlock = currentBlock->prevBlock;
	FragmentBlock* nextBlock = currentBlock->nextBlock;

	// determine whether left/right is a free block
	bool isPrevFree = false;
	bool isNextFree = false;
	{
		if (prevBlock)
			if (prevBlock->free)
				isPrevFree = true;
		if (nextBlock)
			if (nextBlock->free)
				isNextFree = true;
	}

	// when left and right block are both free, merge left and right block into one free block
	// (here we delete the prev/next block and keep the current block)
	if(isPrevFree && isNextFree)
	{
		currentBlock->prevFree = prevBlock->prevFree;
		currentBlock->nextFree = nextBlock->nextFree;
		currentBlock->prevBlock = prevBlock->prevBlock;
		currentBlock->nextBlock = nextBlock->nextBlock;

		if(prevBlock->prevFree)
			prevBlock->prevFree->nextFree = currentBlock;
		if(nextBlock->nextFree)
			nextBlock->nextFree->prevFree = currentBlock;
		if(prevBlock->prevBlock)
			prevBlock->prevBlock->nextBlock = currentBlock;
		if(nextBlock->nextBlock)
			nextBlock->nextBlock->prevBlock = currentBlock;

		currentBlock->free = true;
		currentBlock->offset = prevBlock->offset;
		currentBlock->size = prevBlock->size + currentBlock->size + nextBlock->size;

		BOOST_ASSERT(mFragmentBlockHeadFree != NULL);

		// check if prev block, which is going to be delete, is the head of free blocks
		if (prevBlock == mFragmentBlockHeadFree)
			mFragmentBlockHeadFree = currentBlock;

		// check if prev block, which is going to be delete, is the head of all blocks
		if(prevBlock == mFragmentBlockHead)
			mFragmentBlockHead = currentBlock;

		SAFE_DELETE(prevBlock);
		SAFE_DELETE(nextBlock);
	}
	// when previous block is free and next is not free, merge current block into previous block (toward left)
	// (here we delete the current block and keep previous block)
	else if(isPrevFree && !isNextFree)
	{
		prevBlock->nextBlock = nextBlock;

		if(nextBlock)
			nextBlock->prevBlock = prevBlock;

		prevBlock->size += currentBlock->size;

		BOOST_ASSERT(mFragmentBlockHeadFree != NULL);
		BOOST_ASSERT(currentBlock != mFragmentBlockHeadFree);
		BOOST_ASSERT(currentBlock != mFragmentBlockHead);

		SAFE_DELETE(currentBlock);
	}
	// when next block is free and previous block is not free, merge current block into previous block (toward right)
	// (here we delete the current block and keep next block)
	else if(!isPrevFree && isNextFree)
	{
		nextBlock->prevBlock = prevBlock;

		if(prevBlock)
			prevBlock->nextBlock = nextBlock;

		nextBlock->offset = currentBlock->offset;
		nextBlock->size  += currentBlock->size;

		BOOST_ASSERT(mFragmentBlockHeadFree != NULL);
		BOOST_ASSERT(currentBlock != mFragmentBlockHeadFree);

		// check if current block, which is going to be delete, is the head of all blocks
		if(currentBlock == mFragmentBlockHead)
			mFragmentBlockHead = currentBlock->nextBlock;

		SAFE_DELETE(currentBlock);
	}
	// when both next block and previous block are not free, set the current block as free block
	// and search toward left and right to update free block link
	else
	{
		// free the current block
		currentBlock->free = true;

		// search toward left for free block to update the free link
		bool foundPrevFreeBlock = false;
		{
			while (prevBlock)
			{
				if (prevBlock->free)
				{
					foundPrevFreeBlock = true;
					break;
				}
				prevBlock = prevBlock->prevBlock;
			}

			if (foundPrevFreeBlock)
			{
				// if we found previous free block, then we know the next free block (which is on the right side)
				nextBlock = prevBlock->nextFree;

				// update free link for both left and right side
				prevBlock->nextFree = currentBlock;

				currentBlock->prevFree = prevBlock;
				currentBlock->nextFree = nextBlock;

				if(nextBlock)
					nextBlock->prevFree = currentBlock;
			}
			else
			{
				currentBlock->prevFree = NULL;

				// the current block is the first free block of all blocks
				mFragmentBlockHeadFree = currentBlock;
			}
		}

		bool foundNextFreeBlock = false;
		// when we didn't found previous free block, it's still possible that there's free block on the right
		// search toward right for free block to update the free link
		if(!foundPrevFreeBlock)
		{
			while(nextBlock)
			{
				if(nextBlock->free)
				{
					foundNextFreeBlock = true;
					break;
				}
				nextBlock = nextBlock->nextBlock;
			}

			if(foundNextFreeBlock)
			{
				// if we found next free block, then we know the previous free block (which is on the left side) (which should be NULL)
				prevBlock = nextBlock->prevFree;

				BOOST_ASSERT(prevBlock == NULL);

				// update the free link for both left and right side
				nextBlock->prevFree = currentBlock;

				currentBlock->prevFree = prevBlock;
				currentBlock->nextFree = nextBlock;

				if(prevBlock)
					prevBlock->nextFree = currentBlock;
			}
			else
			{
				currentBlock->nextFree = NULL;
			}
		}
	}

	return true;
}

size_t FragmentAllocator::total()
{
	return mConfiguredChunkSize * mConfiguredNumChunks;
}

size_t FragmentAllocator::used()
{
	return mAllocatedSize;
}

size_t FragmentAllocator::available()
{
	return total() - used();
}

bool FragmentAllocator::isValid(MutablePointer*pointer)
{
	// find the memory block from allocation map
	PointerToBlockMap::iterator it = mPointerToBlockMap.find(pointer->pointerReference);

	// if we cannot find the allocation, return fail
	if(it == mPointerToBlockMap.end())
		return false;

	return true;
}

size_t FragmentAllocator::infoSize(MutablePointer*pointer)
{
	// find the memory block from allocation map
	PointerToBlockMap::iterator it = mPointerToBlockMap.find(pointer->pointerReference);

	// if we cannot find the allocation, return fail
	if(it == mPointerToBlockMap.end())
		return false;

	// we have the corresponding memory block now
	FragmentBlock* currentBlock = it->second;

	return currentBlock->size;
}

void FragmentAllocator::debug()
{
	if(mFragmentBlockHead)
	{
		FragmentBlock* currentBlock = mFragmentBlockHead;
		if(currentBlock->prevBlock)
		{
			printf("\thead block has prevBlock entry\n");
		}
		if(currentBlock->prevFree)
		{
			printf("\thead block has prevFree entry\n");
		}

		bool isNoMoreFreeBlock = false;
		size_t sumSize = 0;
		size_t sumUsed = 0;
		while(currentBlock)
		{
			printf("<--> (%p, %s, offset=%ld, size=%ld (%ld KB) (%ldMB) )\n", currentBlock, (currentBlock->free) ? "   free" : "nonfree", currentBlock->offset, currentBlock->size, currentBlock->size/1024, currentBlock->size/(1024*1024));

			sumSize += currentBlock->size;
			sumUsed += (currentBlock->free) ? currentBlock->size : 0;

			if(currentBlock->nextBlock)
			{
				if(currentBlock->nextBlock->offset !=  currentBlock->offset + currentBlock->size)
				{
					printf("\tnext offset error\n");
				}
				if(currentBlock->nextBlock->prevBlock != currentBlock)
				{
					printf("\tprevBlock link error\n");
				}
			}

			if(currentBlock->free)
			{
				if(isNoMoreFreeBlock)
				{
					printf("\tno more free block is defined, but another free block found\n");
				}

				if(currentBlock->nextFree)
				{
					if(currentBlock->nextFree == currentBlock->nextBlock)
					{
						printf("\tconsecutive free block error\n");
					}
					if(currentBlock->nextFree->prevFree != currentBlock)
					{
						printf("\tprev free link error\n");
					}
				}
				else
				{
					isNoMoreFreeBlock = true;
				}
			}
			else
			{
				if(currentBlock->prevFree)
				{
					printf("\tnonfree block has prevFree link\n");
				}
				if(currentBlock->nextFree)
				{
					printf("\tnonfree block has nextFree link\n");
				}
			}

			currentBlock = currentBlock->nextBlock;
		}

		if(sumSize != mConfiguredChunkSize * mConfiguredNumChunks)
		{
			printf("\ttotal reserved size does not equal to sum size\n");
		}

		printf("total size: %ld bytes (%ld KB) (%ld MB)\n", sumSize, sumSize/1024, sumSize/(1024*1024));
		printf("used  size: %ld bytes (%ld KB) (%ld MB)\n\n", sumUsed, sumUsed/1024, sumUsed/(1024*1024));
	}
	else
	{
		printf("mFragmentBlockHead == NULL\n");
	}

	if(mFragmentBlockHeadFree)
	{
		FragmentBlock* currentBlock = mFragmentBlockHeadFree;
		if(currentBlock->prevFree)
		{
			printf("\thead free block has prevFree entry\n");
		}

		size_t sumSize = 0;
		while(currentBlock)
		{
			printf("<--> (%p, %s, offset=%ld, size=%ld (%ld KB) (%ld MB))\n", currentBlock, (currentBlock->free) ? "   free" : "nonfree", currentBlock->offset, currentBlock->size, currentBlock->size/1024, currentBlock->size/(1024*1024));

			sumSize += currentBlock->size;

			if(!currentBlock->free)
			{
				printf("\tcurrent block is not free\n");
			}
			else
			{
				if(currentBlock->nextFree)
				{
					if(currentBlock->nextFree->prevFree != currentBlock)
					{
						printf("\tincorrect free link\n");
					}
					if(currentBlock->nextFree->prevFree != currentBlock)
					{
						printf("\tprev free link error\n");
					}
				}
			}

			currentBlock = currentBlock->nextFree;
		}
		printf("free size: %ld bytes (%ld KB) (%ld MB)\n", sumSize, sumSize/1024, sumSize/(1024*1024));
	}
	else
	{
		printf("mFragmentBlockHeadFree = NULL\n");
	}
}

FragmentFreeOperator::FragmentFreeOperator(boost::function< void(void*,void*,std::size_t) > copyFunctor) :
	mCopyFunctor(copyFunctor)
{ }

bool FragmentFreeOperator::operator() (FragmentAllocator& allocator)
{
#if ZILLIANS_FRAGMENTFREEALLOCATOR_ENABLE_CONCURRENT_ALLOCATION
	tbb::mutex::scoped_lock lock(allocator.mAllocationLock);
#endif

	FragmentBlock* currentBlock = allocator.mFragmentBlockHead;
	if(!currentBlock)
		return true;

	size_t sumOffset = 0;
	size_t sumFreeSpace = 0;
	FragmentBlock* lastAllocatedBlock = NULL;
	while(true)
	{
		FragmentBlock* nextBlock = NULL;

		if(currentBlock->free)
		{
			sumFreeSpace += currentBlock->size;

			if(currentBlock->prevBlock)
				currentBlock->prevBlock->nextBlock = currentBlock->nextBlock;
			if(currentBlock->nextBlock)
				currentBlock->nextBlock->prevBlock = currentBlock->prevBlock;

			nextBlock = currentBlock->nextBlock;

			if(currentBlock == allocator.mFragmentBlockHead)
				allocator.mFragmentBlockHead = nextBlock;

			SAFE_DELETE(currentBlock);
		}
		else
		{
			lastAllocatedBlock = currentBlock;

			sumOffset += currentBlock->size;

			if(sumFreeSpace > 0)
			{
				// migrate memory block
				byte* dstPtr = currentBlock->pointerReference->data - sumFreeSpace;
				byte* srcPtr = currentBlock->pointerReference->data;

				// copy through given copy functor
				// note that the copy functor must support overlapped copy (i.e. the source and destination memory region are overlapped)
				mCopyFunctor(dstPtr, srcPtr, currentBlock->size);
				currentBlock->offset -= sumFreeSpace;
			}

			nextBlock = currentBlock->nextBlock;
		}

		if(!nextBlock)
			break;

		currentBlock = nextBlock;
	}

	// append a single free memory block if there's free space
	if(sumFreeSpace > 0)
	{
		FragmentBlock* freeBlock = new FragmentBlock;
		freeBlock->free = true;
		freeBlock->offset = sumOffset;
		freeBlock->size = sumFreeSpace;

		freeBlock->prevFree = freeBlock->nextFree = NULL;
		freeBlock->prevBlock = lastAllocatedBlock;
		lastAllocatedBlock->nextBlock = freeBlock;
		freeBlock->nextBlock = NULL;

		allocator.mFragmentBlockHeadFree = freeBlock;
	}
	else
	{
		allocator.mFragmentBlockHeadFree = NULL;
	}

	return true;
}
}
