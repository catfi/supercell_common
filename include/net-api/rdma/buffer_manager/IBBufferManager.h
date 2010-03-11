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
 * @date Feb 16, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_RDMA_IBBUFFERMANAGER_H_
#define ZILLIANS_NET_RDMA_IBBUFFERMANAGER_H_

#include "core-api/Prerequisite.h"
#include "core-api/Singleton.h"
#include "core-api/BufferManager.h"
#include "core-api/ScalablePoolAllocator.h"
#include "net-api/rdma/infiniband/IBDeviceResource.h"
#include "net-api/rdma/infiniband/IBDeviceResourceManager.h"

using namespace zillians;

namespace zillians { namespace net { namespace rdma {

class IBBufferManager : public zillians::BufferManager, public zillians::Singleton<IBBufferManager>
{
public:
	IBBufferManager(size_t poolSize);
	virtual ~IBBufferManager();

public:
	virtual SharedPtr<Buffer> createBuffer(size_t size);
	virtual SharedPtr<Buffer> sliceBuffer(SharedPtr<Buffer> original, size_t size = 0);
	virtual SharedPtr<Buffer> cloneBuffer(SharedPtr<Buffer> original);

public:
	Buffer* allocateBuffer(size_t size);
	void returnBuffer(Buffer* buffer);

public:
	std::size_t bytesInUsed();

private:
	ScalablePoolAllocator* mAllocator;

private:
	byte* mPool;
	size_t mPoolSize;
	tbb::atomic<std::size_t> mBytesInUsed;

private:
	static log4cxx::LoggerPtr mLogger;
};

} } }

#endif/*ZILLIANS_NET_RDMA_IBBUFFERMANAGER_H_*/
