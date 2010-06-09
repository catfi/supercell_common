/**
 * Zillians MMO
 * Copyright (C) 2007-2008 Zillians.com, Inc.
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

#include "networking/rdma/Poller.h"

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr Poller::mLogger(log4cxx::Logger::getLogger("zillians.common.networking.sys.Poller"));

//////////////////////////////////////////////////////////////////////////
Poller::Poller(const ev::loop_ref &loop) : mLoop(loop)
{
	mTerminated = false;

	createDefaultWatchers();
}

Poller::~Poller()
{
	terminate();
}

//////////////////////////////////////////////////////////////////////////
void Poller::start(ev::io *w, FastCallback *cb)
{
	mIoReqQueue.requests.push( PollerReqPair<ev::io*>(w, OP_IO_START, cb) );
	//if(!mIoReqQueue.watcher.async_pending())
		mIoReqQueue.watcher.send();
}

void Poller::stop(ev::io *w, FastCallback *cb)
{
	mIoReqQueue.requests.push( PollerReqPair<ev::io*>(w, OP_IO_STOP, cb) );
	//if(!mIoReqQueue.watcher.async_pending())
		mIoReqQueue.watcher.send();
}

void Poller::restart(ev::io *w, FastCallback *cb)
{
	mIoReqQueue.requests.push( PollerReqPair<ev::io*>(w, OP_IO_RESTART, cb) );
	//if(!mIoReqQueue.watcher.async_pending())
		mIoReqQueue.watcher.send();
}

void Poller::start(ev::async *w, FastCallback *cb)
{
	mAsyncReqQueue.requests.push( PollerReqPair<ev::async*>(w, OP_ASYNC_START, cb) );
	//if(!mAsyncReqQueue.watcher.async_pending())
		mAsyncReqQueue.watcher.send();
}

void Poller::stop(ev::async *w, FastCallback *cb)
{
	mAsyncReqQueue.requests.push( PollerReqPair<ev::async*>(w, OP_ASYNC_STOP, cb) );
	//if(!mAsyncReqQueue.watcher.async_pending())
		mAsyncReqQueue.watcher.send();
}

void Poller::start(ev::timer *w, FastCallback *cb)
{
	mTimerReqQueue.requests.push( PollerReqPair<ev::timer*>(w, OP_TIMER_START, cb) );
	//if(!mTimerReqQueue.watcher.async_pending())
		mTimerReqQueue.watcher.send();
}

void Poller::stop(ev::timer *w, FastCallback *cb)
{
	mTimerReqQueue.requests.push( PollerReqPair<ev::timer*>(w, OP_TIMER_STOP, cb) );
	//if(!mTimerReqQueue.watcher.async_pending())
		mTimerReqQueue.watcher.send();
}

void Poller::restart(ev::timer *w, FastCallback *cb)
{
	mTimerReqQueue.requests.push( PollerReqPair<ev::timer*>(w, OP_TIMER_AGAIN, cb) );
	//if(!mTimerReqQueue.watcher.async_pending())
		mTimerReqQueue.watcher.send();
}

//////////////////////////////////////////////////////////////////////////
void Poller::run()
{
	while(!mTerminated)
	{
		mLoop.loop();
	}
}

void Poller::terminate()
{
	if(!mTerminated)
	{
		mTerminated = true;
		mLoop.unloop();
	}
}

//////////////////////////////////////////////////////////////////////////
void Poller::createDefaultWatchers()
{
	mIoReqQueue.watcher.set(mLoop);
	mIoReqQueue.watcher.set<Poller, &Poller::handleIoReq>(this);
	mIoReqQueue.watcher.start();

	mTimerReqQueue.watcher.set(mLoop);
	mTimerReqQueue.watcher.set<Poller, &Poller::handleTimerReq>(this);
	mTimerReqQueue.watcher.start();

	mAsyncReqQueue.watcher.set(mLoop);
	mAsyncReqQueue.watcher.set<Poller, &Poller::handleAsyncReq>(this);
	mAsyncReqQueue.watcher.start();
}

//////////////////////////////////////////////////////////////////////////
void Poller::handleIoReq(ev::async &w, int revents)
{
	PollerReqPair<ev::io*> request;
	while(mIoReqQueue.requests.try_pop(request))
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

void Poller::handleTimerReq(ev::async &w, int revents)
{
	PollerReqPair<ev::timer*> request;
	while(mTimerReqQueue.requests.try_pop(request))
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

void Poller::handleAsyncReq(ev::async &w, int revents)
{
	PollerReqPair<ev::async*> request;
	while(mAsyncReqQueue.requests.try_pop(request))
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
		}
		if(request.cb) request.cb->invoke();
	}
}

} } }
