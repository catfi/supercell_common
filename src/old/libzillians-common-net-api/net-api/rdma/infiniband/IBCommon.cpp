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
 * @date Mar 1, 2009 sdk - Initial version created.
 */

#include "networking/rdma/infiniband/IBCommon.h"

namespace zillians { namespace net { namespace rdma {

bool IBCheckConfiguration(log4cxx::LoggerPtr &mLogger)
{
	IB_INFO("[CHECK INFINIBAND RDMA CONFIGURATION]");

	// check the ack buffer count
	if((IB_DEFAULT_RECEIVE_BUFFER_COUNT / IB_DEFAULT_THRESHOLD_BUFFER_ACK) - 1 >= IB_DEFAULT_ACK_BUFFER_COUNT)
	{
		IB_ERROR("[ERROR] ack buffer count design rule check failed");
		IB_ERROR("IB_DEFAULT_ACK_BUFFER_COUNT = " << IB_DEFAULT_ACK_BUFFER_COUNT);
		IB_ERROR("IB_DEFAULT_DATA_BUFFER_COUNT = " << IB_DEFAULT_DATA_BUFFER_COUNT);
		IB_ERROR("IB_DEFAULT_RECEIVE_BUFFER_COUNT = " << IB_DEFAULT_RECEIVE_BUFFER_COUNT);
		IB_ERROR("IB_DEFAULT_THRESHOLD_BUFFER_ACK = " << IB_DEFAULT_THRESHOLD_BUFFER_ACK);
		IB_ERROR("IB_DEFAULT_ACK_BUFFER_COUNT = " << IB_DEFAULT_ACK_BUFFER_COUNT);
		BOOST_ASSERT((IB_DEFAULT_RECEIVE_BUFFER_COUNT / IB_DEFAULT_THRESHOLD_BUFFER_ACK) - 1 < IB_DEFAULT_ACK_BUFFER_COUNT);
	}

	// check control buffer count
	if(IB_DEFAULT_CONTROL_BUFFER_COUNT <= (IB_DEFAULT_MAX_SEND_IN_FLIGHT+IB_DEFAULT_WR_ENTRIES) / IB_DEFAULT_ACK_BUFFER_COUNT)
	{
		IB_ERROR("[ERROR] control buffer count design rule check failed");
		IB_ERROR("IB_DEFAULT_CONTROL_BUFFER_COUNT = " << IB_DEFAULT_CONTROL_BUFFER_COUNT);
		IB_ERROR("IB_DEFAULT_MAX_SEND_IN_FLIGHT = " << IB_DEFAULT_MAX_SEND_IN_FLIGHT);
		IB_ERROR("IB_DEFAULT_WR_ENTRIES = " << IB_DEFAULT_WR_ENTRIES);
		IB_ERROR("IB_DEFAULT_THRESHOLD_BUFFER_ACK = " << IB_DEFAULT_ACK_BUFFER_COUNT);
		// here we multiple by 2 because the large send will generate 2 control buffer (one send request, one send ack)
		BOOST_ASSERT(IB_DEFAULT_CONTROL_BUFFER_COUNT > (IB_DEFAULT_MAX_SEND_IN_FLIGHT+IB_DEFAULT_WR_ENTRIES)*2 / IB_DEFAULT_THRESHOLD_BUFFER_ACK);
	}

}

} } }
