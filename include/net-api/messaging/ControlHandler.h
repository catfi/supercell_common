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
 * @date Mar 19, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_MESSAGING_CONTROLHANDLER_H_
#define ZILLIANS_MESSAGING_CONTROLHANDLER_H_

namespace zillians { namespace messaging {

struct ControlHandler
{
	virtual void handle(uint32 type, SharedPtr<Buffer> b, SharedPtr<Connection> connection) = 0;
};

struct ControlHandlerBinder : public ControlHandler
{
	struct placeholders
	{
		static inline boost::arg<1> type()
		{
		  return boost::arg<1>();
		}

		static inline boost::arg<2> buffer()
		{
		  return boost::arg<2>();
		}
	};

	typedef typename boost::function< void (uint32, SharedPtr<Buffer>) > handler_type;

	ControlHandlerBinder(handler_type h) : handler(h) { }

	virtual void handle(uint32 type, SharedPtr<Buffer> b)
	{
		handler(type, b);
	}
	handler_type handler;
};

} }

#endif /* ZILLIANS_MESSAGING_CONTROLHANDLER_H_ */
