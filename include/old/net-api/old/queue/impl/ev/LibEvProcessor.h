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

#ifndef LIBEVPROCESSOR_H_
#define LIBEVPROCESSOR_H_

#include "core-api/Types.h"
#include "core-api/Callback.h"
#include "net-api/queue/impl/ev/LibEvWrapper.h"
#include "tbb/concurrent_queue.h"
#include "boost/function.hpp"

namespace zillians {

typedef boost::function<void(void*)> LibEvProcessorCallback;

class LibEvProcessor
{
public:
	LibEvProcessor(const ev::loop_ref &loop);
	virtual ~LibEvProcessor();

public:
	void evIoStart   (ev::io *w, Callback* cb = NULL);
	void evIoStop    (ev::io *w, Callback* cb = NULL);
	void evIoRestart (ev::io *w, Callback* cb = NULL);
	void evTimerStart(ev::timer *w, Callback* cb = NULL);
	void evTimerStop (ev::timer *w, Callback* cb = NULL);
	void evTimerAgain(ev::timer *w, Callback* cb = NULL);
	void evAsyncStart(ev::async *w, Callback* cb = NULL);
	void evAsyncStop (ev::async *w, Callback* cb = NULL);

public:
	void run();
	void terminate();

private:
	void createDefaultWatchers();

private:
	void handleIoReq   (ev::async &w, int revents);
	void handleTimerReq(ev::async &w, int revents);
	void handleAsyncReq(ev::async &w, int revents);

private:
	static log4cxx::LoggerPtr mLogger;

private:
	enum OP_IO
	{
		OP_IO_START,
		OP_IO_STOP,
		OP_IO_RESTART,
	};

	enum OP_TIMER
	{
		OP_TIMER_START,
		OP_TIMER_STOP,
		OP_TIMER_AGAIN,
	};

	enum OP_ASYNC
	{
		OP_ASYNC_START,
		OP_ASYNC_STOP,
		OP_ASYNC_SEND,
	};

	template <typename T>
	class LibEvReqPair
	{
	public:
		LibEvReqPair(T _w = NULL, int _op = 0, Callback *_cb = NULL) : w(_w), op(_op), cb(_cb) { }
		LibEvReqPair(const LibEvReqPair& obj) : w(obj.w), op(obj.op), cb(obj.cb) { }
		~LibEvReqPair() { }
	public:
		T w;
		int op;
		Callback *cb;
		void *arg;
	};

	template <typename T>
	class LibEvReqQueue
	{
	public:
		LibEvReqQueue() { }
		LibEvReqQueue(const LibEvReqQueue& req) { }
		~LibEvReqQueue() { }
	public:
		tbb::concurrent_queue< LibEvReqPair<T> > requests;
		ev::async watcher;
	};

	LibEvReqQueue<ev::io*>    mIoReqQueue;
	LibEvReqQueue<ev::timer*> mTimerReqQueue;
	LibEvReqQueue<ev::async*> mAsyncReqQueue;

	bool mTerminated;
	ev::loop_ref mLoop;
};

}

#endif /*LIBEVPROCESSOR_H_*/
