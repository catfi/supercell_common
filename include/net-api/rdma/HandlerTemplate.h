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
 * @date Feb 16, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_RDMA_HANDLERTEMPLATE_H_
#define ZILLIANS_NET_RDMA_HANDLERTEMPLATE_H_

namespace zillians { namespace net { namespace rdma {

template <typename Connection>
class HandlerTemplate
{
public:
	HandlerTemplate() { }
	virtual ~HandlerTemplate() { }

public:
	virtual void handle(int32 type, shared_ptr<Buffer> b, shared_ptr<Connection> connection) = 0;
};

} } }

#endif/*ZILLIANS_NET_RDMA_HANDLERTEMPLATE_H_*/
