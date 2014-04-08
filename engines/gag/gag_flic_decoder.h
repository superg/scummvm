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
		GagFlicVideoTrack(const FlicHeader &a_flic_header, Common::SeekableReadStream *stream);
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

		struct WavFmtHeader
		{
			uint32 wave_id;          // WAVE ID: "WAVE"
			uint32 chunk_id;         // Chunk ID: "fmt "
			uint32 chunk_size;       // Chunk size: 16 or 18 or 40
			uint16 wave_type;        // Format code
			uint16 channels_count;   // Number of interleaved channels
			uint32 sample_rate;      // Sampling rate (blocks per second)
			uint32 data_rate;        // Data rate
			uint16 block_align;      // Data block size (bytes)
			uint16 bits_per_sec;     // Bits per sample

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

		virtual void decodeExtended(uint16 a_type, const uint8 *a_data);
		void DecodeGagVideoMVZ(const MvzFrameHeader &a_header, const uint8 *a_data, bool a_mvz8);
	};

	class GagFlicAudioTrack : public AudioTrack
	{
	public:
		GagFlicAudioTrack();
		virtual ~GagFlicAudioTrack();

		virtual Audio::AudioStream *getAudioStream() const;
	};

	static const uint _TRACK_AUDIO;
};

}



#endif
