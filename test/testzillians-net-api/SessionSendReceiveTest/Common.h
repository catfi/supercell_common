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
 * @date Aug 13, 2009 sdk - Initial version created.
 */

#include "core-api/Prerequisite.h"
#include "net-api/sys/Session.h"
#include "net-api/sys/SessionEngine.h"

using namespace zillians::net::sys;

struct MySessionContext
{
	uint32 id;
	uint32 type;
	shared_ptr<Buffer> buffer;
};

MySessionContext* createContext(uint32 id)
{
	MySessionContext* ctx = new MySessionContext();
	ctx->id = id;
	ctx->buffer.reset(new Buffer(4096));
	return ctx;
}

void sendTestMessage(TcpSession& session)
{
	shared_ptr<Buffer> a0(new Buffer(1024));
	shared_ptr<Buffer> a1(new Buffer(1024));
	shared_ptr<Buffer> a2(new Buffer(1024));
	shared_ptr<Buffer> a3(new Buffer(1024));

	for(int32 i=0;i<1024/sizeof(int32);++i)
	{
		int32 x = 0;
		a0->write(x);
	}
	for(int32 i=0;i<1024/sizeof(int32);++i)
	{
		int32 x = 1;
		a1->write(x);
	}
	for(int32 i=0;i<1024/sizeof(int32);++i)
	{
		int32 x = 2;
		a2->write(x);
	}
	for(int32 i=0;i<1024/sizeof(int32);++i)
	{
		int32 x = 3;
		a3->write(x);
	}

	shared_ptr<BufferCollection> collection(new BufferCollection);
	collection->add(a0);
	collection->add(a1);
	collection->add(a2);
	collection->add(a3);

	session.write(0, collection);
}

bool verifyTestMessage(TcpSession& session, shared_ptr<Buffer> buffer)
{
	for(int32 i=0;i<4096/sizeof(int32);++i)
	{
		int32 x;
		buffer->read(x);

		if(i < (1024/sizeof(int32)))
		{
			if(x != 0)
			{
				printf("verification failed at %d, expected %d, got %d\n", i, 0, x);
				return false;
			}
		}
		else if(i < (1024/sizeof(int32))*2)
		{
			if(x != 1)
			{
				printf("verification failed at %d, expected %d, got %d\n", i, 1, x);
				return false;
			}
		}
		else if(i < (1024/sizeof(int32))*3)
		{
			if(x != 2)
			{
				printf("verification failed at %d, expected %d, got %d\n", i, 2, x);
				return false;
			}
		}
		else if(i < (1024/sizeof(int32))*4)
		{
			if(x != 3)
			{
				printf("verification failed at %d, expected %d, got %d\n", i, 3, x);
				return false;
			}
		}
	}

	return true;
}
