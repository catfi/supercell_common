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
 * @file BufferManagerTest.cpp
 * Test on RdmaBufferManager, Buffer
 *
 * @date Feb 12, 2009 nothing - Initial version created.
 * @date Feb 16, 2009 nothing - Rewritten to fit new BufferManager design
 * @date Mar 2, 2009 nothing - Rewritten to fit new BufferManager design
 */
#include <iostream>
#include <vector>
#include "core-api/Buffer.h"
#include "core-api/BufferManager.h"
#include "net-api/sys/buffer_manager/RdmaBufferManager.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

using std::cout;

#define TEST_SIZE 1000000//20971520
#define TEST_BLOCK_SIZE 25535

int testRdmaBufMgr();
int testCudaBufMgr();
int testRun();

zillians::BufferManager* g_BufferMgr = NULL;
zillians::net::RdmaDeviceResourceManager* g_RdmaDevMgr = NULL;

int main(int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure();
	g_RdmaDevMgr = new zillians::net::RdmaDeviceResourceManager();

	int ret = 0;
	if( (ret = testRdmaBufMgr()) != 0 )
	{
		cout<<ret;
		return ret;
	}
	/*
	if( (ret = testCudaBufMgr()) != 0 )
	{
		return ret;
	}
	*/

	//cout<<"BufferManagerTest return code: "

	SAFE_DELETE(g_RdmaDevMgr);
	//cout<<ret;
	return ret;
}

int testRdmaBufMgr()
{
	g_BufferMgr = new zillians::net::RdmaBufferManager(TEST_SIZE);
	if( g_BufferMgr == NULL )
	{
		return -1;
	}

	int ret = testRun();

	SAFE_DELETE( g_BufferMgr );
	return ret;
}
/*
int testCudaBufMgr()
{
	g_BufferMgr = new zillians::CudaBufferManager(TEST_SIZE);
	if( g_BufferMgr == NULL )
	{
		return -1;
	}

	int ret = testRun();

	SAFE_DELETE( g_BufferMgr );
	return ret;
}
*/


/**
 * This test copies a piece of data into Buffer object then read it back, then
 * checks if the read data is identical to the data put in
 *
 * Used methods:
 * @li BufferManger::allocateBuffer()
 * @li BufferManager::returnBuffer()
 * @li Buffer::set()
 * @li Buffer::get()
 *
 * @return 0 for success, -1 for bad allocation, -2 for incorrect data read from Buffer object
 */
int testRun()
{
	std::vector<SharedPtr<zillians::Buffer> > Bufs;
	SharedPtr<zillians::Buffer> buf;
	byte* testData = new byte[TEST_BLOCK_SIZE];
	byte* testOut = new byte[TEST_BLOCK_SIZE];
	int ret = 0;

	for(int i = 0; i < TEST_BLOCK_SIZE; ++i)
	{
		testData[i] = static_cast<byte>(i % 256);
	}

	//buf = g_BufferMgr->allocateBuffer(TEST_BLOCK_SIZE);
	buf = g_BufferMgr->createBuffer(TEST_BLOCK_SIZE);

	if(buf != NULL)
	{
		buf->set(testData, 0, TEST_BLOCK_SIZE);
		buf->get(testOut, 0, TEST_BLOCK_SIZE);

		for(int i = 0; i < TEST_BLOCK_SIZE; ++i)
		{
			if(testData[i] != testOut[i])
			{
				ret = -2;
				break;
			}
		}
	}
	else
	{
		ret = -1;
	}
	delete[] testOut;
	delete[] testData;
//	if(buf != NULL)
//	{
//		g_BufferMgr->returnBuffer(buf);
//	}
//	else
//	{
//		return -3;
//	}

	// Test 2 - Allocate lots of buffers
	testData = new byte[TEST_BLOCK_SIZE];
	testOut = new byte[TEST_BLOCK_SIZE];
	for(int i = 0; i < TEST_BLOCK_SIZE; ++i)
	{
		testData[i] = static_cast<byte>(i % 256);
	}
	while(true)
	{
		int rnd = rand() % TEST_BLOCK_SIZE;
		//zillians::Buffer* pBuf = g_BufferMgr->allocateBuffer(rnd);
		SharedPtr<zillians::Buffer> pBuf = g_BufferMgr->createBuffer(rnd);
		if(pBuf == NULL) { break; }
		pBuf->set(testData, 0, rnd);
		Bufs.push_back(pBuf);
	}
	for(std::vector<SharedPtr<zillians::Buffer> >::iterator it = Bufs.begin(); it != Bufs.end(); ++it)
	{
		size_t sz = (*it)->allocatedSize();
		memset(testOut, 0, sz);
		(*it)->get(testOut, 0, sz);
		for(int i = 0; i < sz; ++i)
		{
			if(testData[i] != testOut[i])
			{
				ret = -4;
				break;
			}
		}
		if( ret == -4 ) { break; }
	}
	delete[] testOut;
	delete[] testData;

	Bufs.clear();
//	while( Bufs.size() > 0)
//	{
//		int idx = rand() % Bufs.size();
//		std::vector<zillians::Buffer*>::iterator i = Bufs.begin();
//		for(int c = 0; c != idx; c++, i++);
//		g_BufferMgr->returnBuffer(*i);
//		Bufs.erase(i);
//		//g_BufferMgr->returnBuffer(*(Bufs.begin()));
//		//Bufs.erase(Bufs.begin());
//	}
	return ret;
}
