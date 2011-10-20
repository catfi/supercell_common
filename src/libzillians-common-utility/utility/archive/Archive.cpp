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
	mArchiveMode(mode)
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
								Z_DEFAULT_COMPRESSION, 0,
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

	// TODO: fill the archive_item.zip_info
	std::memset(&archive_item.zip_info, 0, sizeof(zip_fileinfo));

	return add(archive_item);
}

bool Archive::extractAll(std::vector<ArchiveItem_t>& archive_items)
{
	return true;
}

}
