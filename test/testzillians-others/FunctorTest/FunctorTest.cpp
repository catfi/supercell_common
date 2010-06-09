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

#include <stdlib.h>
//#include "core/Types.h"
#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "tbb/tick_count.h"
#include "FastDelegate.h"

using namespace fastdelegate;

class Message
{
public:
	Message()
	{
		type = 0;
	}

public:
	int type;
};

class MyNaiveHandler
{
public:
	__attribute__((noinline)) static void handleStatic(Message* message)
	{
		message->type += 2;
	}

	__attribute__((noinline)) void handleDynamic(Message* message)
	{
		message->type += 4;
	}
};

class VirtualHandler
{
public:
	virtual void handle(Message* message) = 0;
};

class MyVirtualHandler1 : public VirtualHandler
{
public:
	virtual void handle(Message* message)
	{
		message->type += 2;
	}
};

class MyVirtualHandler2 : public VirtualHandler
{
public:
	virtual void handle(Message* message)
	{
		message->type += 4;
	}
};

class MyVirtualHandler3 : public VirtualHandler
{
public:
	virtual void handle(Message* message)
	{
		message->type += 6;
	}
};

class MyVirtualHandler4 : public VirtualHandler
{
public:
	virtual void handle(Message* message)
	{
		message->type += 8;
	}
};

class MyVirtualHandler5 : public VirtualHandler
{
public:
	virtual void handle(Message* message)
	{
		message->type += 10;
	}
};

class MyVirtualHandler6 : public VirtualHandler
{
public:
	virtual void handle(Message* message)
	{
		message->type += 12;
	}
};

class MyVirtualHandler7 : public VirtualHandler
{
public:
	virtual void handle(Message* message)
	{
		message->type += 14;
	}
};

class MyVirtualHandler8 : public VirtualHandler
{
public:
	virtual void handle(Message* message)
	{
		message->type += 16;
	}
};

class TemplateHandler
{
public:
	TemplateHandler() { clear(); }
	TemplateHandler(const TemplateHandler &obj)
	{
		mCallback = obj.mCallback;
		mReference = obj.mReference;
	}
	virtual ~TemplateHandler() { }

public:
	template <class T, void (T::*method)(Message*)>
	inline void bind(T* obj)
	{
		mReference = (void*)obj;
		mCallback = method_body<T, method>;
	}

	template <class T, void (T::*method)(Message*)>
	inline static void method_body(TemplateHandler* ref, Message* message)
	{
		T* obj = static_cast<T*>(ref->mReference);
		(obj->*method)(message);
	}

public:
	template <class T, void (T::*method)(Message*) const>
	inline void bind(T* obj)
	{
		mReference = (void*)obj;
		mCallback = const_method_body<T, method>;
	}

	template <class T, void (T::*method)(Message*) const>
	inline static void const_method_body(TemplateHandler* ref, Message* message)
	{
		T* obj = static_cast<T*>(ref->mReference);
		(obj->*method)(message);
	}

public:
	inline void invoke(Message *message)
	{
		if(mCallback) mCallback(this, message);
	}

	inline void clear()
	{
		mCallback = NULL; mReference = NULL;
	}

private:
	void (*mCallback)(TemplateHandler*, Message*);
	void *mReference;
};

class TemplateHandler1
{
public:
	inline void handle(Message* message)
	{
		message->type += 2;
	}
};

class TemplateHandler2
{
public:
	inline void handle(Message* message)
	{
		message->type += 4;
	}
};

class TemplateHandler3
{
public:
	inline void handle(Message* message)
	{
		message->type += 6;
	}
};

class TemplateHandler4
{
public:
	inline void handle(Message* message)
	{
		message->type += 8;
	}
};

class TemplateHandler5
{
public:
	inline void handle(Message* message)
	{
		message->type += 10;
	}
};

class TemplateHandler6
{
public:
	inline void handle(Message* message)
	{
		message->type += 12;
	}
};

class TemplateHandler7
{
public:
	inline void handle(Message* message)
	{
		message->type += 14;
	}
};

class TemplateHandler8
{
public:
	inline void handle(Message* message)
	{
		message->type += 16;
	}
};

typedef FastDelegate1<Message*> MyFastDelegate;

int main(int argc, char** argv)
{
	int count = 100000000;
	if(argc>1) count = atoi(argv[1]);

	tbb::tick_count start, end;

	// test direct static call
	if(true)
	{
		Message msg;
		start = tbb::tick_count::now();
		for(int i=0;i<count;++i)
		{
			MyNaiveHandler::handleStatic(&msg);
		}
		end = tbb::tick_count::now();
		tbb::tick_count::interval_t gap = end - start;
		printf("Calling %d static function directly cost %f ms (dummy=%d)\n", count, gap.seconds() * 1000.0, msg.type);
	}

	// test direct member function call
	if(true)
	{
		Message msg;
		MyNaiveHandler c;
		start = tbb::tick_count::now();
		for(int i=0;i<count;++i)
		{
			c.handleDynamic(&msg);
		}
		end = tbb::tick_count::now();
		tbb::tick_count::interval_t gap = end - start;
		printf("Calling %d member function directly cost %f ms (dummy=%d)\n", count, gap.seconds() * 1000.0, msg.type);
	}

	// test boost static functor
	if(true)
	{
		Message msg;
		start = tbb::tick_count::now();
		boost::function<void(Message*)> f = &MyNaiveHandler::handleStatic;
		for(int i=0;i<count;++i)
		{
			boost::function<void(Message*)> *g = &f;
			(*g)(&msg);
		}
		end = tbb::tick_count::now();
		tbb::tick_count::interval_t gap = end - start;
		printf("Calling %d static boost::function cost %f ms\n", count, gap.seconds() * 1000.0);
	}

	// test boost dynamic functor
	if(true)
	{
		Message msg;
		MyNaiveHandler c;
		start = tbb::tick_count::now();
		boost::function<void(MyNaiveHandler*, Message*)> f = &MyNaiveHandler::handleDynamic;
		for(int i=0;i<count;++i)
		{
			boost::function<void(MyNaiveHandler*, Message*)> *g = &f;
			(*g)(&c, &msg);
		}
		end = tbb::tick_count::now();
		tbb::tick_count::interval_t gap = end - start;
		printf("Calling %d dynamic boost::function cost %f ms\n", count, gap.seconds() * 1000.0);
	}

	// test virtual function wrapper
	if(true)
	{
		Message msg;
		VirtualHandler *vhandler = NULL;
		if(true)
		{
			// try to make it un-predictable so that optimization is impossible
			srand(time(0));
			int selection = rand() % 8;
			switch(selection)
			{
			case 0: vhandler = new MyVirtualHandler1(); break;
			case 1: vhandler = new MyVirtualHandler2(); break;
			case 2: vhandler = new MyVirtualHandler3(); break;
			case 3: vhandler = new MyVirtualHandler4(); break;
			case 4: vhandler = new MyVirtualHandler5(); break;
			case 5: vhandler = new MyVirtualHandler6(); break;
			case 6: vhandler = new MyVirtualHandler7(); break;
			case 7: vhandler = new MyVirtualHandler8(); break;
			}
		}
		start = tbb::tick_count::now();
		for(int i=0;i<count;++i)
		{
			vhandler->handle(&msg);
		}
		end = tbb::tick_count::now();
		tbb::tick_count::interval_t gap = end - start;
		printf("Calling %d virtual function wrapper cost %f ms\n", count, gap.seconds() * 1000.0);
		//SAFE_DELETE(vhandler);
		delete vhandler;
	}

	// test templated functor wrapper
	if(true)
	{
		Message msg;
		TemplateHandler1 t1;
		TemplateHandler2 t2;
		TemplateHandler3 t3;
		TemplateHandler4 t4;
		TemplateHandler5 t5;
		TemplateHandler6 t6;
		TemplateHandler7 t7;
		TemplateHandler8 t8;
		TemplateHandler *thandler = new TemplateHandler();
		if(true)
		{
			// try to make it un-predictable so that optimization is impossible
			srand(time(0));
			int selection = rand() % 8;
			switch(selection)
			{
			case 0: thandler->bind<TemplateHandler1, &TemplateHandler1::handle>(&t1); break;
			case 1: thandler->bind<TemplateHandler2, &TemplateHandler2::handle>(&t2); break;
			case 2: thandler->bind<TemplateHandler3, &TemplateHandler3::handle>(&t3); break;
			case 3: thandler->bind<TemplateHandler4, &TemplateHandler4::handle>(&t4); break;
			case 4: thandler->bind<TemplateHandler5, &TemplateHandler5::handle>(&t5); break;
			case 5: thandler->bind<TemplateHandler6, &TemplateHandler6::handle>(&t6); break;
			case 6: thandler->bind<TemplateHandler7, &TemplateHandler7::handle>(&t7); break;
			case 7: thandler->bind<TemplateHandler8, &TemplateHandler8::handle>(&t8); break;
			}
		}
		start = tbb::tick_count::now();
		for(int i=0;i<count;++i)
		{
			thandler->invoke(&msg);
		}
		end = tbb::tick_count::now();
		tbb::tick_count::interval_t gap = end - start;
		printf("Calling %d templated function wrapper cost %f ms\n", count, gap.seconds() * 1000.0);
		//SAFE_DELETE(thandler);
		delete thandler;
	}

	if(true)
	{
		Message msg;
		TemplateHandler1 t1;
		TemplateHandler2 t2;
		TemplateHandler3 t3;
		TemplateHandler4 t4;
		TemplateHandler5 t5;
		TemplateHandler6 t6;
		TemplateHandler7 t7;
		TemplateHandler8 t8;
		MyFastDelegate thandler;
		if(true)
		{
			// try to make it un-predictable so that optimization is impossible
			srand(time(0));
			int selection = rand() % 8;
			switch(selection)
			{
			case 0: thandler.bind(&t1, &TemplateHandler1::handle); break;
			case 1: thandler.bind(&t2, &TemplateHandler2::handle); break;
			case 2: thandler.bind(&t3, &TemplateHandler3::handle); break;
			case 3: thandler.bind(&t4, &TemplateHandler4::handle); break;
			case 4: thandler.bind(&t5, &TemplateHandler5::handle); break;
			case 5: thandler.bind(&t6, &TemplateHandler6::handle); break;
			case 6: thandler.bind(&t7, &TemplateHandler7::handle); break;
			case 7: thandler.bind(&t8, &TemplateHandler8::handle); break;
			}
		}
		start = tbb::tick_count::now();
		for(int i=0;i<count;++i)
		{
			thandler(&msg);
		}
		end = tbb::tick_count::now();
		tbb::tick_count::interval_t gap = end - start;
		printf("Calling %d fast delegate function wrapper cost %f ms\n", count, gap.seconds() * 1000.0);
	}

	return 0;
}
