/**
 * Zillians MMO
 * Copyright (C) 2007-2011 Zillians.com, Inc.
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

#ifndef ZILLIANS_CONTEXTHUBSERIALIZATION_H_
#define ZILLIANS_CONTEXTHUBSERIALIZATION_H_

#include "core/ContextHub.h"
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/vector.hpp>

namespace zillians {

struct ContextHubSerializationBase
{
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	UNUSED_ARGUMENT(ar);
    	UNUSED_ARGUMENT(version);

    	// dummy serialization
    }
};

template<int N, typename Types>
struct ContextHubSerializationImpl : public ContextHubSerializationImpl<N-1, Types>
{
	ContextHubSerializationImpl(ContextHub<ContextOwnership::transfer>& h) : ContextHubSerializationImpl<N-1,Types>(h), hub(h)
	{ }

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const
    {
    	UNUSED_ARGUMENT(version);

    	typedef typename boost::mpl::at<Types, boost::mpl::int_<N - 1> >::type T;
    	ar & boost::serialization::base_object<ContextHubSerializationImpl<N-1,Types>>(*this);

    	T* t = hub.get<T>();
    	ar & t;
    }

    template<class Archive>
    void load(Archive& ar, const unsigned int version)
    {
    	UNUSED_ARGUMENT(version);

    	typedef typename boost::mpl::at<Types, boost::mpl::int_<N - 1> >::type T;
    	ar & boost::serialization::base_object<ContextHubSerializationImpl<N-1,Types>>(*this);

    	T* t = NULL;
    	ar & t;
    	hub.set<T>(t);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    ContextHub<ContextOwnership::transfer>& hub;
};

template<typename Types>
struct ContextHubSerializationImpl<0, Types> : public ContextHubSerializationBase
{
	ContextHubSerializationImpl(ContextHub<ContextOwnership::transfer>& h) : hub(h)
	{ }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	UNUSED_ARGUMENT(version);

    	ar & boost::serialization::base_object<ContextHubSerializationBase>(*this);
    }

    ContextHub<ContextOwnership::transfer>& hub;
};

template<typename Types>
struct ContextHubSerialization
{
	ContextHubSerialization(ContextHub<ContextOwnership::transfer>& h) : hub(h), t(h)
	{ }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	UNUSED_ARGUMENT(version);

    	ar & t;
    }

    ContextHub<ContextOwnership::transfer>& hub;
    ContextHubSerializationImpl<boost::mpl::size<Types>::value, Types> t;
};

}

//BOOST_CLASS_EXPORT_GUID(zillians::ContextHubSerializationBase, "ContextHubSerializationBase")

#endif /* ZILLIANS_CONTEXTHUBSERIALIZATION_H_ */
