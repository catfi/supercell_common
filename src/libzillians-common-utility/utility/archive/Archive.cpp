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

#include "utility/archive/Archive.h"
#include <fstream>
#include <cstring>

namespace zillians {

Archive::Archive(const std::string& archive_name, ArchiveMode mode) :
	mArchive(NULL),
	mArchiveName(archive_name),
	mArchiveMode(mode),
	mCompressLevel(Z_DEFAULT_COMPRESSION)
{}

Archive::~Archive()
{
	if (!!mArchive)
	{
		close();
	}
}

bool Archive::open()
{
	if (mArchiveMode == ArchiveMode::ARCHIVE_FILE_COMPRESS)
	{
		mArchive = zipOpen64(mArchiveName.c_str(), 0);
	}
	else
	if (mArchiveMode == ArchiveMode::ARCHIVE_FILE_DECOMPRESS)
	{
		mArchive = unzOpen64(mArchiveName.c_str());
	}

	return mArchive != NULL;
}

bool Archive::close()
{
	if (mArchive == NULL) return false;

	if (mArchiveMode == ArchiveMode::ARCHIVE_FILE_COMPRESS)
	{
		zipClose(mArchive, NULL);
	}
	else
	if (mArchiveMode == ArchiveMode::ARCHIVE_FILE_DECOMPRESS)
	{
		unzClose(mArchive);
	}

	mArchive = NULL;
	return true;
}


bool Archive::add(ArchiveItem_t& archive_item)
{
	if (mArchive == NULL || mArchiveMode != ArchiveMode::ARCHIVE_FILE_COMPRESS) return false;

	// Open file in the archive
	int result = ZIP_OK;
	result = zipOpenNewFileInZip3_64(mArchive, archive_item.filename.c_str(), &archive_item.zip_info,
								NULL, 0, NULL, 0, NULL /* comment */,
								Z_DEFLATED,
								mCompressLevel, 0,
								/* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
								-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
								NULL /* PASSWORD */, 0 /* CRC */, 0 /* large file */);
	if (result != ZIP_OK) return false;

	// Write buffer to the file in the archive
	result = zipWriteInFileInZip(mArchive, &archive_item.buffer[0], archive_item.buffer.size());
	if (result != ZIP_OK) return false;

	// Close the file in the archive
	result = zipCloseFileInZip(mArchive);
	if (result != ZIP_OK) return false;

	return true;
}

bool Archive::add(const std::string& filename)
{
	if (mArchive == NULL || mArchiveMode != ArchiveMode::ARCHIVE_FILE_COMPRESS) return false;

	ArchiveItem_t archive_item;
	archive_item.filename = filename;

	// read the file into the buffer
	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	archive_item.buffer.resize( file.tellg() );
	file.seekg(0, std::ios::beg);
	file.read( (char*)&archive_item.buffer[0], archive_item.buffer.size() );
	file.close();

	// TODO: fill the archive_item.zip_info (which inculdes file date)
	std::memset(&archive_item.zip_info, 0, sizeof(zip_fileinfo));

	return add(archive_item);
}

bool Archive::extractAll(std::vector<ArchiveItem_t>& archive_items)
{
	if (mArchive == NULL || mArchiveMode != ArchiveMode::ARCHIVE_FILE_DECOMPRESS) return false;

    unz_global_info64 global_info;

    if ( unzGetGlobalInfo64(mArchive, &global_info) != UNZ_OK) return false;

    // Extract one file at a time and then jump to the next file
   	archive_items.resize( global_info.number_entry );
    for (int i = 0; i < global_info.number_entry; i++)
    {
    	if (!extractCurrentFile(archive_items[i])) return false;

    	// We don't need to jump to the next file if the current one is that last
    	if (i != global_info.number_entry - 1)
    	{
    		int result = unzGoToNextFile(mArchive);
    		if (result != UNZ_OK) return false;
    	}
    }

	return true;
}

bool Archive::extractAllToFolder(std::vector<ArchiveItem_t>& archive_items, std::string folder_path = "")
{
	/**
	 * TODO: put the files under the folder_path, instead, currently only put to the current folder
	 * TODO: the extracted item ArchiveItem_t may contain the filename like this one some_folder/abc.txt, however,
	 * 		 currently not support to generate the unexisted folder
	 */
	if (!extractAll(archive_items)) return false;


	// Now, write to the disk
	for (int i = 0; i < archive_items.size(); i++)
	{
		ArchiveItem_t& item = archive_items[i];
		std::ofstream file(item.filename.c_str(), std::ios::out | std::ios::binary);
		file.write((char*)&item.buffer[0], item.buffer.size());
		file.close();

		// TODO: (not sure) write back unzip_info
	}
	return true;
}

bool Archive::extractCurrentFile(ArchiveItem_t& archive_item)
{
    unz_file_info64 file_info;
	int result;

	// TODO: Uh... make the file length more reasonable
	const int max_filename_length = 1024;
	char inzip_filename[max_filename_length] = {0};

	// Retrive current file information
    result = unzGetCurrentFileInfo64(mArchive, &file_info, inzip_filename, max_filename_length, NULL, 0, NULL, 0);
    if (result != UNZ_OK) return false;

    archive_item.filename = inzip_filename;
    archive_item.unzip_info = file_info;

	// Open current file
	result = unzOpenCurrentFile(mArchive);
	if (result != UNZ_OK) return false;

    // Now, retrieve the buffer chunk by chunk
    int read_count = 0;
    char byte;

    while ( (read_count = unzReadCurrentFile(mArchive, &byte, 1)) > 0 )
    {
    	archive_item.buffer.push_back(byte);
    }
    // Check unzReadCurrentFile comment, you will know there's an error if the read count is negative.
    if (read_count < 0) return false;

    // Close the current file
    result = unzCloseCurrentFile(mArchive);
    if (result != UNZ_OK) return false;

	return true;
}

void Archive::setCompressLevel(int level)
{
	// the range is 0~9
	mCompressLevel = (level < 0) ? (0) : level;
	mCompressLevel = (mCompressLevel > 9) ? (9) : mCompressLevel;
}

}
