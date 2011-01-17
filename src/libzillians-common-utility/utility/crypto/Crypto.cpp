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

#include "utility/crypto/Crypto.h"
#include "utility/crypto/base64.h"
#include <ctype.h>
#include <iostream>

static char _encode_char_rolling_offset(char data_char, char key_char)
{
	if(!isalpha(data_char) || !isalpha(key_char))
		return data_char;
	size_t range_size = 'z'-'a'+1;
	if(isupper(data_char))
		return 'A'+((data_char-'A')+(toupper(key_char)-'A'))%range_size;
	if(islower(data_char))
		return 'a'+((data_char-'a')+(tolower(key_char)-'a'))%range_size;
	return 'A';
}
static char _decode_char_rolling_offset(char data_char, char key_char)
{
	if(!isalpha(data_char) || !isalpha(key_char))
		return data_char;
	size_t range_size = 'z'-'a'+1;
	if(isupper(data_char))
		return 'A'+(static_cast<size_t>(range_size)+(data_char-'A')-(toupper(key_char)-'A'))%range_size;
	if(islower(data_char))
		return 'a'+(static_cast<size_t>(range_size)+(data_char-'a')-(tolower(key_char)-'a'))%range_size;
	return 'A';
}

namespace zillians {

std::string Crypto_t::encryptStringBasic(std::string Data, std::string Key, bool PostBase64Encode)
{
	std::string EncryptedData;
	for(int i = 0; i<Data.length(); i++)
		EncryptedData.append(1, _encode_char_rolling_offset(Data.c_str()[i], Key.c_str()[i%Key.length()]));
	if(PostBase64Encode)
		return base64_encode(reinterpret_cast<const unsigned char*>(EncryptedData.c_str()), EncryptedData.length());
	return EncryptedData;
}
std::string Crypto_t::decryptStringBasic(std::string Data, std::string Key, bool PreBase64Decode)
{
	if(PreBase64Decode)
		Data = base64_decode(Data);
	std::string DecryptedData;
	for(int i = 0; i<Data.length(); i++)
		DecryptedData.append(1, _decode_char_rolling_offset(Data.c_str()[i], Key.c_str()[i%Key.length()]));
	return DecryptedData;
}

}
