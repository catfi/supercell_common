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

#ifndef COROUTINE_H_
#define COROUTINE_H_

#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

namespace zillians {

class coroutine
{
public:
	coroutine() :
		value_(0)
	{
	}
private:
	friend class coroutine_ref;
	int value_;
};

class coroutine_ref
{
public:
	coroutine_ref(coroutine& c) :
		value_(c.value_)
	{
	}
	coroutine_ref(coroutine* c) :
		value_(c->value_)
	{
	}
	operator int() const
	{
		return value_;
	}
	int operator=(int v)
	{
		return value_ = v;
	}
private:
	int& value_;
};

#define coro_reenter(c) \
  switch (coroutine_ref _coro_value = c)

#define coro_entry \
  extern void you_forgot_to_add_the_entry_label(); \
  bail_out_of_coroutine: break; \
  case 0

#define coro_yield \
  if ((_coro_value = __LINE__) == 0) \
  { \
    case __LINE__: ; \
    (void)&you_forgot_to_add_the_entry_label; \
  } \
  else \
    for (bool _coro_bool = false;; \
         _coro_bool = !_coro_bool) \
      if (_coro_bool) \
        goto bail_out_of_coroutine; \
      else

}

#endif /* COROUTINE_H_ */
