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

#ifndef ZILLIANS_NET_RDMA_POLLER_H_
#define ZILLIANS_NET_RDMA_POLLER_H_

#include "core-api/Prerequisite.h"
#include "tbb/concurrent_queue.h"
#include "boost/function.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h>

#define EV_MULTIPLICITY 1
#define EV_MAXPRI 0
#define EV_MINPRI 0
#include "net-api/rdma/ev/ev++.h"

namespace zillians { namespace net { namespace rdma {

/**
 * \brief Poller uses libev to poll for various kinds of events in a very efficient way
 */
class Poller
{
public:
	Poller(const ev::loop_ref &loop);
	virtual ~Poller();

public:
	void start  (ev::io *w, FastCallback* cb = NULL);
	void stop   (ev::io *w, FastCallback* cb = NULL);
	void restart(ev::io *w, FastCallback* cb = NULL);

	void start  (ev::async *w, FastCallback* cb = NULL);
	void stop   (ev::async *w, FastCallback* cb = NULL);

	void start  (ev::timer *w, FastCallback* cb = NULL);
	void stop   (ev::timer *w, FastCallback* cb = NULL);
	void restart(ev::timer *w, FastCallback* cb = NULL);

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
	};

	template< class T >
	class PollerReqPair
	{
	public:
		PollerReqPair(T _w = NULL, int _op = 0, FastCallback *_cb = NULL) : w(_w), op(_op), cb(_cb) { }
		PollerReqPair(const PollerReqPair& obj) : w(obj.w), op(obj.op), cb(obj.cb) { }
		~PollerReqPair() { }
	public:
		T w;
		int op;
		FastCallback *cb;
		void *arg;
	};

	template< class T >
	class PollerReqQueue : public boost::noncopyable
	{
	public:
		PollerReqQueue() { }
		~PollerReqQueue() { }
	public:
		tbb::concurrent_queue< PollerReqPair<T> > requests;
		ev::async watcher;
	};

	PollerReqQueue<ev::io*>    mIoReqQueue;
	PollerReqQueue<ev::timer*> mTimerReqQueue;
	PollerReqQueue<ev::async*> mAsyncReqQueue;

	bool mTerminated;
	ev::loop_ref mLoop;
};

} } }

#endif/*ZILLIANS_NET_RDMA_POLLER_H_*/
