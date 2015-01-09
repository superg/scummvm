#ifndef GAG_FLIC_DECODER_H__
#define GAG_FLIC_DECODER_H__



#include "audio/audiostream.h"
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

		void fix()
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
		uint16 bits_per_sample;  // Bits per sample

		void fix()
		{
			wave_id = FROM_BE_32(wave_id);
			chunk_id = FROM_BE_32(chunk_id);
			chunk_size = FROM_LE_32(chunk_size);
			wave_type = FROM_LE_16(wave_type);
			channels_count = FROM_LE_16(channels_count);
			sample_rate = FROM_LE_32(sample_rate);
			data_rate = FROM_LE_32(data_rate);
			block_align = FROM_LE_16(block_align);
			bits_per_sample = FROM_LE_16(bits_per_sample);
		}
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

	// All MOV and MVZ files contain audio frames
	// Audio stream format:
	// <GAG_SND_CTRL>
	// <GAG_SND_PCM1>
	// [GAG_SND_PCM2]
	// ...
	// <GAG_SND_STOP>

	class GagFlicVideoTrack : public FlicVideoTrack
	{
	public:
		GagFlicVideoTrack(const FlicHeader &a_flic_header, Common::SeekableReadStream *stream);
		virtual ~GagFlicVideoTrack();

		void DecodeGagVideoMVZ(const MvzFrameHeader &a_header, const uint8 *a_data, bool a_mvz8);
	};

	class GagFlicAudioTrack : public AudioTrack
	{
	public:
		GagFlicAudioTrack(const WavFmtHeader &a_wav_fmt_header);
		virtual ~GagFlicAudioTrack();

		bool endOfTrack() const;
		virtual Audio::AudioStream *getAudioStream() const;
		void queueBuffer(const byte *a_buffer, uint32 a_size);
		void setEndOfTrack();

	private:
		bool _bps16;
		bool _endOfTrack;
		Audio::QueuingAudioStream *_audStream;
	};

	GagFlicAudioTrack *_audioTrack;

	const GagFlicVideoTrack *getVideoTrack() const;
	GagFlicVideoTrack *getVideoTrack();
	virtual void decodeExtended(uint16 a_type, const uint8 *a_data);
};

}



#endif
