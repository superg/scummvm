/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "charset.h"
#include "object.h"
#include "resource.h"
#include "scumm.h"
#include "verbs.h"

void Scumm::checkV2Inventory(int x, int y) {
	int object = 0;

	if ((y < virtscr[2].topline + 34) || !(_mouseButStat & MBS_LEFT_CLICK)) 
		return;

	object = ((y - virtscr[2].topline - 34) / 8) * 2;
	if (x > 150)
		object++;

	object = findInventory(_scummVars[VAR_EGO], object+1);
	if (object > 0) {
		_scummVars[35] = object;
		runScript(4, 0, 0, 0);
	}
}

void Scumm::redrawV2Inventory() {
	int i, items = 0, curInventoryCount = 0;
	bool alternate = false;
	int max_inv = getInventoryCount(_scummVars[VAR_EGO]);

	if (!(_userState & 64))
		return;

	if (curInventoryCount > max_inv)
		curInventoryCount = max_inv;

	_string[1].charset = 1;
	_string[1].color = 5;

	items = 0;
	for (i = curInventoryCount + 1; i <= max_inv; i++) {
		int obj = findInventory(_scummVars[VAR_EGO], i);
		if ((obj == 0) || (items == 4)) break;
		
		_string[1].ypos = virtscr[2].topline + 34 + (8*(items / 2));
		if (alternate)
			_string[1].xpos = 200;
		else
			_string[1].xpos = 0;

		_messagePtr = getObjOrActorName(obj);
		assert(_messagePtr);
		drawString(1);
		items++;

		alternate = !alternate;
	}

	if (curInventoryCount > 0) { // Draw Up Arrow
		_string[1].xpos = 145;
		_string[1].ypos = virtscr[2].topline + 32;
		_messagePtr = (const byte *)"U";
		drawString(1);
	}

	if (items == 4) {     // Draw Down Arrow
		_string[1].xpos = 145;
		_string[1].ypos = virtscr[2].topline + 47;
		_messagePtr = (const byte *)"D";
		drawString(1);
	}
}

void Scumm::redrawVerbs() {
	int i;
	int verb = (_cursor.state > 0 ? checkMouseOver(_mouse.x, _mouse.y) : 0);
	for (i = _maxVerbs-1; i >= 0; i--) {
		if (i == verb && _verbs[verb].hicolor)
			drawVerb(i, 1);
		else
			drawVerb(i, 0);
	}
	_verbMouseOver = verb;
}

void Scumm::checkExecVerbs() {
	int i, over;
	VerbSlot *vs;

	if (_userPut <= 0 || _mouseButStat == 0)
		return;

	if (_mouseButStat < MBS_MAX_KEY) {
		/* Check keypresses */
		vs = &_verbs[1];
		for (i = 1; i < _maxVerbs; i++, vs++) {
			if (vs->verbid && vs->saveid == 0 && vs->curmode == 1) {
				if (_mouseButStat == vs->key) {
					runInputScript(1, vs->verbid, 1);
					return;
				}
			}
		}
		runInputScript(4, _mouseButStat, 1);
	} else if (_mouseButStat & MBS_MOUSE_MASK) {
		byte code = _mouseButStat & MBS_LEFT_CLICK ? 1 : 2;
		if (_mouse.y >= virtscr[0].topline && _mouse.y < virtscr[0].topline + virtscr[0].height) {
			over = checkMouseOver(_mouse.x, _mouse.y);
			if (over != 0) {
				runInputScript(1, _verbs[over].verbid, code);
				return;
			}
			runInputScript(2, 0, code);
		} else {
			over = checkMouseOver(_mouse.x, _mouse.y);

			// FIXME For the future: Indy3 and under inv scrolling
			/*
			   if (over >= 31 && over <= 36) 
			   over += _inventoryOffset;
			 */
			runInputScript(1, over != 0 ? _verbs[over].verbid : 0, code);
		}
	}
}

void Scumm::verbMouseOver(int verb) {
	if (_verbMouseOver == verb)
		return;

	if (_verbs[_verbMouseOver].type != kImageVerbType) {
		drawVerb(_verbMouseOver, 0);
		_verbMouseOver = verb;
	}

	if (_verbs[verb].type != kImageVerbType && _verbs[verb].hicolor) {
		drawVerb(verb, 1);
		_verbMouseOver = verb;
	}
}

int Scumm::checkMouseOver(int x, int y) {
	VerbSlot *vs;
	int i = _maxVerbs - 1;

	vs = &_verbs[i];
	do {
		if (vs->curmode != 1 || !vs->verbid || vs->saveid || y < vs->y || y >= vs->bottom)
			continue;
		if (vs->center) {
			if (x < -(vs->right - vs->x - vs->x) || x >= vs->right)
				continue;
		} else {
			if (x < vs->x || x >= vs->right)
				continue;
		}

		return i;
	} while (--vs, --i);

	if (_features & GF_AFTER_V2)
		checkV2Inventory(x, y);

	return 0;
}

void Scumm::drawVerb(int verb, int mode) {
	VerbSlot *vs;
	bool tmp;

	if (!verb)
		return;

	vs = &_verbs[verb];

	if (!vs->saveid && vs->curmode && vs->verbid) {
		if (vs->type == kImageVerbType) {
			drawVerbBitmap(verb, vs->x, vs->y);
			return;
		}
		
		restoreVerbBG(verb);

		_string[4].charset = vs->charset_nr;
		_string[4].xpos = vs->x;
		_string[4].ypos = vs->y;
		_string[4].right = _screenWidth - 1;
		_string[4].center = vs->center;

		if (vs->curmode == 2)
			_string[4].color = vs->dimcolor;
		else if (mode && vs->hicolor)
			_string[4].color = vs->hicolor;
		else
			_string[4].color = vs->color;

		// FIXME For the future: Indy3 and under inv scrolling
		/*
		   if (verb >= 31 && verb <= 36) 
		   verb += _inventoryOffset;
		 */

		_messagePtr = getResourceAddress(rtVerb, verb);
		if (!_messagePtr)
			return;
		assert(_messagePtr);

		if ((_features & GF_AFTER_V8) && (_messagePtr[0] == '/')) {
			char pointer[20];
			int i, j;

			translateText(_messagePtr, _transText);

			for (i = 0, j = 0; (_messagePtr[i] != '/' || j == 0) && j < 19; i++) {
				if (_messagePtr[i] != '/')
					pointer[j++] = _messagePtr[i];
			}
			pointer[j] = 0;
			_messagePtr = _transText;
		}

		tmp = _charset->_center;
		_charset->_center = 0;
		drawString(4);
		_charset->_center = tmp;

		vs->right = _charset->_str.right;
		vs->bottom = _charset->_str.bottom;
		vs->old = _charset->_str;
		_charset->_str.left = _charset->_str.right;
	} else {
		restoreVerbBG(verb);
	}
}

void Scumm::restoreVerbBG(int verb) {
	VerbSlot *vs;

	vs = &_verbs[verb];

	if (vs->old.left != -1) {
		restoreBG(vs->old, vs->bkcolor);
		vs->old.left = -1;
	}
}

void Scumm::drawVerbBitmap(int verb, int x, int y) {
	VirtScreen *vs;
	VerbSlot *vst;
	byte twobufs, *imptr = 0;
	int ydiff, xstrip;
	int imgw, imgh;
	int i, tmp;
	byte *obim;
	ImageHeader *imhd;
	uint32 size;

	if ((vs = findVirtScreen(y)) == NULL)
		return;

	gdi.disableZBuffer();

	twobufs = vs->alloctwobuffers;
	vs->alloctwobuffers = 0;

	xstrip = x >> 3;
	ydiff = y - vs->topline;

	obim = getResourceAddress(rtVerb, verb);
	assert(obim);
	if (_features & GF_OLD_BUNDLE) {
		imgw = obim[0];
		imgh = obim[1] >> 3;
		imptr = obim + 2;
	} else if (_features & GF_SMALL_HEADER) {
		size = READ_LE_UINT32(obim);

		imgw = (*(obim + size + 11));
		imgh = (*(obim + size + 17)) >> 3;
		imptr = (obim + 8);
	} else {
		imhd = (ImageHeader *)findResourceData(MKID('IMHD'), obim);
		if (_features & GF_AFTER_V7) {
			imgw = READ_LE_UINT16(&imhd->v7.width) >> 3;
			imgh = READ_LE_UINT16(&imhd->v7.height) >> 3;
		} else {
			imgw = READ_LE_UINT16(&imhd->old.width) >> 3;
			imgh = READ_LE_UINT16(&imhd->old.height) >> 3;
		}

		if (_features & GF_AFTER_V8) {
			warning("drawVerbBitmap(%d, %d, %d)", verb, x, y);
			imptr = findResource(MKID('IMAG'), obim);
			assert(imptr);
			imptr = findResource(MKID('WRAP'), imptr);
			assert(imptr);
			imptr = findResource(MKID('OFFS'), imptr);
			assert(imptr);
			// Get the address of the second SMAP (corresponds to IM01)
			imptr += READ_LE_UINT32(imptr + 12);
		} else
			imptr = findResource(MKID('IM01'), obim);
		if (!imptr)
			error("No image for verb %d", verb);
	}
	assert(imptr);
	for (i = 0; i < imgw; i++) {
		tmp = xstrip + i;
		if (tmp < gdi._numStrips)
			gdi.drawBitmap(imptr, vs, tmp, ydiff, imgw * 8, imgh * 8, i, 1, Gdi::dbAllowMaskOr);
	}

	vst = &_verbs[verb];
	vst->right = vst->x + imgw * 8;
	vst->bottom = vst->y + imgh * 8;
	vst->old.left = vst->x;
	vst->old.right = vst->right;
	vst->old.top = vst->y;
	vst->old.bottom = vst->bottom;

	gdi.enableZBuffer();

	vs->alloctwobuffers = twobufs;
}

int Scumm::getVerbSlot(int id, int mode) {
	int i;
	for (i = 1; i < _maxVerbs; i++) {
		if (_verbs[i].verbid == id && _verbs[i].saveid == mode) {
			return i;
		}
	}
	return 0;
}

void Scumm::killVerb(int slot) {
	VerbSlot *vs;

	if (slot == 0)
		return;

	vs = &_verbs[slot];
	vs->verbid = 0;
	vs->curmode = 0;

	nukeResource(rtVerb, slot);

	if (vs->saveid == 0) {
		drawVerb(slot, 0);
		verbMouseOver(0);
	}
	vs->saveid = 0;
}

void Scumm::setVerbObject(uint room, uint object, uint verb) {
	byte *obimptr;
	byte *obcdptr;
	uint32 size, size2;
	FindObjectInRoom foir;
	int i;

	if (whereIsObject(object) == WIO_FLOBJECT)
		error("Can't grab verb image from flobject");

	if (_features & GF_OLD_BUNDLE) {
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr == object) {
				findObjectInRoom(&foir, foImageHeader, object, room);
				size = READ_LE_UINT16(foir.obim);
				byte *ptr = createResource(rtVerb, verb, size + 2);
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				ptr[0] = *(obcdptr + 9);	// Width
				ptr[1] = *(obcdptr + 15);	// Height
				memcpy(ptr + 2, foir.obim, size);
				return;
			}
		}
	} else if (_features & GF_SMALL_HEADER) {
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr == object) {
				// FIXME - the only thing we need from the OBCD is the image size!
				// So we could use almost the same code (save for offsets)
				// as in the GF_OLD_BUNDLE code. But of course that would break save games
				// unless we insert special conversion code... <sigh>
				findObjectInRoom(&foir, foImageHeader, object, room);
				size = READ_LE_UINT32(foir.obim);
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				size2 = READ_LE_UINT32(obcdptr);
				createResource(rtVerb, verb, size + size2);
				obimptr = getResourceAddress(rtRoom, room) - foir.roomptr + foir.obim;
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				memcpy(getResourceAddress(rtVerb, verb), obimptr, size);
				memcpy(getResourceAddress(rtVerb, verb) + size, obcdptr, size2);
				return;
			}
		}
	} else {
		findObjectInRoom(&foir, foImageHeader, object, room);
		size = READ_BE_UINT32_UNALIGNED(foir.obim + 4);
		createResource(rtVerb, verb, size);
		obimptr = getResourceAddress(rtRoom, room) - foir.roomptr + foir.obim;
		memcpy(getResourceAddress(rtVerb, verb), obimptr, size);
	}
}
