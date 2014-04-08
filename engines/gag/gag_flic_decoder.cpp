#include "gag_flic_decoder.h"



namespace Gag
{

const uint GagFlicDecoder::_TRACK_AUDIO = 1;


GagFlicDecoder::GagFlicDecoder()
	: Video::FlicDecoder()
{
	;
}


GagFlicDecoder::~GagFlicDecoder()
{
	;
}


bool GagFlicDecoder::loadStream(Common::SeekableReadStream *stream)
{
	return init<GagFlicVideoTrack>(stream);
}


GagFlicDecoder::GagFlicVideoTrack::GagFlicVideoTrack(const FlicHeader &a_flic_header, Common::SeekableReadStream *stream)
	: FlicVideoTrack(a_flic_header, stream)
{
	;
}


GagFlicDecoder::GagFlicVideoTrack::~GagFlicVideoTrack()
{
	;
}


void GagFlicDecoder::GagFlicVideoTrack::MvzFrameHeader::Fix()
{
	area_width = FROM_LE_16(area_width);
	area_height = FROM_LE_16(area_height);
	area_offset_x = FROM_LE_16(area_offset_x);
	area_offset_y = FROM_LE_16(area_offset_y);
	width = FROM_LE_16(width);
	height = FROM_LE_16(height);
	unknown1 = FROM_LE_16(unknown1);
	unknown2 = FROM_LE_16(unknown2);
}


void GagFlicDecoder::GagFlicVideoTrack::WavFmtHeader::Fix()
{
	wave_id = FROM_BE_32(wave_id);
	chunk_id = FROM_BE_32(chunk_id);
	chunk_size = FROM_LE_32(chunk_size);
	wave_type = FROM_LE_16(wave_type);
	channels_count = FROM_LE_16(channels_count);
	sample_rate = FROM_LE_32(sample_rate);
	data_rate = FROM_LE_32(data_rate);
	block_align = FROM_LE_16(block_align);
	bits_per_sec = FROM_LE_16(bits_per_sec);
}


void GagFlicDecoder::GagFlicVideoTrack::decodeExtended(uint16 a_type, const uint8 *a_data)
{
	GagSubchunkType type = (GagSubchunkType)a_type;
	switch(type)
	{
	case GAG_MVZ5:
	case GAG_MVZ8:
	{
		MvzFrameHeader header = *(const MvzFrameHeader *)a_data;
		header.Fix();

		DecodeGagVideoMVZ(header, a_data + sizeof(header), type == GAG_MVZ8);
	}
		break;
/*
	case GAG_SND_PCM1:
		;
		break;

	case GAG_SND_PCM2:
		;
		break;

	case GAG_SND_STOP:
		;
		break;
*//*
	case GAG_SND_CTRL:
	{
		const Track *track = getTrack(_TRACK_AUDIO);
		if(track == nullptr)
		{
			track = new GagFlicAudioTrack(data);
			addTrack(_audioTrack);
		}
	}*/
		break;

	default:
		debug("warning: subchunk type not implemented (type: %d)", a_type);
	}
}


void GagFlicDecoder::GagFlicVideoTrack::DecodeGagVideoMVZ(const MvzFrameHeader &a_header, const uint8 *a_data, bool a_mvz8)
{
	const uint8 *src = a_data;

	for(uint16 y = 0; y < a_header.area_height; ++y)
	{
		uint16 dst_y = a_header.area_offset_y + y;
		uint16 count;

		if(a_mvz8)
		{
			count = READ_LE_UINT16(src);
			src += sizeof(count);
		}

		for(uint16 x = 0; a_mvz8 ? count : x < a_header.area_width;)
		{
			if(a_mvz8)
			{
				x += *src++;
				--count;
			}

			uint16 dst_x = a_header.area_offset_x + x;

			uint8 control = *src++;
			uint32 data;
			uint8 size;

			uint8 type = (control & 0xc0) >> 6;
			control &= ~0xc0;

			// block encoding:
			// type 0: 6bit - size, 8bit - fill index
			// type 1: 6bit - size, <size> bytes of index data to copy
			// type 2: 5bit - Y relative offset back from current, 9bit - X offset
			// type 3: 7bit - Y relative offset back from current, 10bit - X offset, 5bit - size
			switch(type)
			{
			case 0:
				size = control;
				memset(_surface->getBasePtr(dst_x, dst_y), *src, size);
				_dirtyRects.push_back(Common::Rect(dst_x, dst_y, dst_x + size, dst_y + 1));
				++src;
				break;

			case 1:
				size = control;
				memcpy(_surface->getBasePtr(dst_x, dst_y), src, size);
				_dirtyRects.push_back(Common::Rect(dst_x, dst_y, dst_x + size, dst_y + 1));
				src += size;
				break;

			case 2:
			{
				data = control << 8 | *src++;
				size = 3;
				uint16 src_x = a_header.area_offset_x + (data & 0x1ff);
				uint16 src_y = a_header.area_offset_y + (y - (data >> 9 & 0x1f));
				memcpy(_surface->getBasePtr(dst_x, dst_y), _surface->getBasePtr(src_x, src_y), size);
				_dirtyRects.push_back(Common::Rect(dst_x, dst_y, dst_x + size, dst_y + 1));
			}
				break;

			case 3:
			{
				data = control << 16 | *src << 8 | *(src + 1);
				src += 2;
				size = data & 0x1f;
				uint16 src_x = a_header.area_offset_x + (data >> 5 & 0x3ff);
				uint16 src_y = a_header.area_offset_y + (y - (data >> 15 & 0x7f));
				memcpy(_surface->getBasePtr(dst_x, dst_y), _surface->getBasePtr(src_x, src_y), size);
				_dirtyRects.push_back(Common::Rect(dst_x, dst_y, dst_x + size, dst_y + 1));
			}
			}

			x += size;
		}
	}
}


GagFlicDecoder::GagFlicAudioTrack::GagFlicAudioTrack()
{
	;
}


GagFlicDecoder::GagFlicAudioTrack::~GagFlicAudioTrack()
{
	;
}


Audio::AudioStream *GagFlicDecoder::GagFlicAudioTrack::getAudioStream() const
{
	return nullptr;
}

}
