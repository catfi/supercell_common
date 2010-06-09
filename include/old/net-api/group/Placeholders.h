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
 * @date Oct 6, 2009 sdk - Initial version created.
 */


#ifndef ZILLIANS_NET_GROUP_PLACEHOLDERS_H_
#define ZILLIANS_NET_GROUP_PLACEHOLDERS_H_

namespace zillians { namespace net { namespace group { namespace placeholders {

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

namespace data {

#if defined(GENERATING_DOCUMENTATION)

unspecified source_ref;

unspecified type;

unspecified message_ref;

unspecified buffer_ref;

unspecified size;

#elif defined(__BORLANDC__) || defined(__GNUC__)

inline boost::arg<1> source_ref()
{
  return boost::arg<1>();
}

inline boost::arg<2> type()
{
  return boost::arg<2>();
}

inline boost::arg<2> message_ref()
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

#else

#if BOOST_WORKAROUND(BOOST_MSVC, < 1400)

static boost::arg<1>& source_ref = zillians::networking::group::placeholders::detail::placeholder<1>::get();
static boost::arg<2>& type = zillians::networking::group::placeholders::detail::placeholder<2>::get();
static boost::arg<2>& message_ref = zillians::networking::group::placeholders::detail::placeholder<2>::get();
static boost::arg<3>& buffer_ref = zillians::networking::group::placeholders::detail::placeholder<3>::get();
static boost::arg<4>& size = zillians::networking::group::placeholders::detail::placeholder<4>::get();

#else

namespace
{
	boost::arg<1>& source_ref = zillians::networking::group::placeholders::detail::placeholder<1>::get();
	boost::arg<2>& type = zillians::networking::group::placeholders::detail::placeholder<2>::get();
	boost::arg<2>& message_ref = zillians::networking::group::placeholders::detail::placeholder<2>::get();
	boost::arg<3>& buffer_ref = zillians::networking::group::placeholders::detail::placeholder<3>::get();
	boost::arg<4>& size = zillians::networking::group::placeholders::detail::placeholder<4>::get();
} // namespace

#endif

#endif

}

namespace membership {

#if defined(GENERATING_DOCUMENTATION)

unspecified group_name;

unspecified source_info;

unspecified reason;

#elif defined(__BORLANDC__) || defined(__GNUC__)

inline boost::arg<1> group_name()
{
  return boost::arg<1>();
}

inline boost::arg<2> source_info()
{
  return boost::arg<2>();
}

inline boost::arg<3> reason()
{
  return boost::arg<3>();
}

#else

#if BOOST_WORKAROUND(BOOST_MSVC, < 1400)

static boost::arg<1>& group_name = zillians::networking::group::placeholders::detail::placeholder<1>::get();
static boost::arg<2>& source_info = zillians::networking::group::placeholders::detail::placeholder<2>::get();
static boost::arg<3>& reason = zillians::networking::group::placeholders::detail::placeholder<3>::get();

#else

namespace
{
	boost::arg<1>& group_name = zillians::networking::group::placeholders::detail::placeholder<1>::get();
	boost::arg<2>& source_info = zillians::networking::group::placeholders::detail::placeholder<2>::get();
	boost::arg<3>& reason = zillians::networking::group::placeholders::detail::placeholder<3>:get();
} // namespace

#endif

#endif

}

} } } }

#endif/*ZILLIANS_NET_GROUP_PLACEHOLDERS_H_*/
