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

#include "common/endian.h"
#include "common/rect.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "video/flic_decoder.h"

namespace Video
{

FlicDecoder::FlicDecoder()
	: _fileStream(NULL)
{
	;
}


FlicDecoder::~FlicDecoder() {
	close();
}


bool FlicDecoder::loadStream(Common::SeekableReadStream *stream) {
	return init<FlicVideoTrack>(stream);
}


void FlicDecoder::close() {
	VideoDecoder::close();

	if (_fileStream != NULL) {
		delete _fileStream;
		_fileStream = NULL;
	}
}


const Common::List<Common::Rect> &FlicDecoder::getDirtyRects() const {
	return getVideoTrack()->getDirtyRects();
}


void FlicDecoder::clearDirtyRects() {
	getVideoTrack()->clearDirtyRects();
}


void FlicDecoder::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	getVideoTrack()->copyDirtyRectsToBuffer(dst, pitch);
}


bool FlicDecoder::readHeader(FlicHeader &flic_header, Common::SeekableReadStream *stream) {
	if (_fileStream->read(&flic_header, sizeof(flic_header)) != sizeof(flic_header)) {
		warning("FlicDecoder::loadStream(): incomplete FLC file");
		return false;
	}
	flic_header.fix();

	if (flic_header.type != _TYPE_FLC) {
		warning("FlicDecoder::loadStream(): attempted to load non-FLC data (type = 0x%04X)", flic_header.type);
		return false;
	}

	if (flic_header.depth != 8) {
		warning("FlicDecoder::loadStream(): attempted to load an FLC with a palette of color depth %d. Only 8-bit color palettes are supported", flic_header.depth);
		return false;
	}

	return true;
}


const FlicDecoder::FlicVideoTrack *FlicDecoder::getVideoTrack() const
{
	return static_cast<FlicVideoTrack *>(const_cast<Track *>(getTrack(0)));
}


FlicDecoder::FlicVideoTrack *FlicDecoder::getVideoTrack()
{
	return static_cast<FlicVideoTrack *>(getTrack(0));
}


void FlicDecoder::readNextPacket() {
	FrameHeader header;
	if (_fileStream->read(&header, sizeof(header)) != sizeof(header))
		error("FlicDecoder::readNextPacket(): FLC file is corrupted");
	header.fix();

	if (header.type != _TYPE_FRAME)
		error("FlicDecoder::readNextPacket(): unknown frame header type (type = 0x%02X)", header.type);

	// extended attributes
	if (header.delay)
		getVideoTrack()->setDelay(header.delay);
	if (header.width || header.height) {
		if (!header.width)
			header.width = getVideoTrack()->getWidth();
		if (!header.height)
			header.height = getVideoTrack()->getHeight();

		getVideoTrack()->reallocateSurface(header.width, header.height);
	}

	// exclude header size
	uint32 data_size = header.size - sizeof(header);
	if (data_size) {
		uint8 *data = new uint8[data_size];
		if (_fileStream->read(data, data_size) != data_size)
			error("FlicDecoder::readNextPacket(): FLC file is corrupted");

		decodeFrame(header, data);

		delete [] data;
	}

	getVideoTrack()->updateFrame();
}


void FlicDecoder::decodeFrame(const FrameHeader &a_frame_header, const uint8 *a_data) {
	for (uint16 i = 0; i < a_frame_header.chunks; ++i) {
		ChunkHeader chunk = *(const ChunkHeader *)a_data;
		chunk.fix();
		a_data += sizeof(ChunkHeader);

		switch (chunk.type) {
		case COLOR_256:
			getVideoTrack()->decodeColor256(a_data);
			break;

		case DELTA_FLC:
			getVideoTrack()->decodeDeltaFLC(a_data);
			break;

		case COLOR_64:
		case DELTA_FLI:
			error("FlicDecoder::decodeFrame(): unimplemented chunk type (type = 0x%02X)", chunk.type);
			break;

		case BLACK:
			getVideoTrack()->decodeBlack(a_data);
			break;

		case BYTE_RUN:
			getVideoTrack()->decodeByteRun(a_data);
			break;

		case LITERAL:
			getVideoTrack()->decodeLiteral(a_data);
			break;

		case PSTAMP:
			//TODO: implement thumb image extraction?
			break;

		default:
			decodeExtended(chunk.type, a_data);
			break;
		}

		a_data += chunk.type == LITERAL ? getWidth() * getHeight() : chunk.size - sizeof(ChunkHeader);
	}
}


void FlicDecoder::decodeExtended(uint16 a_type, const uint8 *) {
	error("FlicDecoder::decodeExtended(): unknown chunk type (type = 0x%02X)", a_type);
}


FlicDecoder::FlicVideoTrack::FlicVideoTrack(const FlicHeader &a_flic_header, Common::SeekableReadStream *stream)
	: _surface(NULL)
	, _stream(stream)
	, _frameCount(a_flic_header.frames)
	, _startFrameDelay(a_flic_header.delay)
	, _dirtyPalette(false)
{
	_offsetFrame[0] = a_flic_header.oframe1;
	_offsetFrame[1] = a_flic_header.oframe2;

	reallocateSurface(a_flic_header.width, a_flic_header.height);

	_palette = new byte[_FLC_PALETTE_ENTRY_SIZE * _FLC_PALETTE_SIZE];
	memset(_palette, 0, _FLC_PALETTE_ENTRY_SIZE * _FLC_PALETTE_SIZE);

	//NOTE: don't use rewind() here because object is not fully initialized yet
	reset(0);
}


FlicDecoder::FlicVideoTrack::~FlicVideoTrack() {
	delete [] _palette;

	_surface->free();
	delete _surface;
}


bool FlicDecoder::FlicVideoTrack::isRewindable() const {
	return true;
}


bool FlicDecoder::FlicVideoTrack::rewind() {
	reset(0);
	return true;
}


uint16 FlicDecoder::FlicVideoTrack::getWidth() const {
	return _surface->w;
}


uint16 FlicDecoder::FlicVideoTrack::getHeight() const {
	return _surface->h;
}


Graphics::PixelFormat FlicDecoder::FlicVideoTrack::getPixelFormat() const {
	return _surface->format;
}


int FlicDecoder::FlicVideoTrack::getCurFrame() const {
	return _curFrame;
}


int FlicDecoder::FlicVideoTrack::getFrameCount() const {
	return _frameCount;
}


uint32 FlicDecoder::FlicVideoTrack::getNextFrameStartTime() const {
	return _nextFrameStartTime;
}


const Graphics::Surface *FlicDecoder::FlicVideoTrack::decodeNextFrame() {
	return _surface;
}


const byte *FlicDecoder::FlicVideoTrack::getPalette() const {
	_dirtyPalette = false;
	return _palette;
}


bool FlicDecoder::FlicVideoTrack::hasDirtyPalette() const {
	return _dirtyPalette;
}


const Common::List<Common::Rect> &FlicDecoder::FlicVideoTrack::getDirtyRects() const {
	return _dirtyRects;
}


void FlicDecoder::FlicVideoTrack::clearDirtyRects() {
	_dirtyRects.clear();
}


void FlicDecoder::FlicVideoTrack::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	for (Common::List<Common::Rect>::const_iterator it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it)
		for (int y = it->top; y < it->bottom; ++y)
			memcpy(dst + y * pitch + it->left, _surface->getBasePtr(it->left, y), it->right - it->left);

	_dirtyRects.clear();
}


void FlicDecoder::FlicVideoTrack::reallocateSurface(uint16 a_width, uint16 a_height) {

	if (_surface != NULL) {
		_surface->free();
		delete _surface;
	}

	_surface = new Graphics::Surface;
	_surface->create(a_width, a_height, Graphics::PixelFormat::createFormatCLUT8());
}


bool FlicDecoder::FlicVideoTrack::loop() {
	reset(1);
	return true;
}


void FlicDecoder::FlicVideoTrack::setDelay(uint32 delay) {
	_frameDelay = delay;
}


void FlicDecoder::FlicVideoTrack::updateFrame() {
	++_curFrame;
	_nextFrameStartTime += _frameDelay;
}


void FlicDecoder::FlicVideoTrack::decodeColor256(const uint8 *a_data) {
	uint8 start = 0;
	uint16 opc = READ_LE_UINT16(a_data);
	a_data += sizeof(opc);
	while (opc--) {
		start += *a_data++;
		uint16 size = *a_data++;
		if (!size)
			size = _FLC_PALETTE_SIZE;

		memcpy(_palette + start * _FLC_PALETTE_ENTRY_SIZE, a_data, size * _FLC_PALETTE_ENTRY_SIZE);
		a_data += size * _FLC_PALETTE_ENTRY_SIZE;

		start += size;
	}

	_dirtyPalette = true;
}


void FlicDecoder::FlicVideoTrack::decodeDeltaFLC(const uint8 *a_data) {
	uint16 lp_count = READ_LE_UINT16(a_data);
	a_data += sizeof(lp_count);
	for (uint16 y = 0; lp_count--; ++y) {
		DeltaOpcodeType optype;
		do {
			uint16 opcode = READ_LE_UINT16(a_data);
			a_data += sizeof(opcode);

			optype = (DeltaOpcodeType)(opcode >> 14);
			switch (optype) {
			case DOT_PACKETCOUNT:
				// interpret the RLE data
				for (uint16 x = 0; opcode--;) {
					x += *a_data++;

					int16 size = (int8)*a_data++;
					size *= 2;
					if (size >= 0) {
						memcpy((byte *)_surface->getBasePtr(x, y), a_data, size);
						a_data += size;
						_dirtyRects.push_back(Common::Rect(x, y, x + size, y + 1));
					} else {
						size = -size;

						uint16 pattern = READ_UINT16(a_data);
						a_data += sizeof(pattern);
						for (int i = 0; i < size / 2; ++i)
							WRITE_UINT16((byte *)_surface->getBasePtr(x + i * 2, y), pattern);
						_dirtyRects.push_back(Common::Rect(x, y, x + size, y + 1));
					}

					x += size;
				}
				break;

			case DOT_UNDEFINED:
				break;

			case DOT_LASTPIXEL:
				*((byte *)_surface->getBasePtr(getWidth() - 1, y)) = opcode & 0xff;
				_dirtyRects.push_back(Common::Rect(getWidth() - 1, y, getWidth(), y + 1));
				break;

			case DOT_LINESKIPCOUNT:
				y += -(int16)opcode;
			}
		} while(optype != DOT_PACKETCOUNT);
	}
}


void FlicDecoder::FlicVideoTrack::decodeBlack(const uint8 *) {
	memset((byte *)_surface->getPixels(), 0, getWidth() * getHeight());

	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(0, 0, getWidth(), getHeight()));
}


void FlicDecoder::FlicVideoTrack::decodeByteRun(const uint8 *a_data) {
	for (uint16 y = 0; y < getHeight(); ++y) {
		// skip over obsolete opcount byte
		++a_data;

		for (uint16 x = 0; x < getWidth(); ) {
			int8 size = *a_data++;
			if (size >= 0)
				memset(_surface->getBasePtr(x, y), *a_data++, size);
			else {
				size = -size;
				memcpy(_surface->getBasePtr(x, y), a_data, size);
				a_data += size;
			}

			x += size;
		}
	}

	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(0, 0, getWidth(), getHeight()));
}


void FlicDecoder::FlicVideoTrack::decodeLiteral(const uint8 *a_data) {
	memcpy(_surface->getPixels(), a_data, getWidth() * getHeight());

	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(0, 0, getWidth(), getHeight()));
}


void FlicDecoder::FlicVideoTrack::reset(uint frame) {
	_curFrame = -1;
	_nextFrameStartTime = 0;
	_frameDelay = _startFrameDelay;

	_stream->seek(_offsetFrame[frame]);
}

}
