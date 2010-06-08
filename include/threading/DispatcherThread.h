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
 * @date Jun 8, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_THREADING_DISPATCHERTHREAD_H_
#define ZILLIANS_THREADING_DISPATCHERTHREAD_H_

#include "core-api/Prerequisite.h"
#include "core-api/SharedPtr.h"
#include "core-api/Semaphore.h"
#include "core-api/ContextHub.h"
#include "threading/DispatcherNetwork.h"
#include "threading/DispatcherDestination.h"

namespace zillians { namespace threading {

template<typename Message>
class DispatcherThread : public ContextHub<ContextOwnership::transfer>
{
public:
	DispatcherThread(DispatcherNetwork<Message>* dispatcher, uint32 self_id);
	~DispatcherThread();

public:
	uint32 getIdentity() const;
	DispatcherNetwork<Message>* getDispatcherNetwork() const;

public:
	shared_ptr<DispatcherDestination> createDestination(uint32 dest);

public:
	/**
	 * Read the first message available from any pipes
	 * @param source
	 * @param message
	 */
	bool read(uint32& source, Message* message, bool blocking = false);

public:
	Semaphore getSignaler();

public:
	void processCommand();
};

} }

#endif /* ZILLIANS_THREADING_DISPATCHERTHREAD_H_ */
