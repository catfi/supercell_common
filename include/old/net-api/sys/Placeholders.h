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
 * @date Jul 17, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_SYS_PLACEHOLDERS_H_
#define ZILLIANS_NET_SYS_PLACEHOLDERS_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind/arg.hpp>
#include <boost/detail/workaround.hpp>

namespace zillians { namespace net { namespace sys { namespace placeholders {

namespace detail {
	template <int Number>
	struct placeholder
	{
		static boost::arg<Number>& get()
		{
			static boost::arg<Number> result;
			return result;
		}
	};
}

#if defined(GENERATING_DOCUMENTATION)

unspecified error;

unspecified byte_transferred;

#elif defined(__BORLANDC__) || defined(__GNUC__)

inline boost::arg<1> error()
{
  return boost::arg<1>();
}

inline boost::arg<2> byte_transferred()
{
  return boost::arg<2>();
}

#else

#if BOOST_WORKAROUND(BOOST_MSVC, < 1400)

static boost::arg<1>& error = zillians::networking::sys::placeholders::detail::placeholder<1>::get();
static boost::arg<2>& byte_transferred = zillians::networking::sys::placeholders::detail::placeholder<2>::get();

#else

namespace
{
	boost::arg<1>& error = zillians::networking::sys::placeholders::detail::placeholder<1>::get();
	boost::arg<2>& byte_transferred = zillians::networking::sys::placeholders::detail::placeholder<2>::get();
} // namespace

#endif

#endif

namespace dispatch {

#if defined(GENERATING_DOCUMENTATION)

unspecified source_ref;

unspecified message_ref;

unspecified type;

unspecified buffer_ref;

unspecified size;

unspecified error;

#elif defined(__BORLANDC__) || defined(__GNUC__)

inline boost::arg<1> source_ref()
{
  return boost::arg<1>();
}


inline boost::arg<2> message_ref()
{
  return boost::arg<2>();
}


inline boost::arg<2> type()
{
  return boost::arg<2>();
}

inline boost::arg<3> buffer_ref()
{
  return boost::arg<3>();
}

inline boost::arg<4> size()
{
  return boost::arg<4>();
}


inline boost::arg<2> error()
{
  return boost::arg<2>();
}

#else

#if BOOST_WORKAROUND(BOOST_MSVC, < 1400)

static boost::arg<1>& source_ref = zillians::networking::sys::placeholders::detail::placeholder<1>::get();

static boost::arg<2>& message_ref = zillians::networking::sys::placeholders::detail::placeholder<2>::get();

static boost::arg<2>& type = zillians::networking::sys::placeholders::detail::placeholder<2>::get();
static boost::arg<3>& buffer_ref = zillians::networking::sys::placeholders::detail::placeholder<3>::get();
static boost::arg<3>& size = zillians::networking::sys::placeholders::detail::placeholder<4>::get();

static boost::arg<2>& error = zillians::networking::sys::placeholders::detail::placeholder<2>::get();

#else

namespace
{
	boost::arg<1>& source_ref = zillians::networking::sys::placeholders::detail::placeholder<1>::get();

	boost::arg<2>& message_ref = zillians::networking::sys::placeholders::detail::placeholder<2>::get();

	boost::arg<2>& type = zillians::networking::sys::placeholders::detail::placeholder<2>::get();
	boost::arg<3>& buffer_ref = zillians::networking::sys::placeholders::detail::placeholder<3>::get();
	boost::arg<4>& size = zillians::networking::sys::placeholders::detail::placeholder<4>::get();

	boost::arg<2>& error = zillians::networking::sys::placeholders::detail::placeholder<2>::get();
} // namespace

#endif

#endif

}

} } } }


#endif/*ZILLIANS_NET_SYS_PLACEHOLDERS_H_*/
