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
 * @date May 19, 2010 jerry - Initial version created.
 */

#include "utility/crypto/machine_info.h"
#ifdef __PLATFORM_LINUX__
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <linux/if.h>
	#include <netdb.h>
#endif
#include <stdio.h>
#include <string.h>
#include <string>

static std::string _rtrim_char(const std::string &s, char c)
{
    std::string s2 = s;
    while(!s2.empty() && s2[s2.length()-1] == c)
        s2.erase(s2.length()-1);
    return s2;
}

namespace zillians {

std::string GetMacAddress()
{
#ifdef __PLATFORM_LINUX__
	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	strcpy(s.ifr_name, "eth0");
	if(0 != ioctl(fd, SIOCGIFHWADDR, &s))
		return "";
	std::string result;
	for(int i = 0; i<6; ++i)
	{
		char buf[20];
		sprintf(buf, "%02x ", static_cast<unsigned char>(s.ifr_addr.sa_data[i]));
		result.append(buf);
	}
	result = _rtrim_char(result, ' ');
	return result;
#endif

#ifdef __PLATFORM_WINDOWS__
	// NOTE: not supported
#endif
}

}
