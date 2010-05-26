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

#ifndef TCPBUFFERMANAGER_H_
#define TCPBUFFERMANAGER_H_

#include "core-api/Prerequisite.h"
#include "core-api/Singleton.h"
#include "core-api/BufferManager.h"
#include "core-api/ScalablePoolAllocator.h"

namespace zillians { namespace net {

class TcpBufferManager : public zillians::BufferManager, public zillians::Singleton<TcpBufferManager>
{
public:
	TcpBufferManager(size_t poolSize);
	virtual ~TcpBufferManager();

public:
	virtual shared_ptr<Buffer> createBuffer(size_t size);
	virtual shared_ptr<Buffer> sliceBuffer(shared_ptr<Buffer> original, size_t size = 0);
	virtual shared_ptr<Buffer> cloneBuffer(shared_ptr<Buffer> original);

public:
	Buffer* allocateBuffer(size_t size);
	void returnBuffer(Buffer* buffer);

private:
	ScalablePoolAllocator* mAllocator;

private:
	byte* mPool;
	size_t mPoolSize;

private:
	static log4cxx::LoggerPtr mLogger;
};

} }

#endif/*TCPBUFFERMANAGER_H_*/
