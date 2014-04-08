#ifndef CDF_ARCHIVE_H__
#define CDF_ARCHIVE_H__



#include "common/archive.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/stream.h"



namespace Gag
{

class CdfArchive : public Common::Archive
{
public:
	CdfArchive(const Common::String &a_fn, bool a_case_sensitive = false);
	virtual ~CdfArchive();

	virtual bool hasFile(const Common::String &name) const;
	virtual int listMembers(Common::ArchiveMemberList &list) const;
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;

private:
#include "common/pack-start.h"
	struct Header
	{
		char magic[7];
		uint32 index_size_enc;
		uint32 files_count;
		uint32 size_total_dec;

		void Fix()
		{
			le2sys(index_size_enc);
			le2sys(files_count);
			le2sys(size_total_dec);
		}
	};

	struct FileIndex
	{
		uint8 type;
		char filename[35];
		uint32 offset;
		uint32 size_dec;

		void Fix()
		{
			le2sys(offset);
			le2sys(size_dec);
		}
	};
#include "common/pack-end.h"

	struct FileEntry
	{
		uint32 offset;
		uint32 size;
		bool compressed;
	};

	bool m_CaseSensitive;
	mutable Common::File m_File;
	Common::HashMap<Common::String, FileEntry> m_FileEntries;

	const FileEntry *GetFileEntry(const Common::String &a_fn) const;
	void FileDecode(uint8 *dst, uint32 a_dst_size, uint32 a_offset) const;
	uint32 BlockDecode(uint8 *dst, uint32 a_dst_size, const uint8 *a_src, uint32 a_src_size) const;
};

}



#endif
