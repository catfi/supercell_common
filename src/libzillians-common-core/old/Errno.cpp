// 
// Zillians MMO
// Copyright (C) 2007-2008 Zillians.com, Inc.
// For more information see http://www.zillians.com
//
// Zillians MMO is the library and runtime for massive multiplayer online game
// development in utility computing model, which runs as a service for every 
// developer to build their virtual world running on our GPU-assisted machines
//
// This is a close source library intended to be used solely within Zillians.com
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
//
// Contact Information: info@zillians.com
//

#include "core/Errno.h"
#include <tbb/concurrent_vector.h>
#include <errno.h>

namespace zillians {

//////////////////////////////////////////////////////////////////////////
Errno::Errno()
{
	errno = 0;
}

Errno::~Errno()
{
}

//////////////////////////////////////////////////////////////////////////
void Errno::reset()
{
	errno = 0;
}

//////////////////////////////////////////////////////////////////////////
/*
int Errno::code()
{
	return errno;
}
*/

//////////////////////////////////////////////////////////////////////////
#ifdef __ZN_WINDOWS__
#else
#endif

#include <string.h>
const char* Errno::brief()
{
	return brief(Errno::code());
}

const char* Errno::brief(int code)
{
	static char buffer[256];
	
	strcpy(buffer, strerror(code));
	return buffer;
}

const char* Errno::detail()
{
	return Errno::detail(Errno::code());
}

const char* Errno::detail(int code)
{
	static char detail[256];
	
	sprintf(detail, "%d (%s)", code, strerror(code));
	
	return detail;
}

}
