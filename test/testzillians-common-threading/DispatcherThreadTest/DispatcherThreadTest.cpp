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


#include "core/Prerequisite.h"
#include "threading/Dispatcher.h"
#include "threading/DispatcherThreadContext.h"
#include "threading/DispatcherDestination.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

#define ITERATIONS	1024
#define READER_WRITER_THREAD_COUNT	32

using namespace zillians;
using namespace zillians::threading;
using std::cout;
using std::endl;

log4cxx::LoggerPtr mLogger(log4cxx::Logger::getLogger("DispatcherThreadTest"));

namespace {

struct Message
{
	int count;
};

}

void writer_thread(shared_ptr<DispatcherThreadContext<Message> > dt, uint32 destination)
{
	shared_ptr<DispatcherDestination<Message> > dest = dt->createDestination(destination);

	Message m;

	for(int i=0;i<ITERATIONS;++i)
	{
//		cout << "writer iteration: " << i << endl;
		m.count = i;
		dest->write(m);
	}
}

void reader_thread(shared_ptr<DispatcherThreadContext<Message> > dt, uint32 source, bool blocking)
{
	Message m[ITERATIONS];
	uint32  s[ITERATIONS];
	uint32 k = 0;
	uint32 n = ITERATIONS;
	uint32 totalMsg = 0;
	uint32 i = 0;
	while(totalMsg < ITERATIONS)
	{
//		cout << "reader iteration: " << i << endl;

		n = ITERATIONS;
		if(dt->read(s, m, n, blocking)) 
		{
			totalMsg += n;
			for(int j = 0; j < n; ++j)
			{
				BOOST_ASSERT(m[j].count == k);
				BOOST_ASSERT(s[j] == source);
				++k;
			}
		}
		++i;
	}
}

void writer_reader_thread(shared_ptr<DispatcherThreadContext<Message> > dt, std::vector<uint32> destinations)
{
	Message m; m.count = 1;
	for(std::vector<uint32>::iterator it = destinations.begin(); it != destinations.end(); ++it)
	{
		shared_ptr<DispatcherDestination<Message> > dest = dt->createDestination(*it);
		for(int i=0;i<ITERATIONS;++i)
		{
			dest->write(m);
		}
	}

	for(int i=0;i<destinations.size() * ITERATIONS;++i)
	{
		for(uint32 i=0;i<ITERATIONS;++i)
		{
			uint32 source;
			while(dt->read(source, m));

			BOOST_ASSERT(m.count == 1);
		}
	}
}

int main (int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure();

	Dispatcher<Message> dispatcher(63);

	for(int i=0;i<10;++i)
	{
		shared_ptr<DispatcherThreadContext<Message> > reader_dt = dispatcher.createThreadContext();
		shared_ptr<DispatcherThreadContext<Message> > writer_dt = dispatcher.createThreadContext();

		boost::thread t0(boost::bind(reader_thread, reader_dt, writer_dt->getIdentity(), false));
		boost::thread t1(boost::bind(writer_thread, writer_dt, reader_dt->getIdentity()));

		t0.join();
		t1.join();

		cout << "iteration: " << i+1 << " of 10, non-blocking reading ok" << endl;
	}
	
	for(int i=0;i<10;++i)
	{
		shared_ptr<DispatcherThreadContext<Message> > reader_dt = dispatcher.createThreadContext();
		shared_ptr<DispatcherThreadContext<Message> > writer_dt = dispatcher.createThreadContext();

		boost::thread t0(boost::bind(reader_thread, reader_dt, writer_dt->getIdentity(), true));
		boost::thread t1(boost::bind(writer_thread, writer_dt, reader_dt->getIdentity()));

		t0.join();
		t1.join();

		cout << "iteration: " << i+1 << " of 10, blocking reading ok" << endl;
	}

	for(int i=0;i<10;++i)
	{
		cout << "iteration: " << i+1 << " of 10" << endl;
		std::vector<uint32> destinations;
		std::vector<shared_ptr<DispatcherThreadContext<Message> > > dts;
		for(int i=0;i<READER_WRITER_THREAD_COUNT;++i)
		{
			shared_ptr<DispatcherThreadContext<Message> > dt = dispatcher.createThreadContext();
			destinations.push_back(dt->getIdentity());
			dts.push_back(dt);
		}

		boost::thread** wr_threads = new boost::thread*[READER_WRITER_THREAD_COUNT];

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
