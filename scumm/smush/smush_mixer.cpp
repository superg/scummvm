/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/util.h"

#include "scumm/smush/smush_mixer.h"
#include "scumm/smush/channel.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse.h"

#include "sound/mixer.h"


namespace Scumm {

SmushMixer::SmushMixer(SoundMixer *m) :
	_mixer(m),
	_soundFrequency(22050) {
	for (int32 i = 0; i < NUM_CHANNELS; i++) {
		_channels[i].id = -1;
		_channels[i].chan = NULL;
	}
}

SmushMixer::~SmushMixer() {
	for (int32 i = 0; i < NUM_CHANNELS; i++) {
		_mixer->stopHandle(_channels[i].handle);
	}
}

SmushChannel *SmushMixer::findChannel(int32 track) {
	debugC(DEBUG_SMUSH, "SmushMixer::findChannel(%d)", track);
	for (int32 i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i].id == track)
			return _channels[i].chan;
	}
	return NULL;
}

void SmushMixer::addChannel(SmushChannel *c) {
	int32 track = c->getTrackIdentifier();
	int i;

	debugC(DEBUG_SMUSH, "SmushMixer::addChannel(%d)", track);

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i].id == track)
			warning("SmushMixer::addChannel(%d): channel already exists", track);
	}

	for (i = 0; i < NUM_CHANNELS; i++) {
		if ((_channels[i].chan == NULL || _channels[i].id == -1) && !_channels[i].handle.isActive()) {
			_channels[i].chan = c;
			_channels[i].id = track;
			return;
		}
	}

	for (i = 0; i < NUM_CHANNELS; i++) {
		warning("channel %d : %p(%d, %d)", i, (void *)_channels[i].chan, 
			_channels[i].chan ? _channels[i].chan->getTrackIdentifier() : -1, 
			_channels[i].chan ? _channels[i].chan->isTerminated() : 1);
	}

	error("SmushMixer::addChannel(%d): no channel available", track);
}

bool SmushMixer::handleFrame() {
	debugC(DEBUG_SMUSH, "SmushMixer::handleFrame()");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i].id != -1) {
			if (_channels[i].chan->isTerminated()) {
				delete _channels[i].chan;
				_channels[i].id = -1;
				_channels[i].chan = NULL;
				_channels[i].stream->finish();
				_channels[i].stream = 0;
			} else {
				int32 rate, vol, pan;
				bool stereo, is_16bit;
				void *data;

				_channels[i].chan->getParameters(rate, stereo, is_16bit, vol, pan);
				int32 size = _channels[i].chan->availableSoundData();
				byte flags = stereo ? SoundMixer::FLAG_STEREO : 0;

				if (is_16bit) {
					data = malloc(size * (stereo ? 2 : 1) * 4);
					_channels[i].chan->getSoundData((int16 *)data, size);
					size *= stereo ? 4 : 2;

					flags |= SoundMixer::FLAG_16BITS;

				} else {
					data = malloc(size * (stereo ? 2 : 1) * 2);
					_channels[i].chan->getSoundData((int8 *)data, size);
					size *= stereo ? 2 : 1;

					flags |= SoundMixer::FLAG_UNSIGNED;
				}

				if (_mixer->isReady()) {
					if (!_channels[i].handle.isActive()) {
						_channels[i].stream = makeAppendableAudioStream(rate, flags, 500000);
						_mixer->playInputStream(SoundMixer::kSFXSoundType, &_channels[i].handle, _channels[i].stream);
					}
					_mixer->setChannelVolume(_channels[i].handle, vol);
					_mixer->setChannelBalance(_channels[i].handle, pan);
					_channels[i].stream->append((byte *)data, size);
				}
				free(data);
			}
		}
	}
	return true;
}

bool SmushMixer::stop() {
	debugC(DEBUG_SMUSH, "SmushMixer::stop()");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i].id != -1) {
			delete _channels[i].chan;
			_channels[i].id = -1;
			_channels[i].chan = NULL;
			_channels[i].stream->finish();
			_channels[i].stream = 0;
		}
	}
	return true;
}

} // End of namespace Scumm
