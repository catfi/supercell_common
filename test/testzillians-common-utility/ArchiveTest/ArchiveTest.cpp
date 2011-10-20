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
#include "utility/archive/Archive.h"
#include <tr1/unordered_set>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <iostream>

#define BOOST_TEST_MODULE ArchiveTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;

BOOST_AUTO_TEST_SUITE( ArchiveTestSuit )

BOOST_AUTO_TEST_CASE( Archive_Test )
{
	// Random create several source files
	const int generated_file_count = 4;
	std::vector< boost::filesystem::path > sources;

	for (int i = 0; i < generated_file_count; i++)
	{
		// Create the file name with the random content
		UUID source_filename;
		source_filename.random();
		boost::filesystem::path source_filepath = operator/(boost::filesystem::path("/tmp"), (std::string)source_filename);
		std::string command = "dd if=/dev/urandom of=" + source_filepath.generic_string() + " bs=1M count=1";
		system(command.c_str());

		std::cout << "Source file: " << source_filepath << std::endl;
		sources.push_back(source_filepath);
	}

	// Create a random archive name
	UUID archive_name;
	archive_name.random();
	boost::filesystem::path archive_path = operator/(boost::filesystem::path("/tmp"), (std::string)archive_name + std::string(".zip"));
	std::cout << "Archive file: " << archive_path << std::endl;

	// Archive the content
	{
		Archive ar(archive_path.generic_string(), ArchiveMode::ARCHIVE_FILE_COMPRESS);

		BOOST_CHECK( ar.open() );
		// Add to archive
		for (int i = 0; i < generated_file_count; i++)
			ar.add(sources[i].generic_string());
		BOOST_CHECK( ar.close() );
	}

	// Extract the content
	std::vector<ArchiveItem_t> archive_items;
	{
		Archive ar(archive_path.generic_string(), ArchiveMode::ARCHIVE_FILE_DECOMPRESS);

		BOOST_CHECK( ar.open() );
		BOOST_CHECK( ar.extractAll(archive_items) );
		BOOST_CHECK( ar.close() );
	}

	// Check whether we have all the file names and check the content
	int match_count = 0;
	for (int i = 0; i < archive_items.size(); i++)
	{
		for (int j = 0; j < sources.size(); j++)
		{
			if (archive_items[i].filename == sources[i].generic_string())
			{
				match_count++;

				// Read the source buffer
				std::vector<unsigned char> source_buffer;
				std::ifstream source_file(sources[i].generic_string().c_str(), std::ios::in | std::ios::binary | std::ios::ate);
				source_buffer.resize( source_file.tellg() );
				source_file.seekg (0, std::ios::beg);
				source_file.read((char*)&source_buffer[0], source_buffer.size());
				source_file.close();

				// Compare with the extracted buffer
				BOOST_CHECK( archive_items[i].buffer == source_buffer );

				break;
			}
		}
	}

	BOOST_CHECK( match_count == generated_file_count );

	// Finally, remove the dirty files
	std::remove(archive_path.generic_string().c_str());
	for (int i = 0; i < sources.size(); i++)
	{
		std::remove(sources[i].generic_string().c_str());
	}
}

BOOST_AUTO_TEST_SUITE_END()
