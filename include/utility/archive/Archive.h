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

#ifndef ZILLIANS_ARCHIVE_H_
#define ZILLIANS_ARCHIVE_H_

#include "zlib/minizip/zip.h"
#include "zlib/minizip/unzip.h"

#include <string>
#include <vector>

namespace zillians {

typedef voidp zip_file_t;

enum ArchiveMode
{
	ARCHIVE_FILE_COMPRESS,
	ARCHIVE_FILE_DECOMPRESS,
};

struct ArchiveItem_t
{
	std::vector<unsigned char> buffer;

	// The filename is the name represented in the archive.
	std::string filename;

	union
	{
		zip_fileinfo zip_info;
		unz_file_info64 unzip_info;
	};
};


class Archive
{
public:
	Archive(const std::string& archive_name, ArchiveMode mode);
	virtual ~Archive();

public:
	/**
	 * Open the archive, which will create an empty one.
	 */
	bool open();

	/**
	 * Close the archive
	 */
	bool close();

	/**
	 * Add series of buffer into the archive with the file name written in the archive file structure.
	 *
	 * @param archive_item : the structure specify the buffer and filename to store into the archive
	 * @return True if success; otherwise, false
	 */
	bool add(ArchiveItem_t& archive_item);

	/**
	 * Add a file into the archive in which the file name is the same as the input parameter
	 *
	 * @param filename : the file name which could be used to access local file
	 * @return True if success; otherwise, false
	 */
	bool add(const std::string& filename);

	/**
	 * Extract all files in the archive, represented as a list of ArchiveItem_t
	 *
	 * @param archive_items : return a list of archive items
	 * @return True if success; otherwise, false
	 */
	bool extractAll(std::vector<ArchiveItem_t>& archive_items);

	/**
	 * Extract all files in the archive to the specific folder, also return a list of ArchiveItem_t
	 *
	 * @param archive_items: return a list of archive items
	 * @param folder_path : the folder to place the extracted files
	 * @return True if success; otherwise, false
	 */
	bool extractAllToFolder(std::vector<ArchiveItem_t>& archive_items, std::string folder_path = "");

	/**
	 * Set the compress level. Range is 0~9.
	 *
	 * @param level : level of compression
	 */
	void setCompressLevel(int level);

private:
	bool extractCurrentFile(ArchiveItem_t& archive_item);

private:
	zip_file_t mArchive;
	std::string mArchiveName;
	ArchiveMode mArchiveMode;

	// Only work for compression
	int mCompressLevel;
};

}

#endif
