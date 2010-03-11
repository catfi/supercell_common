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

#include "net-api/queue/impl/ev/LibEvProcessor.h"

namespace zillians {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr LibEvProcessor::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.provider.ev.LibEvProcessor"));

//////////////////////////////////////////////////////////////////////////
LibEvProcessor::LibEvProcessor(const ev::loop_ref &loop) : mLoop(loop)
{
	mTerminated = false;

	createDefaultWatchers();
}

LibEvProcessor::~LibEvProcessor()
{
	terminate();
}

//////////////////////////////////////////////////////////////////////////
void LibEvProcessor::evIoStart(ev::io *w, Callback *cb)
{
	mIoReqQueue.requests.push( LibEvReqPair<ev::io*>(w, OP_IO_START, cb) );
	//if(!mIoReqQueue.watcher.async_pending())
		mIoReqQueue.watcher.send();
}

void LibEvProcessor::evIoStop(ev::io *w, Callback *cb)
{
	mIoReqQueue.requests.push( LibEvReqPair<ev::io*>(w, OP_IO_STOP, cb) );
	//if(!mIoReqQueue.watcher.async_pending())
		mIoReqQueue.watcher.send();
}

void LibEvProcessor::evIoRestart(ev::io *w, Callback *cb)
{
	mIoReqQueue.requests.push( LibEvReqPair<ev::io*>(w, OP_IO_RESTART, cb) );
	//if(!mIoReqQueue.watcher.async_pending())
		mIoReqQueue.watcher.send();
}

void LibEvProcessor::evTimerStart(ev::timer *w, Callback *cb)
{
	mTimerReqQueue.requests.push( LibEvReqPair<ev::timer*>(w, OP_TIMER_START, cb) );
	//if(!mTimerReqQueue.watcher.async_pending())
		mTimerReqQueue.watcher.send();
}

void LibEvProcessor::evTimerStop(ev::timer *w, Callback *cb)
{
	mTimerReqQueue.requests.push( LibEvReqPair<ev::timer*>(w, OP_TIMER_STOP, cb) );
	//if(!mTimerReqQueue.watcher.async_pending())
		mTimerReqQueue.watcher.send();
}

void LibEvProcessor::evTimerAgain(ev::timer *w, Callback *cb)
{
	mTimerReqQueue.requests.push( LibEvReqPair<ev::timer*>(w, OP_TIMER_AGAIN, cb) );
	//if(!mTimerReqQueue.watcher.async_pending())
		mTimerReqQueue.watcher.send();
}

void LibEvProcessor::evAsyncStart(ev::async *w, Callback *cb)
{
	mAsyncReqQueue.requests.push( LibEvReqPair<ev::async*>(w, OP_ASYNC_START, cb) );
	//if(!mAsyncReqQueue.watcher.async_pending())
		mAsyncReqQueue.watcher.send();
}

void LibEvProcessor::evAsyncStop(ev::async *w, Callback *cb)
{
	mAsyncReqQueue.requests.push( LibEvReqPair<ev::async*>(w, OP_ASYNC_STOP, cb) );
	//if(!mAsyncReqQueue.watcher.async_pending())
		mAsyncReqQueue.watcher.send();
}

//////////////////////////////////////////////////////////////////////////
void LibEvProcessor::run()
{
	while(!mTerminated)
	{
		mLoop.loop();
	}
}

void LibEvProcessor::terminate()
{
	if(!mTerminated)
	{
		mTerminated = true;
		mLoop.unloop();
	}
}

//////////////////////////////////////////////////////////////////////////
void LibEvProcessor::createDefaultWatchers()
{
	mIoReqQueue.watcher.set(mLoop);
	mIoReqQueue.watcher.set<LibEvProcessor, &LibEvProcessor::handleIoReq>(this);
	mIoReqQueue.watcher.start();

	mTimerReqQueue.watcher.set(mLoop);
	mTimerReqQueue.watcher.set<LibEvProcessor, &LibEvProcessor::handleTimerReq>(this);
	mTimerReqQueue.watcher.start();

	mAsyncReqQueue.watcher.set(mLoop);
	mAsyncReqQueue.watcher.set<LibEvProcessor, &LibEvProcessor::handleAsyncReq>(this);
	mAsyncReqQueue.watcher.start();
}

//////////////////////////////////////////////////////////////////////////
void LibEvProcessor::handleIoReq(ev::async &w, int revents)
{
	LibEvReqPair<ev::io*> request;
	while(mIoReqQueue.requests.pop_if_present(request))
	{
		request.w->set(mLoop);
		switch(request.op)
		{
		case OP_IO_START:
			request.w->start(); //printf("IO START\n");
			break;
		case OP_IO_STOP:
			request.w->stop();  //printf("IO STOP\n");
			break;
		case OP_IO_RESTART:
			request.w->stop(); request.w->start(); //printf("IO RESTART\n");
			break;
		}
		if(request.cb) request.cb->invoke();
	}
}

void LibEvProcessor::handleTimerReq(ev::async &w, int revents)
{
	LibEvReqPair<ev::timer*> request;
	while(mTimerReqQueue.requests.pop_if_present(request))
	{
		request.w->set(mLoop);
		switch(request.op)
		{
		case OP_TIMER_START:
			request.w->start(); request.w->again(); //printf("TIMER START\n");
			break;
		case OP_TIMER_STOP:
			request.w->stop();  //printf("TIMER STOP\n");
			break;
		case OP_TIMER_AGAIN:
			request.w->again(); //printf("TIMER AGAIN\n");
			break;
		}
		if(request.cb) request.cb->invoke();
	}
}

void LibEvProcessor::handleAsyncReq(ev::async &w, int revents)
{
	LibEvReqPair<ev::async*> request;
	while(mAsyncReqQueue.requests.pop_if_present(request))
	{
		request.w->set(mLoop);
		switch(request.op)
		{
		case OP_ASYNC_START:
			request.w->start(); //printf("ASYNC START\n");
			break;
		case OP_ASYNC_STOP:
			request.w->stop();  //printf("ASYNC STOP\n");
			break;
		case OP_ASYNC_SEND:
			request.w->send();  //printf("ASYNC SEND\n");
			break;
		}
		if(request.cb) request.cb->invoke();
	}
}

}
