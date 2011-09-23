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
#include "utility/crypto/machine_info.h"
#include <ctype.h>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/evp.h>

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
std::string Crypto_t::genHardwareIdentKey()
{
	return GetMacAddress();
}


bool Crypto_t::symmetricCipher(const std::vector<unsigned char>& in_buffer, int nid, const std::string& key, const std::string& iv, bool encode, std::vector<unsigned char>& buffer)
{
	// load all cipher modules
	OpenSSL_add_all_ciphers();

	// Select the specific cipher module
	const EVP_CIPHER* cipher = EVP_get_cipherbynid(nid);
	if (!cipher) return false;

	// Each cipher has its own taste for the key and IV. So we need to check if the input key and IV is appropriate
	// for the specific cipher module
	if (key.size() < EVP_CIPHER_key_length(cipher) || iv.size() < EVP_CIPHER_iv_length(cipher))
	{
		return false;
	}

	EVP_CIPHER_CTX ctx = {0};
	EVP_CIPHER_CTX_init(&ctx);
	EVP_CipherInit_ex(&ctx, cipher, NULL, (const unsigned char*)key.c_str(), (const unsigned char*)iv.c_str(), encode);
	size_t block_size = EVP_CIPHER_block_size(cipher);
	unsigned char* encrypt_buffer = (unsigned char*) malloc(block_size + in_buffer.size());

	// Read the raw buffer and convert to encrypt one. And then collect to the output buffer

	int out_count = 0;
	buffer.clear();
	bool fail = false;

	while (true)
	{
		if (!EVP_CipherUpdate(&ctx, encrypt_buffer, &out_count, &in_buffer[0], in_buffer.size()))
		{
			fail = true;
			break;
		}
		for (int i = 0; i < out_count; i++)
			buffer.push_back(encrypt_buffer[i]);

		// handling the last block
		unsigned char* block = encrypt_buffer + out_count;
		if (!EVP_CipherFinal_ex(&ctx, block, &out_count))
		{
			fail = true;
			break;
		}
		for (int i = 0; i < out_count; i++)
			buffer.push_back(block[i]);
		break;
	}

	// free resource
	free(encrypt_buffer);
	EVP_CIPHER_CTX_cleanup(&ctx);
	return (fail == true) ? (false) : (true);
}

bool Crypto_t::symmetricCipher(const std::string& file, int nid, const std::string& key, const std::string& iv, bool encode, std::vector<unsigned char>& buffer)
{
	/*
	 * Read the buffer from files
	 */
	BIO* bin;
	if ( (bin = BIO_new_file(file.c_str(), "rb")) == NULL )
	{
		return false;
	}

	int read_count = 0;
	unsigned char raw_buffer[1024];
	std::vector<unsigned char> in_buffer;
	while ((read_count = BIO_read(bin, raw_buffer, sizeof(raw_buffer))) > 0)
	{
		for (int i = 0; i < read_count; i++)
			in_buffer.push_back(raw_buffer[i]);
	}

	BIO_free_all(bin);

	/*
	 * Do the real symmetric cipher
	 */
	return symmetricCipher(in_buffer, nid, key, iv, encode, buffer);
}

}
