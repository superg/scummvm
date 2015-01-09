/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef VIDEO_FLICDECODER_H
#define VIDEO_FLICDECODER_H

#include "common/endian.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/stream.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace Video {

/**
 * Decoder for FLIC videos.
 *
 * Video decoder used in engines:
 *  - tucker
 *  - gag
 */
class FlicDecoder : public VideoDecoder {
public:
	FlicDecoder();
	virtual ~FlicDecoder();

	virtual bool loadStream(Common::SeekableReadStream *stream);
	virtual void close();

	const Common::List<Common::Rect> &getDirtyRects() const;
	void clearDirtyRects();
	void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

protected:
#include "common/pack-start.h"
	struct FlicHeader {
		uint32 size;            // size of flic including this header
		uint16 type;            // either _TYPE_FLI or _TYPE_FLC below
		uint16 frames;          // number of frames in flic
		uint16 width;           // flic width in pixels
		uint16 height;          // flic height in pixels
		uint16 depth;           // bits per pixel (always 8 now)
		uint16 flags;           // _FLI_FINISHED or _FLI_LOOPED ideally
		uint32 delay;           // delay between frames
		uint16 reserved1;       // set to zero
		uint32 created;         // date of flic creation (FLC only)
		uint32 creator;         // serial # of flic creator (FLC only)
		uint32 updated;         // date of flic update (FLC only)
		uint32 updater;         // serial # of flic updater (FLC only)
		uint16 aspect_dx;       // width of square rectangle (FLC only)
		uint16 aspect_dy;       // height of square rectangle (FLC only)
		 uint8 reserved2[38];   // set to zero
		uint32 oframe1;         // offset to frame 1 (FLC only)
		uint32 oframe2;         // offset to frame 2 (FLC only)
		 uint8 reserved3[40];   // set to zero

		void fix() {
			le2sys(size);
			le2sys(type);
			le2sys(frames);
			le2sys(width);
			le2sys(height);
			le2sys(depth);
			le2sys(delay);
			le2sys(created);
			le2sys(creator);
			le2sys(updated);
			le2sys(updater);
			le2sys(aspect_dx);
			le2sys(aspect_dy);
			le2sys(oframe1);
			le2sys(oframe2);
		}
	};

	struct FrameHeader {
		uint32 size;          // size of frame including header
		uint16 type;          // always m_TYPE_PREFIX or m_TYPE_FRAME
		uint16 chunks;        // number of chunks in frame
		uint16 delay;         // EXTENDED: delay override
		uint16 reserved;      // EXTENDED: always 0
		uint16 width;         // EXTENDED: width override
		uint16 height;        // EXTENDED: height override

		void fix() {
			le2sys(size);
			le2sys(type);
			le2sys(chunks);
			le2sys(delay);
			le2sys(width);
			le2sys(height);
		}
	};

	struct ChunkHeader {
		uint32 size;   // size of chunk including header
		uint16 type;   // value from ChunkTypes below

		void fix() {
			le2sys(size);
			le2sys(type);
		}
	};
#include "common/pack-end.h"

	class FlicVideoTrack : public VideoTrack {
	public:
		FlicVideoTrack(const FlicHeader &a_flic_header, Common::SeekableReadStream *stream);
		virtual ~FlicVideoTrack();

		// inherited from Track
		virtual bool isRewindable() const;
		virtual bool rewind();

		// inherited from VideoTrack
		virtual uint16 getWidth() const;
		virtual uint16 getHeight() const;
		virtual Graphics::PixelFormat getPixelFormat() const;
		virtual int getCurFrame() const;
		virtual int getFrameCount() const;
		virtual uint32 getNextFrameStartTime() const;
		virtual const Graphics::Surface *decodeNextFrame();
		virtual const byte *getPalette() const;
		virtual bool hasDirtyPalette() const;

		const Common::List<Common::Rect> &getDirtyRects() const;
		void clearDirtyRects();
		void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

		void reallocateSurface(uint16 a_width, uint16 a_height);
		bool loop();
		void setDelay(uint32 delay);
		void updateFrame();

		void decodeColor256(const uint8 *a_data);
		void decodeBlack(const uint8 *a_data);
		void decodeLiteral(const uint8 *a_data);
		void decodeByteRun(const uint8 *a_data);
		void decodeDeltaFLC(const uint8 *a_data);

	protected:
		Graphics::Surface *_surface;
		Common::List<Common::Rect> _dirtyRects;

	private:
		Common::SeekableReadStream *_stream;
		uint32 _frameCount;
		uint32 _startFrameDelay;
		uint16 _offsetFrame[2];

		byte *_palette;
		mutable bool _dirtyPalette;

		int _curFrame;
		uint32 _nextFrameStartTime;
		uint32 _frameDelay;

		void reset(uint frame);
	};

	template<typename T>
	bool init(Common::SeekableReadStream *stream) {
		close();
		_fileStream = stream;

		FlicHeader flic_header;
		bool success = readHeader(flic_header, _fileStream);
		if (success) {
			addTrack(new T(flic_header, _fileStream));
		}

		return success;
	}

	bool readHeader(FlicHeader &flic_header, Common::SeekableReadStream *stream);

private:
	enum DeltaOpcodeType {
		DOT_PACKETCOUNT,
		DOT_UNDEFINED,
		DOT_LASTPIXEL,
		DOT_LINESKIPCOUNT
	};

	enum ChunkType {
		COLOR_256    =    4,   // 256 level color pallette info (FLC only)
		DELTA_FLC    =    7,   // word-oriented delta compression (FLC only)
		COLOR_64     =   11,   // 64 level color pallette info
		DELTA_FLI    =   12,   // byte-oriented delta compression
		BLACK        =   13,   // whole frame is color 0
		BYTE_RUN     =   15,   // byte run-length compression
		LITERAL      =   16,   // uncompressed pixels
		PSTAMP       =   18    // "Postage stamp" chunk (FLC only)
	};

	// FlicHeader::type
	static const uint16 _TYPE_FLI = 0xAF11;   // 320x200 .FLI type ID
	static const uint16 _TYPE_FLC = 0xAF12;   // variable rez .FLC type ID

	// FlicHeader::flags
	static const uint16 _FLI_FINISHED = 0x0001;
	static const uint16 _FLI_LOOPED   = 0x0002;

	// FrameHeader::type
	static const uint16 _TYPE_PREFIX = 0xF100;
	static const uint16 _TYPE_FRAME  = 0xF1FA;

	static const uint _FLC_PALETTE_SIZE = 256;
	static const uint _FLC_PALETTE_ENTRY_SIZE = 3;

	Common::SeekableReadStream *_fileStream;

	const FlicVideoTrack *getVideoTrack() const;
	FlicVideoTrack *getVideoTrack();
	virtual void readNextPacket();
	void decodeFrame(const FrameHeader &a_frame_header, const uint8 *a_data);
	virtual void decodeExtended(uint16 a_type, const uint8 *a_data);
};

}

#endif
