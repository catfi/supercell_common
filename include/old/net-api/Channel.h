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
 * @date Jun 30, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_CHANNEL_H_
#define ZILLIANS_NET_CHANNEL_H_

#include "core/Prerequisite.h"

namespace zillians { namespace net {

/**
 * Channel provides an communication abstraction to a cluster node.
 */
class Channel
{
public:
	typedef boost::function< void(const boost::system::error_code&, const std::size_t&) > AsyncSendCompletionHandler;

	Channel()
	{ }

	virtual ~Channel()
	{ }

public:
	/**
	 * Synchronous Send.
	 *
	 * When application calls synchronous send, the send() will only return
	 * when the given buffer has been written to network, so the buffer is
	 * free for re-use or destruction.
	 *
	 * @param type The type of the buffer.
	 * @param buffer The reference-counted buffer object.
	 * @return True on success. False otherwise.
	 */
	virtual void send(uint32 type, shared_ptr<Buffer> buffer) = 0;

	/**
	 * Asynchronous Send.
	 *
	 * When application calls asynchronous send, the sendAsync() will return
	 * immediately, and the given buffer is stored (as a reference) and will
	 * be written to network over time, and in the end the given asynchronous
	 * handler will be invoked when the buffer is ready for re-use or destruction.
	 *
	 * @param type The type of the buffer.
	 * @param buffer The reference-counted buffer object.
	 * @param handler The asynchronous completion handler.
	 * @return True on success. False otherwise.
	 */
	//virtual void sendAsync(uint32 type, shared_ptr<Buffer> buffer, AsyncSendCompletionHandler handler) = 0;

	/**
	 * Synchronous Send with Acknowledgment.
	 *
	 * When application calls synchronous send with acknowledgment, the acksend()
	 * will only return when the buffer has been received and processed by the
	 * other side, and the acknowledgment message has been received from the other
	 * side (after they finish processing the message completely).
	 *
	 * @param type The type of the buffer.
	 * @param buffer The reference-counted buffer object.
	 * @return True on success. False otherwise.
	 */
	virtual void acksend(uint32 type, shared_ptr<Buffer> buffer) = 0;

	/**
	 * Asynchronous Send with Acknowledgment.
	 *
	 * When application calls asynchronous send with acknowledgment, the acksendAsync()
	 * will return immediately, and the given buffer is stored (as a reference)
	 * and will be written to network over time. The asynchronous handler will be
	 * invoked when either we encounter any error during the ack send process (so
	 * the send operation simply failed), or receive the ack message from the other
	 * side (so the ack send is completed).
	 *
	 * @param type The type of the buffer.
	 * @param buffer The reference-counted buffer object.
	 * @param handler The asynchronous completion handler.
	 * @return True on success. False otherwise.
	 */
	virtual void acksendAsync(uint32 type, shared_ptr<Buffer> buffer, AsyncSendCompletionHandler handler) = 0;

public:
	/**
	 * Set the associated context object.
	 *
	 * @note The ownership of the given object is transferred to this Channel object. So
	 * the object will be deleted when the context is cleared or the Channel object is
	 * destroyed.
	 *
	 * @note Note that each Channel may have multiple context object with different types.
	 * But for each type of the context object, there are at most one instance.
	 *
	 * @param ctx The associated context object.
	 */
	template <typename T>
	inline void setContext(T* ctx)
	{
		refContext<T>() = shared_ptr<T>(ctx);
	}

	/**
	 * Get the associated context object.
	 *
	 * @note Note that each Channel may have multiple context object with different types.
	 * But for each type of the context object, there are at most one instance.
	 *
	 * @return The associated context object.
	 */
	template <typename T>
	inline T* getContext()
	{
		shared_ptr<T> ctx = static_pointer_cast<T>(refContext<T>());
		return ctx.get();
	}

	/**
	 * Clear/Reset the associated context object.
	 *
	 * @note Note that the context object will be destroyed.
	 */
	template <typename T>
	inline void clearContext()
	{
		refContext<T>().reset();
	}

public:
	/**
	 * Set the unique identifier.
	 *
	 * @note Each channel has an unique identifier as UUID used to identify itself among peers.
	 *
	 * @param id The unique identifier.
	 */
	inline void setIdentifier(const UUID& id)
	{ mIdentifier = id; }

	/**
	 * Get the unique identifier.
	 *
	 * @note Each channel has an unique identifier as UUID used to identify itself among peers.
	 *
	 * @return The unique identifier.
	 */
	inline const UUID& getIdentifier()
	{ return mIdentifier; }

private:
	/**
	 * The helper method to implement context storage.
	 *
	 * @return The reference-counted object pointer.
	 */
	template <typename T>
	inline shared_ptr<void>& refContext()
	{
		static uint32 index = mContextIndexer++;
		BOOST_ASSERT(index < kMaximumSupportedContextTypes);
		return mContext[index];
	}

private:
	const static std::size_t kMaximumSupportedContextTypes = 8UL;

	static tbb::atomic<uint32> mContextIndexer;
	shared_ptr<void> mContext[kMaximumSupportedContextTypes];

	UUID mIdentifier;
};

} }

#endif/*ZILLIANS_NET_CHANNEL_H_*/
