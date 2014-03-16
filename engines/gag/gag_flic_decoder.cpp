#include "gag_flic_decoder.h"



namespace Gag
{

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


GagFlicDecoder::GagFlicVideoTrack::GagFlicVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height)
	: FlicVideoTrack(stream, frameCount, width, height)
{
	;
}


GagFlicDecoder::GagFlicVideoTrack::~GagFlicVideoTrack()
{
	;
}


void GagFlicDecoder::GagFlicVideoTrack::decodeExtended(uint16 subchunkType, uint8 *data, uint32 dataSize)
{
	GagSubchunkType type = (GagSubchunkType)subchunkType;
	switch(type)
	{
	case GAG_MVZ5:
	case GAG_MVZ8:
		DecodeGagVideoMVZ(data, dataSize, type == GAG_MVZ8);
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

	case GAG_SND_CTRL:
		;
		break;
*/
	default:
		debug("warning: subchunk type not implemented (type: %d, size: %d)", subchunkType, dataSize);
	}
}


void GagFlicDecoder::GagFlicVideoTrack::DecodeGagVideoMVZ(const uint8 *a_data, uint32 a_data_size, bool a_mvz8)
{
	const uint8 *src = a_data;

	MvzFrameHeader header = *(const MvzFrameHeader *)src;
	header.Fix();
	src += sizeof(MvzFrameHeader);

	for(uint16 y = 0; y < header.area_height; ++y)
	{
		uint16 dst_y = header.area_offset_y + y;
		uint16 count;

		if(a_mvz8)
		{
			count = READ_LE_UINT16(src);
			src += sizeof(count);
		}

		for(uint16 x = 0; a_mvz8 ? count : x < header.area_width;)
		{
			if(a_mvz8)
			{
				x += *src++;
				--count;
			}

			uint16 dst_x = header.area_offset_x + x;

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
				uint16 src_x = header.area_offset_x + (data & 0x1ff);
				uint16 src_y = header.area_offset_y + (y - (data >> 9 & 0x1f));
				memcpy(_surface->getBasePtr(dst_x, dst_y), _surface->getBasePtr(src_x, src_y), size);
				_dirtyRects.push_back(Common::Rect(dst_x, dst_y, dst_x + size, dst_y + 1));
			}
				break;

			case 3:
			{
				data = control << 16 | *src << 8 | *(src + 1);
				src += 2;
				size = data & 0x1f;
				uint16 src_x = header.area_offset_x + (data >> 5 & 0x3ff);
				uint16 src_y = header.area_offset_y + (y - (data >> 15 & 0x7f));
				memcpy(_surface->getBasePtr(dst_x, dst_y), _surface->getBasePtr(src_x, src_y), size);
				_dirtyRects.push_back(Common::Rect(dst_x, dst_y, dst_x + size, dst_y + 1));
			}
			}

			x += size;
		}
	}
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

}
