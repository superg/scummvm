#include "common/debug.h"
#include "common/memstream.h"
#include "common/zlib.h"
#include "cdf_archive.h"



using namespace Common;



namespace Gag
{

CdfArchive::CdfArchive(const String &a_fn, bool a_case_sensitive)
	: m_CaseSensitive(a_case_sensitive)
{
	if(!m_File.open(a_fn))
	{
		warning("CdfArchive::CdfArchive(): Could not find the archive file");
		return;
	}

	Header header;
	m_File.read(&header, sizeof(header));
	header.Fix();
	if(strcmp(header.magic, "CDF97a"))
	{
		warning("CdfArchive::CdfArchive(): Could not find a valid header");
		return;
	}

	debug("CdfArchive::CdfArchive(%s): compressed index size: %u", a_fn.c_str(), header.index_size_enc);
	debug("CdfArchive::CdfArchive(%s): files count: %u", a_fn.c_str(), header.files_count);
	debug("CdfArchive::CdfArchive(%s): total data amount: %u", a_fn.c_str(), header.size_total_dec);

	// create hashmap from file indices for faster access
	FileIndex *file_indices = new FileIndex[header.files_count];
	FileDecode((uint8 *)file_indices, header.files_count * sizeof(FileIndex), m_File.size() - header.index_size_enc);
	for(uint32 i = 0; i < header.files_count; ++i)
	{
		FileEntry file_entry;
		file_entry.offset = file_indices[i].offset;
		file_entry.size = file_indices[i].size_dec;
		file_entry.compressed = file_indices[i].type & 0x10;

		String filename(file_indices[i].filename);
		if(!m_CaseSensitive)
			filename.toLowercase();

		m_FileEntries[filename] = file_entry;
	}
	delete [] file_indices;
}


CdfArchive::~CdfArchive()
{
	;
}


bool CdfArchive::hasFile(const String &name) const
{
	return GetFileEntry(name) != NULL;
}


int CdfArchive::listMembers(ArchiveMemberList &list) const
{
	for(HashMap<String, FileEntry>::const_iterator it = m_FileEntries.begin(); it != m_FileEntries.end(); ++it)
		list.push_back(ArchiveMemberList::value_type(new GenericArchiveMember(it->_key, this)));

	return m_FileEntries.size();
}


const ArchiveMemberPtr CdfArchive::getMember(const String &name) const
{
	return hasFile(name) ? ArchiveMemberPtr(new GenericArchiveMember(name, this)) : ArchiveMemberPtr();
}


SeekableReadStream *CdfArchive::createReadStreamForMember(const String &name) const
{
	SeekableReadStream *stream = NULL;

	const FileEntry *file_entry = GetFileEntry(name);
	if(file_entry != NULL)
	{
		byte *data_ptr = (byte *)malloc(file_entry->size);

		// compressed file
        if(file_entry->compressed)
			FileDecode((uint8 *)data_ptr, file_entry->size, file_entry->offset);
		// uncompressed file
        else
        {
			//TODO: sequential reading for this case?
			m_File.seek(file_entry->offset);
			m_File.read(data_ptr, file_entry->size);
        }

		stream = new MemoryReadStream(data_ptr, file_entry->size, DisposeAfterUse::YES);
	}

	return stream;
}


const CdfArchive::FileEntry *CdfArchive::GetFileEntry(const String &a_fn) const
{
	String fn(a_fn);
	if(!m_CaseSensitive)
		fn.toLowercase();

	HashMap<String, FileEntry>::const_iterator it = m_FileEntries.find(fn);
	return it == m_FileEntries.end() ? NULL : &it->_value;
}


void CdfArchive::FileDecode(uint8 *dst, uint32 a_dst_size, uint32 a_offset) const
{
	m_File.seek(a_offset);

	// there are N block offsets, first offset is read first,
	// calculate from it next offsets count and read others
	uint32 offset0;
	m_File.read(&offset0, sizeof(offset0));
	uint32 *block_offsets = new uint32[offset0 / sizeof(uint32)];
	block_offsets[0] = offset0;
	uint32 blocks_count = offset0 / sizeof(uint32) - 1;
	m_File.read(&block_offsets[1], blocks_count * sizeof(uint32));

	// reassemble file from blocks
	uint32 offset = 0;
	for(uint32 i = 0; i < blocks_count; ++i)
	{
		m_File.seek(a_offset + block_offsets[i]);
		uint32 block_size = block_offsets[i + 1] - block_offsets[i];

		uint8 *buffer = new uint8[block_size];
		m_File.read(buffer, block_size);
		offset += BlockDecode(&dst[offset], a_dst_size - offset, buffer, block_size);
		delete [] buffer;
	}

	delete [] block_offsets;
}


uint32 CdfArchive::BlockDecode(uint8 *dst, uint32 a_dst_size, const uint8 *a_src, uint32 a_src_size) const
{
	uint32 dst_size = 0;

	uint16 data_type = READ_LE_UINT16(a_src);

	const uint8 *src = a_src + sizeof(uint16);
	uint32 src_size = a_src_size - sizeof(uint16);

	// data uncompressed, just copy contents
	if(data_type == 0)
	{
		memcpy(dst, src, src_size);
		dst_size = src_size;
	}
	// deflate compression used
	else if(data_type == 8)
		dst_size = inflateZlibHeaderless((byte *)dst, a_dst_size, src, src_size);

	return dst_size;
}

}
