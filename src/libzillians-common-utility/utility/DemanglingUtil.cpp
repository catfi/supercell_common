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
 * @date Aug 10, 2010 sdk - Initial version created.
 */

#include "utility/DemanglingUtil.h"

#if defined(__GNUC__)
    #include <cxxabi.h>
    #include <cstdlib>
#endif

namespace zillians {

std::string demangle(const std::type_info &ti)
{
#if defined(__GNUC__)

    int status = 0;

    struct raii
    {
        raii(char *text) : text_(text) { }
        ~raii() { free(text_); }
        char *text_;
    }
    demangled(abi::__cxa_demangle(ti.name(), 0, 0, &status));

    if (status == 0) return demangled.text_;

#endif
    return ti.name();
}

}
