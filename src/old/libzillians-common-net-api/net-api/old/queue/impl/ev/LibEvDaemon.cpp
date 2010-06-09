//
// Zillians MMO
// Copyright (C) 2007-2008 Zillians.com, Inc.
// For more information see http://www.zillians.com
//
// Zillians MMO is the library and runtime for massive multiplayer online game
// development in utility computing model, which runs as a service for every
// developer to build their virtual world running on our GPU-assisted machines
//
// This is a close source library intended to be used solely within Zillians.com
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Contact Information: info@zillians.com
//

#include "networking/queue/impl/ev/LibEvDaemon.h"
#include <tbb/task_scheduler_init.h>
#include <tbb/tbb_allocator.h>
#include <tbb/tbb_thread.h>
#include <boost/bind.hpp>

namespace zillians {

//////////////////////////////////////////////////////////////////////////
LibEvDaemon::LibEvDaemon()
{
	// TODO we could have multiple read/write/checker processor depending on number of processor available...
	// create processors
	LibEvProcessor *p0 = new LibEvProcessor(ev_loop_new(0));
	LibEvProcessor *p1 = new LibEvProcessor(ev_loop_new(0));
	LibEvProcessor *p2 = new LibEvProcessor(ev_loop_new(0));

	// make association
	mReadProcessor = mCheckProcessor = p0;
	mWriteProcessor = p1;
	mAcceptProcessor = mConnectProcessor = p2;
}

LibEvDaemon::~LibEvDaemon()
{
	terminate();

	SAFE_DELETE(mReadProcessor);
	SAFE_DELETE(mWriteProcessor);
	SAFE_DELETE(mAcceptProcessor);
}

//////////////////////////////////////////////////////////////////////////
int32 LibEvDaemon::run()
{
	try
	{
		tbb::tbb_thread t0(boost::bind(&LibEvProcessor::run, *mReadProcessor));
		tbb::tbb_thread t1(boost::bind(&LibEvProcessor::run, *mWriteProcessor));
		tbb::tbb_thread t2(boost::bind(&LibEvProcessor::run, *mAcceptProcessor));

		if(t0.joinable()) t0.join();
		if(t1.joinable()) t1.join();
		if(t2.joinable()) t2.join();
	}
	catch(...)
	{
		return ZN_ERROR;
	}

	return ZN_OK;
}

int32 LibEvDaemon::terminate()
{
	try
	{
		mReadProcessor->terminate();
		mWriteProcessor->terminate();
		mAcceptProcessor->terminate();
	}
	catch(...)
	{
		return ZN_ERROR;
	}
	return ZN_OK;
}

}
