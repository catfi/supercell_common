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
 * @date Jul 14, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_SYS_DISPATCHER_H_
#define ZILLIANS_NET_SYS_DISPATCHER_H_

#include "core/Prerequisite.h"

/**
 * @note If HANDLE_MESSAGE_TYPE_OVERFLOW_AS_EXCEPTION is set to 1 (true), any buffer
 * completion whose type is greater than maximum allowed type will result in
 * out-of-range exception; otherwise, the dispatcher will try to dispatch using
 * any-dispatch implementation.
 */
#define _HANDLE_MESSAGE_TYPE_OVERFLOW_AS_EXCEPTION	1

namespace zillians { namespace net { namespace sys {

/**
 * DisaptchT is a universal interface to implement dynamic dispatch.
 */
template <typename Source>
struct DispatchT
{
	virtual void dispatch(Source& source, uint32 type, shared_ptr<Buffer>& b, std::size_t size) = 0;
};

/**
 * BufferDispatch is one type of dispatch implementation to dispatch a
 * buffer completion to user-provided handler.
 */
template <typename Source>
struct BufferDispatchT : public DispatchT<Source>
{
	typedef typename boost::function< void (Source&, uint32, shared_ptr<Buffer>&, std::size_t) > handler_type;

	BufferDispatchT(handler_type h) : handler(h) { }

	virtual void dispatch(Source& source, uint32 type, shared_ptr<Buffer>& b, std::size_t size)
	{
		handler(source, type, b, size);
	}
	handler_type handler;
};

/**
 * MessageDispatchT is one type of dispatch implementation to re-dispatch
 * a buffer completion into a message handler.
 */
template <typename Source, typename M>
struct MessageDispatchT : public DispatchT<Source>
{
	typedef typename boost::function< void (Source&, M& m) > handler_type;

	MessageDispatchT(handler_type handler) : handler_(handler) { }

	virtual void dispatch(Source& source, uint32 type, shared_ptr<Buffer>& b, std::size_t size)
	{
		BOOST_ASSERT(type == M::TYPE);
		M message;

		(*b.get()) >> message;

		handler_(source, message);
	}
	handler_type handler_;
};

/**
 * The DispatcherT template is a generic buffer dispatching class template
 * to implement type-based dispatch.
 *
 * @note The DispatcherT template is NOT thread-safe, that is, you cannot
 * bind or unbind any dispatch object while calling dispatch(), or call
 * bindBuffer() or bindMessage() from different thread simultaneously.
 */
template <typename Source>
class DispatcherT
{
public:
	/**
	 * The DispatcherT default constructor takes a maximum completion type value
	 *
	 * @param maxType
	 */
	DispatcherT(uint32 maxType = 256) : mMaxDispatchType(maxType)
	{
		mDispatchByType = new DispatchT<Source>*[mMaxDispatchType];
		for(uint32 i = 0UL; i < mMaxDispatchType;++i)
		{
			mDispatchByType[i] = NULL;
		}

		mDispatchAny = NULL;
	}

	~DispatcherT()
	{
		SAFE_DELETE(mDispatchAny);

		for(uint32 i = 0UL; i < mMaxDispatchType; ++i)
		{
			if(mDispatchByType[i])
			{
				SAFE_DELETE(mDispatchByType[i]);
			}
		}

		SAFE_DELETE_ARRAY(mDispatchByType);
	}

public:
	/**
	 * Bind an universal handler to handle any message whose type is not
	 * bound to any handler.
	 *
	 * @note the ownership of the given dispatch implementation object is transferred
	 * to this dispatcher, which is be automatically cleaned up (by deleting it)
	 *
	 * @param h The handler functor with function prototype
	 * @code
	 * void handler(Source&, uint32, shared_ptr<Buffer>&, std::size_t);
	 * @endcode
	 */
	void bindAny(typename BufferDispatchT<Source>::handler_type h)
	{
		SAFE_DELETE(mDispatchAny);

		mDispatchAny = new BufferDispatchT<Source>(h);
	}

	/**
	 * Unbind the universal handler
	 *
	 * @note the ownership of the given dispatch implementation object is transferred
	 * to this dispatcher, which is be automatically cleaned up (by deleting it)
	 */
	void unbindAny()
	{
		SAFE_DELETE(mDispatchAny);
	}

	/**
	 * Bind a direct, user-provided dispatch implementation to the dispacther
	 *
	 * @note the ownership of the given dispatch implementation object is transferred
	 * to this dispatcher, which is be automatically cleaned up (by deleting it)
	 *
	 * @param type
	 * @param handler
	 */
	void bind(uint32 type, DispatchT<Source>* handler)
	{
		if(mDispatchByType[type])
		{
			SAFE_DELETE(mDispatchByType[type]);
		}

		mDispatchByType[type] = handler;
	}

	/**
	 * Bind a specific type of buffer completion to a buffer completion handler.
	 *
	 * @note the ownership of the given dispatch implementation object is transferred
	 * to this dispatcher, which is be automatically cleaned up (by deleting it)
	 *
	 * @param type The specific buffer type
	 * @param h the Buffer completion handler
	 */
	void bind(uint32 type, typename BufferDispatchT<Source>::handler_type h)
	{
		if(UNLIKELY(type >= mMaxDispatchType))
		{
			throw std::invalid_argument("given message type is greater than the maximal allowed type in the dispatcher");
		}

		if(mDispatchByType[type])
		{
			SAFE_DELETE(mDispatchByType[type]);
		}

		mDispatchByType[type] = new BufferDispatchT<Source>(h);
	}

	/**
	 * Bind a specific type of message to a message completion handler.
	 *
	 * @note the ownership of the given dispatch implementation object is transferred
	 * to this dispatcher, which is be automatically cleaned up (by deleting it)
	 *
	 * @note Note that although this is a templated method to accept any
	 * type of message, there're some concept requirements for the message
	 * to implement.
	 * The message M must implement the following concept:
	 * @code
	 * 		class M
	 * 		{
	 * 			// either declare the TYPE using enumeration
	 * 			enum { TYPE = xxx };
	 *
	 * 			// or using static const variables
	 * 			static const uint32 TYPE = xxx;
	 *
	 * 			// and finally must implement the serialize() templated method
	 * 			template <typename Archive>
	 * 			void serialize(Archive& ar, const unsigned int version)
	 * 			{
	 * 				ar & result;
	 * 			}
	 * 		};
	 * @endcode
	 *
	 * @param h The message completion handler.
	 */
	template <typename M>
	void bind(typename MessageDispatchT<Source,M>::handler_type h)
	{
		if(UNLIKELY(M::TYPE >= mMaxDispatchType))
		{
			throw std::invalid_argument("given message type is greater than the maximal allowed type in the dispatcher");
		}

		if(mDispatchByType[M::TYPE])
		{
			SAFE_DELETE(mDispatchByType[M::TYPE]);
		}

		mDispatchByType[M::TYPE] = new MessageDispatchT<Source,M>(h);
	}

	/**
	 * Unbind a message dispatch
	 *
	 * @param type The type of buffer completion
	 */
	template <typename M>
	void unbind()
	{
		unbind(M::TYPE);
	}

	/**
	 * Unbind a specific type of buffer dispatch
	 *
	 * @param type The type of buffer completion
	 */
	void unbind(uint32 type)
	{
		SAFE_DELETE(mDispatchByType[type]);
	}

	/**
	 * Determine whether a type of message has been bound to any dispatch implementation
	 */
	template <typename M>
	void isBound()
	{
		return isBound(M::TYPE);
	}

	/**
	 * Determine whether a type has been bound to any dispatch implementation
	 *
	 * @param type The buffer completion type
	 * @return true if it's bound, false otherwise
	 */
	bool isBound(uint32 type)
	{
		return (mDispatchByType[type]) ? true : false;
	}

	/**
	 * Dispatch a buffer completion to pre-registered handlers.
	 *
	 * This function will dispatch the buffer completion and invoke
	 * pre-registered handler through various dispatch implementation
	 * (like BufferDispatchT<> and MessageDispatchT<>) in the same
	 * execution context, so there's explicit context switch involved
	 * in this function.
	 *
	 * If a buffer completion whose type is beyond the maximum allowed
	 * type for this dispatcher, it will either throw an out-of-range
	 * exception if HANDLE_MESSAGE_TYPE_OVERFLOW_AS_EXCEPTION is true,
	 * or try to dispatch to the the any-handler otherwise.
	 *
	 * When a type that is not pre-registered to any user handler, and
	 * the "any" handler is not registered either, it will throw an
	 * invalid argument exception.
	 *
	 * @throws std::invalid_argument Thrown when the buffer completion type is not registered
	 * @throws std::out_of_range Thrown when the buffer completion type is greater than maximum allowed dispatch type
	 *
	 * @param type The type of buffer completion.
	 * @param source The referenced source object.
	 * @param b The completed buffer wrapped as a shared pointer.
	 * @param size The size of the completion (the additional data that has been read into the completed buffer).
	 */
	void dispatch(uint32 type, Source& source, shared_ptr<Buffer>& b, std::size_t size)
	{
#if _HANDLE_MESSAGE_TYPE_OVERFLOW_AS_EXCEPTION
		if(UNLIKELY(type >= mMaxDispatchType))
		{
			throw std::out_of_range("given message type is greater than the maximal allowed type in the dispatcher");
		}
#endif

		if(UNLIKELY(!mDispatchByType[type]))
		{
			if(UNLIKELY(!mDispatchAny))
			{
				throw std::invalid_argument("given message type is not registered");
			}
			else
			{
				mDispatchAny->dispatch(source, type, b, size);
			}
		}
		else
		{
			mDispatchByType[type]->dispatch(source, type, b, size);
		}
	}

private:
	/// The maximum buffer completion type
	const uint32 mMaxDispatchType;

	/// The dispatch table for specific buffer completion type
	DispatchT<Source>** mDispatchByType;

	/// The dispatch implementation for any not-registered buffer completion type
	DispatchT<Source>*  mDispatchAny;
};

} } }

#endif/*ZILLIANS_NET_SYS_DISPATCHER_H_*/
