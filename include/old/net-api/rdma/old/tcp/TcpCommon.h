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
 * @date Mar 2, 2009 sdk - Initial version created.
 */


#ifndef TCPCOMMON_H_
#define TCPCOMMON_H_

#include "core-api/Buffer.h"
#include <sys/uio.h>

//////////////////////////////////////////////////////////////////////////
//#define TCP_ENABLE_DEBUG

//////////////////////////////////////////////////////////////////////////
#ifdef TCP_ENABLE_DEBUG
#define TCP_DEBUG(x) { LOG4CXX_DEBUG(mLogger, x); }
#define TCP_ERROR(x) { LOG4CXX_ERROR(mLogger, x); }
#define TCP_INFO(x)  { LOG4CXX_INFO (mLogger, x); }
#else
#define TCP_DEBUG(x) { }
#define TCP_ERROR(x) { LOG4CXX_ERROR(mLogger, x); }
#define TCP_INFO(x)  { }
#endif

//////////////////////////////////////////////////////////////////////////
#define TCP_DEFAULT_RECEIVE_BUFFER_SIZE		256*1024
#define TCP_DEFAULT_BUFFER_HEADER_SIZE		(sizeof(uint32) + sizeof(uint32))
#define TCP_DEFAULT_MAX_SEND_IN_FLIGHT		256
#define TCP_DEFAULT_MXA_SEND_SIZE			(TCP_DEFAULT_RECEIVE_BUFFER_SIZE - TCP_DEFAULT_BUFFER_HEADER_SIZE)

#endif/*TCPCOMMON_H_*/
