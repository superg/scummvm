#ifndef GAG_FLIC_DECODER_H__
#define GAG_FLIC_DECODER_H__



#include "video/flic_decoder.h"



namespace Gag
{

class GagFlicDecoder : public Video::FlicDecoder
{
public:
	GagFlicDecoder();
	virtual ~GagFlicDecoder();

	virtual bool loadStream(Common::SeekableReadStream *stream);

private:
	class GagFlicVideoTrack : public FlicVideoTrack
	{
	public:
		GagFlicVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height);
		virtual ~GagFlicVideoTrack();

	private:
#include "common/pack-start.h"
		// GAG MVZ chunk, chunk id's are 5 and 8, custom RLE-encoded graphical frame
		struct MvzFrameHeader
		{
			uint16 area_width;
			uint16 area_height;
			uint16 area_offset_x;
			uint16 area_offset_y;
			uint16 width;
			uint16 height;
			uint16 unknown1;
			uint16 unknown2;

			void Fix();
		};
#include "common/pack-end.h"

		enum GagSubchunkType
		{
			GAG_MVZ5     =    5,   // custom GAG chunk, RLE-like
			GAG_MVZ8     =    8,   // similar to GAG_MVZ5, but with pixels skip command
			GAG_SND_PCM1 =  512,   // GAG RAW PCM stream, always 256000 bytes
			GAG_SND_PCM2 =  768,   // GAG RAW PCM stream, always 128000 bytes
			GAG_SND_STOP = 1024,   // GAG stop sound command, no data here
			GAG_SND_CTRL = 1536    // GAG start sound command, includes WAV header
		};

		virtual void decodeExtended(uint16 subchunkType, uint8 *data, uint32 dataSize);
		void DecodeGagVideoMVZ(const uint8 *a_data, uint32 a_data_size, bool a_mvz8);
	};
};

}



#endif
