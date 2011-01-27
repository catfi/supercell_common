/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
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
 * @date Jan 10, 2011 sdk - Initial version created.
 */

/**
 * This code is copied from Chris Knohlohff who is the author of boost asio.
 * Basically he implements stackless coroutine on top of asio. This can be very useful
 * for implementing application with lots asynchronous operations.
 *
 * We modify the naming convention and the macro name to meet our coding standard.
 *
 * @ref http://blog.think-async.com/2009/07/wife-says-i-cant-believe-it-works.html
 * @ref http://blog.think-async.com/2009/08/composed-operations-coroutines-and-code.html
 * @ref http://blog.think-async.com/2009/08/secret-sauce-revealed.html
 */

#ifndef ZILLIANS_COROUTINE_H_
#define ZILLIANS_COROUTINE_H_

#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

namespace zillians {

class Coroutine
{
public:
	Coroutine() : mInternalCoroutineState(0)
	{ }

public:
	/**
	 * reset the coroutine state to initial value (so the coroutine state is re-initialized for new asynchronous transaction)
	 *
	 * @note CAUTION!! DANGEROUS!! ZAC ONLY!! Unless you understand coroutine completely, DO NOT TRY IT without supervision!
	 */
	void reset() { mInternalCoroutineState = 0; }

private:
	friend class CoroutineRef;
	int mInternalCoroutineState;
};

class CoroutineRef
{
public:
	CoroutineRef(Coroutine& c) :
		mInternalCoroutineState(c.mInternalCoroutineState)
	{
	}
	CoroutineRef(Coroutine* c) :
		mInternalCoroutineState(c->mInternalCoroutineState)
	{
	}
	operator int() const
	{
		return mInternalCoroutineState;
	}
	int operator= (int state)
	{
		return mInternalCoroutineState = state;
	}

public:
	/**
	 * reset the coroutine state to initial value (so the coroutine state is re-initialized for new asynchronous transaction)
	 *
	 * @note CAUTION!! DANGEROUS!! ZAC ONLY!! Unless you understand coroutine completely, DO NOT TRY IT without supervision!
	 */
	void reset() { mInternalCoroutineState = 0; }

protected:
	int& mInternalCoroutineState;
};

#define CoroutineReenter(c) \
  switch (CoroutineRef _coro_value = c)

#define CoroutineEntry \
  extern void you_forgot_to_add_the_entry_label(); \
  bail_out_of_coroutine: break; \
  case 0

#define CoroutineYield \
  if ((_coro_value = __LINE__) == 0) \
  { \
    case __LINE__: ; \
    (void)&you_forgot_to_add_the_entry_label; \
  } \
  else \
    for (bool _coro_bool = false;; _coro_bool = !_coro_bool) \
      if (_coro_bool) \
        goto bail_out_of_coroutine; \
      else

}

#endif /* COROUTINE_H_ */
