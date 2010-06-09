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

#ifndef ZILLIANS_THREADING_DISPATCHERNETWORK_H_
#define ZILLIANS_THREADING_DISPATCHERNETWORK_H_

namespace zillians { namespace threading {

template<typename Message>
struct DispatcherNetwork
{
	virtual void write(uint32 source, uint32 destination, const Message& message, bool incomplete) = 0;
	virtual bool read(uint32 source, uint32 destination, Message* message) = 0;
	virtual void distroyThreadContext(uint32 contextId) = 0;
};

} }

#endif /* ZILLIANS_THREADING_DISPATCHERNETWORK_H_ */
