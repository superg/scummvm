#include "audio/decoders/raw.h"
#include "gag_flic_decoder.h"



namespace Gag
{

GagFlicDecoder::GagFlicDecoder()
	: Video::FlicDecoder()
	, _audioTrack(NULL)
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


const GagFlicDecoder::GagFlicVideoTrack *GagFlicDecoder::getVideoTrack() const
{
	return static_cast<GagFlicVideoTrack *>(const_cast<Track *>(getTrack(0)));
}


GagFlicDecoder::GagFlicVideoTrack *GagFlicDecoder::getVideoTrack()
{
	return static_cast<GagFlicVideoTrack *>(getTrack(0));
}


void GagFlicDecoder::decodeExtended(uint16 a_type, const uint8 *a_data)
{
	GagSubchunkType type = (GagSubchunkType)a_type;
	switch(type)
	{
	case GAG_MVZ5:
	case GAG_MVZ8:
	{
		MvzFrameHeader header = *(const MvzFrameHeader *)a_data;
		header.fix();

		getVideoTrack()->DecodeGagVideoMVZ(header, a_data + sizeof(header), type == GAG_MVZ8);
	}
		break;

	case GAG_SND_PCM1:
//		debug("GAG_SND_PCM1");
		_audioTrack->queueBuffer(a_data, 256000);
		break;

	case GAG_SND_PCM2:
//		debug("GAG_SND_PCM2");
		_audioTrack->queueBuffer(a_data, 128000);
		break;

	case GAG_SND_STOP:
//		debug("GAG_SND_STOP");
		_audioTrack->setEndOfTrack();
		;
		break;

	case GAG_SND_CTRL:
		if(_audioTrack == NULL)
		{
			WavFmtHeader wav_fmt_header = *(const WavFmtHeader *)a_data;
			wav_fmt_header.fix();
			_audioTrack = new GagFlicAudioTrack(wav_fmt_header);
			addTrack(_audioTrack);
		}
		break;

	default:
		debug("warning: subchunk type not implemented (type: %d)", a_type);
	}
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


GagFlicDecoder::GagFlicAudioTrack::GagFlicAudioTrack(const WavFmtHeader &a_wav_fmt_header)
{
	_endOfTrack = false;
	_bps16 = a_wav_fmt_header.bits_per_sample == 16;
	_audStream = Audio::makeQueuingAudioStream(a_wav_fmt_header.sample_rate, a_wav_fmt_header.channels_count > 1);
}


GagFlicDecoder::GagFlicAudioTrack::~GagFlicAudioTrack()
{
	delete _audStream;
}


bool GagFlicDecoder::GagFlicAudioTrack::endOfTrack() const
{
	return /*AudioTrack::endOfTrack() &&*/ _endOfTrack;
}


Audio::AudioStream *GagFlicDecoder::GagFlicAudioTrack::getAudioStream() const
{
	return _audStream;
}


void GagFlicDecoder::GagFlicAudioTrack::queueBuffer(const byte *a_buffer, uint32 a_size)
{
	byte flags = Audio::FLAG_LITTLE_ENDIAN;
	if(_bps16)
		flags |= Audio::FLAG_16BITS;

	if(_audStream->isStereo())
		flags |= Audio::FLAG_STEREO;

	byte *buffer = (byte *)malloc(a_size);
	memcpy(buffer, a_buffer, a_size);
	_audStream->queueBuffer(buffer, a_size, DisposeAfterUse::YES, flags);
}


void GagFlicDecoder::GagFlicAudioTrack::setEndOfTrack()
{
	_endOfTrack = true;
}

}
