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
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/**
 * @file BufferManager.cpp
 * @todo Put file description here.
 *
 * @date 2009/02/12 nothing - Initial version created.
 */
#include "core-api/SharedPtr.h"
#include "core-api/Buffer.h"
#include "core-api/BufferManager.h"

namespace zillians {

BufferManager::BufferManager() : mFreeBufferList(new std::list<Buffer*>)
{
	// Create a pool of buffers
	int32 cnt = BufferManagerPolicy::getInitialBufferPoolSize();
	for(;cnt > 0; --cnt)
	{
		mFreeBufferList->push_back(new Buffer());
	}

	// TODO: Allocate mMemory, current code is for testing only
	mMemorySize = TEST_MEMORY_SIZE;
	mMemory = malloc(mMemorySize);

	// Initialize policy-specific stuff
	BufferManagerPolicy::startup(this);

	// NOTE: This call is for a naive implementation of memory management, to be changed.
	mmStartup();
}


BufferManager::~BufferManager()
{
	// NOTE: This call is for a naive implementation of memory management, to be changed.
	mmShutdown();

	// Shutdown any policy-specific stuff
	BufferManagerPolicy::shutdown(this);

	// TODO: Deallocate mMemory, current code is for testing only
	free(mMemory);

	// Delete buffer pool
	for(std::list<Buffer*>::iterator i = mFreeBufferList->begin(); i != mFreeBufferList->end(); i++)
	{
		SAFE_DELETE(*i);
	}
	SAFE_DELETE(mFreeBufferList);
}

SharedPtr<Buffer> BufferManager::allocateBuffer(size_t size)
{
	// Lock this scope!!!
	tbb::spin_mutex::scoped_lock lock(mLock);
	SharedPtr<Buffer> retBuf(mFreeBufferList->front(), boost::bind(&BufferManager::returnBuffer, this, _1));

	//TODO: Allocate memory
	byte* p = this->malloc(size);
	if( p == NULL ) { return NULL; }
	refBuf->_setData(p);
	refBuf->_setSize(size);

	mFreeBufferList->pop_front();
	return retBuf;
}


void BufferManager::returnBuffer(Buffer* buf)
{
	// Lock this scope!!!
	tbb::spin_mutex::scoped_lock lock(mLock);
	//TODO: Deallocate memory
	mFreeBufferList->push_front(buf->mHead);
}


byte* BufferManager::malloc(size_t sz)
{
	MemBlock* p = mMemHead;
	// Find next available block
	while( (p->sz < sz) && (p != NULL) && (p->used) )
	{
		p = p->next;
	}
	if(NULL == p) { return NULL; }// Failed to alloc

	if(p->sz == sz)
	{
		p->used = true;
		return p->ptr;
	}
	else
	{
		MemBlock* pUsed = new MemBlock;
		pUsed->used = true;
		pUsed->sz = sz;
		pUsed->prev = p->prev;
		pUsed->next = p;
		if(p->prev != NULL) { p->prev->next = pUsed; }
		p->prev = pUsed;
		p->ptr += sz;
		p->sz -= sz;
	}
}


void BufferManager::free(byte* ptr)
{
	MemBlock* p = mMemHead;
	MemBlock* tmp = NULL;
	while( (p->ptr != ptr) && (p != NULL) )
	{
		p = p->next;
	}
	assert(NULL != p);// Should not happen

	if( (p->prev != NULL) && (p->prev->used == false) )// merge with previous block
	{
		tmp = p;
		p->prev->sz += p->sz;
		p->prev->next = p->next;
		p = p->prev;
		delete tmp;
	}
	if( (p->next != NULL) && (p->next->used == false) )// merge with next block
	{
		tmp = p->next;
		p->sz += p->next->sz;
		p->next = p->next->next;
		if(p->next != NULL)
		{
			p->next->prev = p;
		}
		delete tmp;
	}
	p->used = false;
}

void BufferManager::mmStartup()
{
	mMemHead = new MemBlock;
	mMemHead->ptr = mMemory;
	mMemHead->sz = mMemorySize;
	mMemHead->prev = NULL;
	mMemHead->next = NULL;
}

void BufferManager::mmShutdown()
{
	MemBlock* p = mMemHead;
	while(mMemHead != NULL)
	{
		p = mMemHead->next;
		SAFE_DELETE(mMemHead);
		mMemHead = p;
	}
}

}//zillians
