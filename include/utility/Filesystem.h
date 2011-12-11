/**
 * Zillians MMO
 * Copyright (C) 2007-2011 Zillians.com, Inc.
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

#ifndef ZILLIANS_FILESYSTEM_H_
#define ZILLIANS_FILESYSTEM_H_

#include <boost/filesystem.hpp>
#ifdef _WIN32
#else
#include <unistd.h>
#endif

namespace zillians {

struct Filesystem
{
private:
	Filesystem() { }
	~Filesystem() { }

public:
	// see: http://stackoverflow.com/questions/1746136/how-do-i-normalize-a-pathname-using-boostfilesystem/1750710#1750710
	static boost::filesystem::path resolve(const boost::filesystem::path& p)
	{
	    //p = boost::filesystem::absolute(p);
	    boost::filesystem::path result;
	    for(boost::filesystem::path::iterator it=p.begin();
	        it!=p.end();
	        ++it)
	    {
	        if(*it == "..")
	        {
	            // /a/b/.. is not necessarily /a if b is a symbolic link
	            if(boost::filesystem::is_symlink(result) )
	                result /= *it;
	            // /a/b/../.. is not /a/b/.. under most circumstances
	            // We can end up with ..s in our result because of symbolic links
	            else if(result.filename() == "..")
	                result /= *it;
	            // Otherwise it should be safe to resolve the parent
	            else
	                result = result.parent_path();
	        }
	        else if(*it == ".")
	        {
	            // Ignore
	        }
	        else
	        {
	            // Just cat other path entries
	            result /= *it;
	        }
	    }
	    return result;
	}

	static bool enumerate_package(const boost::filesystem::path& root, const boost::filesystem::path& p, std::deque<std::wstring>& sequence)
	{
		boost::filesystem::path t = p.parent_path();

		while (true)
		{
			if (t.empty())
				return false;

			if (t == root)
				break;
			else
				sequence.push_front(t.stem().wstring());

			t = t.parent_path();
		}

		return true;
	}

	static boost::filesystem::path normalize_path(boost::filesystem::path p)
	{
		if(p.is_absolute())
			return resolve(p);
		else
			return resolve(boost::filesystem::absolute(p));
	}

	static boost::filesystem::path current_executable_path()
	{
#ifdef _WIN32
		char p[MAX_PATH];
		GetModuleFileName(NULL, p, MAX_PATH)
		return boost::filesystem::path(p);
#else
		pid_t pid = getpid();
		char proc_link[2048]; sprintf(proc_link, "/proc/%d/exe", pid);
		char sym_link[2048];
		std::size_t len = readlink(proc_link, sym_link, 1024);
		sym_link[len] = '\0';
		return boost::filesystem::path(sym_link);
#endif
	}
};

}

#endif /* ZILLIANS_FILESYSTEM_H_ */
