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
 * @date May 2, 2009 sdk - Initial version created.
 */

#include "utility/TimerUtil.h"
#include <stdint.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/time.h>

namespace zillians {

uint64_t TimerUtil::clock_get_time_ms()
{
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t time_mu_s = static_cast<uint64_t>(ts.tv_sec)*1000000LL+static_cast<uint64_t>(ts.tv_nsec)/1000LL;
	return time_mu_s/1000;
}

}
