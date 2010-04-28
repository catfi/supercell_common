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
 * @date May 21, 2009 sdk - Initial version created.
 */

#include "net-api/sys/buffer_manager/TcpBufferManager.h"

namespace zillians { namespace net {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr TcpBufferManager::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.sys.buffermgr.TcpBufferManager"));

//////////////////////////////////////////////////////////////////////////
TcpBufferManager::TcpBufferManager(size_t poolSize)
{
	mPoolSize = poolSize;
	mPool = new byte[mPoolSize];

	mAllocator = new ScalablePoolAllocator(mPool, mPoolSize);
}

TcpBufferManager::~TcpBufferManager()
{
	SAFE_DELETE(mAllocator);

	SAFE_DELETE_ARRAY(mPool);
	mPoolSize = 0;
}

//////////////////////////////////////////////////////////////////////////
SharedPtr<Buffer> TcpBufferManager::createBuffer(size_t size)
{
	Buffer* buffer = allocateBuffer(size);

	if(!buffer)
		return SharedPtr<Buffer>();
	else
		return SharedPtr<Buffer>(buffer, boost::bind(&TcpBufferManager::returnBuffer, this, _1));
}

SharedPtr<Buffer> TcpBufferManager::sliceBuffer(SharedPtr<Buffer> original, size_t size)
{
	SharedPtr<Buffer> new_buffer(new Buffer(original->rptr(), size));

	if(size == 0)
	{
		new_buffer->wpos(original->dataSize());
		original->rpos(original->wpos());
	}
	else
	{
		new_buffer->wpos(size);
		original->rpos(original->rpos() + size);
	}

	new_buffer->setContext(boost::static_pointer_cast<void>(original));

	return new_buffer;
}

SharedPtr<Buffer> TcpBufferManager::cloneBuffer(SharedPtr<Buffer> original)
{
	SharedPtr<Buffer> new_buffer(new Buffer(original->mData, original->mAllocatedSize));

	new_buffer->rpos(original->rpos());
	new_buffer->wpos(original->wpos());

	new_buffer->setContext(boost::static_pointer_cast<void>(original));

	return new_buffer;
}

//////////////////////////////////////////////////////////////////////////
Buffer* TcpBufferManager::allocateBuffer(size_t size)
{
	byte* data = mAllocator->allocate(size);
	if(!data) return NULL;

	Buffer* buffer = new Buffer(data, size);

	return buffer;
}

void TcpBufferManager::returnBuffer(Buffer* buffer)
{
	mAllocator->deallocate(buffer->mData);

	SAFE_DELETE(buffer);
}

} }
