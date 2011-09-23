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
 * @date May 19, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_CRYPTO_H_
#define ZILLIANS_CRYPTO_H_

#include <string>
#include <vector>

namespace zillians {

struct Crypto_t
{
	static std::string encryptStringBasic(std::string Data, std::string Key, bool PostBase64Encode = true);
	static std::string decryptStringBasic(std::string Data, std::string Key, bool PreBase64Decode = true);
	static std::string genHardwareIdentKey();

	/**
	 * Symmetric cipher by using openssl library.
	 *
	 * @param file : input file path for decrypt/encrypt
	 * @param nid : specify algorithm (ref. grep NID_ /usr/include/openssl/obj_mac.h)
	 * @param key : key for specific algorithm
	 * @param iv : IV for specific algorithm
	 * @param encode : true if we encode the buffer from file; otherwise, decode it from file
	 * @param buffer: the output buffer
	 * @return True if success; otherwise, false
	 */
	static bool symmetricCipher(const std::string& file, int nid, const std::string& key, const std::string& iv, bool encode, std::vector<unsigned char>& buffer);
};

}

#endif
