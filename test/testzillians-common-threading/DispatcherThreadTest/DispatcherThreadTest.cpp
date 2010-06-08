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
 * @date Oct 7, 2009 rocet - Initial version created.
 */


#include "core-api/Prerequisite.h"
#include "threading/Dispatcher.h"
#include "threading/DispatcherThread.h"
#include "threading/DispatcherDestination.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

#define ITERATIONS	1024
#define READER_WRITER_THREAD_COUNT	32

using namespace zillians;
using namespace zillians::threading;

log4cxx::LoggerPtr mLogger(log4cxx::Logger::getLogger("DispatcherThreadTest"));

namespace {

struct Message
{
	int count;
};

}

void writer_thread(shared_ptr<DispatcherThread<Message> > dt, uint32 destination)
{
	shared_ptr<DispatcherDestination<Message> > dest = dt->createDestination(destination);

	Message m;

	for(int i=0;i<ITERATIONS;++i)
	{
		m.count = i;
		dest->write(m);
	}
}

void read_thread(shared_ptr<DispatcherThread<Message> > dt, uint32 destination)
{
	for(int i=0;i<ITERATIONS;++i)
	{
		Message m;
		while(!dt->read(&m));

		BOOST_ASSERT(m->count == i);
	}
}

void writer_reader_thread(shared_ptr<DispatcherThread<Message> > dt, std::vector<uint32> destinations)
{
	Message m; m.count = 1;
	for(std::vector<uint32>::iterator it = destination.begin(); it != destinations.end(); ++it)
	{
		shared_ptr<DispatcherDestination<Message> > dest = dt->createDestination(*it);
		for(int i=0;i<ITERATIONS;++i)
		{
			dest->write(m);
		}
	}

	for(int i=0;i<destinations.size() * ITERATIONS;++i)
	{
		for(int i=0;i<ITERATIONS;++i)
		{
			Message m;
			while(!dt->read(&m));

			BOOST_ASSERT(m->count == 1);
		}
	}
}

int main (int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure();

	Dispatcher<Message> dispatcher(64);

	if(true)
	{
		shared_ptr<DispatcherThread<Message> > reader_dt = dispatcher.createThread();
		shared_ptr<DispatcherThread<Message> > writer_dt = dispatcher.createThread();

		boost::thread t0(boost::bind(reader_thread, reader_dt, writer_dt->getIdentity()));
		boost::thread t1(boost::bind(writer_thread, writer_dt, reader_dt->getIdentity()));

		t0.join();
		t1.join();
	}

	for(int i=0;i<10;++i)
	{
		std::vector<uint32> destinations;
		std::vector<shared_ptr<DispatcherThread<Message> > > dts;
		for(int i=0;i<READER_WRITER_THREAD_COUNT;++i)
		{
			shared_ptr<DispatcherThread<Message> > dt = dispatcher.createThread();
			destinations.push_back(dt->getIdentity());
			dts.push_back(dt);
		}

		boost::thread** wr_threads = new boost::thread[READER_WRITER_THREAD_COUNT];

		for(int i=0;i<READER_WRITER_THREAD_COUNT;++i)
		{
			wr_threads[i] = new boost::thread(boost::bind(writer_reader_thread, dts[i], destinations));
		}

		for(int i=0;i<READER_WRITER_THREAD_COUNT;++i)
		{
			wr_threads[i]->join();
		}

		delete[] wr_threads;
	}

	return 0;
}
