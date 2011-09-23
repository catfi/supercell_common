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

#include "core/Prerequisite.h"
#include "utility/crypto/Crypto.h"
#include <tr1/unordered_set>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/filesystem.hpp>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>

#define BOOST_TEST_MODULE CryptoTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;

BOOST_AUTO_TEST_SUITE( CryptoTestSuite )

BOOST_AUTO_TEST_CASE( Cipher_AES_256_CBC_Test )
{
	// Random create a source file
	UUID source_filename;
	source_filename.random();
	boost::filesystem::path source_filepath = operator/(boost::filesystem::path("/tmp"), (std::string)source_filename);
	std::string command = "dd if=/dev/urandom of=" + source_filepath.generic_string() + " bs=1M count=1";
	system(command.c_str());

	std::cout << "Source file: " << source_filepath << std::endl;

	// Read the target file
	std::vector<unsigned char> source_buffer;
	std::ifstream source_file(source_filepath.generic_string().c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	source_buffer.resize( source_file.tellg() );
	source_file.seekg (0, std::ios::beg);
	source_file.read((char*)&source_buffer[0], source_buffer.size());
	source_file.close();
	std::cout << "Source file size = " << source_buffer.size() << std::endl;

	// Encrypt the content to buffer
	std::string key = "37006c38e5a711e0b49b6cf0490d7a2f";
	std::string iv = "4fcab246e5a711e0a5c36cf0490d7a2f";

	std::vector<unsigned char> encrypt_buffer;
	BOOST_CHECK( Crypto_t::symmetricCipher(source_filepath.generic_string(), NID_aes_256_cbc, key, iv, true, encrypt_buffer) );
	std::cout << "Encrypt buffer size = " << encrypt_buffer.size() << std::endl;

	// Write encrypt buffer to file
	UUID encrypt_filename;
	encrypt_filename.random();
	boost::filesystem::path encrypt_filepath = operator/(boost::filesystem::path("/tmp"), (std::string)encrypt_filename);
	std::cout << "Write encrypt file to: " << encrypt_filepath << std::endl;

	std::ofstream encrypt_file(encrypt_filepath.generic_string().c_str(), std::ios::out | std::ios::binary);
	encrypt_file.write((const char*)&encrypt_buffer[0], encrypt_buffer.size());
	encrypt_file.close();

	// Decrypt from the encrypt file
	std::vector<unsigned char> decrypt_buffer;
	BOOST_CHECK( Crypto_t::symmetricCipher(encrypt_filepath.generic_string(), NID_aes_256_cbc, key, iv, false, decrypt_buffer) );
	std::cout << "Decrypt buffer size = " << decrypt_buffer.size() << std::endl;

	// Check the buffer whether it is the same
	BOOST_CHECK( source_buffer == decrypt_buffer );

	std::remove(source_filepath.generic_string().c_str());
	std::remove(encrypt_filepath.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
