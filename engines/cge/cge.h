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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#ifndef CGE_H
#define CGE_H

#include "cge/general.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str.h"
#include "common/rect.h"
#include "engines/engine.h"
#include "gui/debugger.h"
#include "graphics/surface.h"
#include "engines/advancedDetector.h"
#include "cge/console.h"
#include "cge/bitmap.h"
#include "cge/sound.h"

namespace CGE {

class Console;
class Sprite;

#define kSavegameVersion 2
#define kSavegameStrSize 11
#define kPocketX    174
#define kPocketY    176
#define kPocketDX   18
#define kPocketDY   22
#define kPocketNX   8
#define kPocketNY   1
#define kPocketSX   8
#define kPocketSY   3
#define kCaveDx     9
#define kCaveDy     10
#define kCaveNx     8
#define kCaveNy     3
#define kCaveMax    kCaveNx * kCaveNy


// our engine debug channels
enum {
	kCGEDebugBitmap = 1 << 0,
	kCGEDebugFile = 1 << 1,
	kCGEDebugEngine = 1 << 2
};

enum SnList {
	kNear, kTake
};

enum CallbackType {
	kNullCB = 0, kQGame, kMiniStep, kXCave, kSndSetVolume
};

struct SavegameHeader {
	uint8 version;
	Common::String saveName;
	Graphics::Surface *thumbnail;
	int saveYear, saveMonth, saveDay;
	int saveHour, saveMinutes;
	int totalFrames;
};

extern const char *savegameStr;

struct Bar {
	uint8 _horz;
	uint8 _vert;
};

class CGEEngine : public Engine {
private:
	uint32 _lastFrame, _lastTick;
	void tick();
	void syncHeader(Common::Serializer &s);
	static void writeSavegameHeader(Common::OutSaveFile *out, SavegameHeader &header);
	void syncGame(Common::SeekableReadStream *readStream, Common::WriteStream *writeStream, bool tiny = false);
	bool savegameExists(int slotNumber);
	Common::String generateSaveName(int slot);
public:
	CGEEngine(OSystem *syst, const ADGameDescription *gameDescription);
	~CGEEngine();
	virtual bool hasFeature(EngineFeature f) const;
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const Common::String &desc);

	static const int _maxCaveArr[5];

	const   ADGameDescription *_gameDescription;
	int    _startupMode;
	int    _demoText;
	int    _oldLev;
	int    _pocPtr;
	bool   _music;
	int    _pocref[kPocketNX];
	uint8  _volume[2];
	int    _maxCave;
	bool   _flag[4];
	bool   _dark;
	bool   _game;
	bool   _finis;
	int    _now;
	int    _lev;
	int    _mode;
	int    _soundOk;
	int    _gameCase2Cpt;
	int    _offUseCount;

	Sprite *_sprTv;
	Sprite *_sprK1;
	Sprite *_sprK2;
	Sprite *_sprK3;

	Common::Point _heroXY[kCaveMax];
	Bar _barriers[kCaveMax];

	Common::RandomSource _randomSource;
	MusicPlayer _midiPlayer;
	BitmapPtr *_miniShp;
	BitmapPtr *_miniShpList;
	int        _startGameSlot;

	virtual Common::Error run();
	GUI::Debugger *getDebugger() {
		return _console;
	}

	void cge_main();
	void switchCave(int cav);
	void startCountDown();
	void quit();
	void resetQSwitch();
	void optionTouch(int opt, uint16 mask);
	void resetGame();
	bool loadGame(int slotNumber, SavegameHeader *header = NULL, bool tiny = false);
	void setMapBrick(int x, int z);
	void switchMapping();
	void loadSprite(const char *fname, int ref, int cav, int col, int row, int pos);
	void loadScript(const char *fname);
	void loadUser();
	void runGame();
	bool showTitle(const char *name);
	void movie(const char *ext);
	void inf(const char *text);
	void selectSound();
	void dummy() {}
	void NONE();
	void SB();
	void caveDown();
	void caveUp();
	void xCave();
	void qGame();
	void SBM();
	void GUS();
	void GUSM();
	void MIDI();
	void AUTO();
	void setPortD();
	void setPortM();
	void setIRQ();
	void setDMA();
	void mainLoop();
	void handleFrame();
	void saveGame(int slotNumber, const Common::String &desc);
	static bool readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header);
	void switchMusic();
	void selectPocket(int n);
	void expandSprite(Sprite *spr);
	void contractSprite(Sprite *spr);
	int  findPocket(Sprite *spr);
	void feedSnail(Sprite *spr, SnList snq);
	void pocFul();
	void hide1(Sprite *spr);
	void loadMapping();
	void saveSound();
	void heroCover(int cvr);
	void trouble(int seq, int text);
	void offUse();
	void tooFar();
	void loadHeroXY();
	void keyClick();
	void switchColorMode();
	void killSprite();
	void switchDebug();
	void miniStep(int stp);
	void postMiniStep(int stp);
	void showBak(int ref);
	void initCaveValues();

	void snBackPt(Sprite *spr, int stp);
	void snHBarrier(const int cave, const int barX);
	void snVBarrier(const int cave, const int barY);
	void snCover(Sprite *spr, int xref);
	void snFlag(int indx, bool v);
	void snFlash(bool on);
	void snGame(Sprite *spr, int num);
	void snGhost(Bitmap *bmp);
	void snGive(Sprite *spr, int stp);
	void snHide(Sprite *spr, int val);
	void snKeep(Sprite *spr, int stp);
	void snKill(Sprite *spr);
	void snLevel(Sprite *spr, int lev);
	void snLight(bool in);
	void snMouse(bool on);
	void snNNext(Sprite *spr, int p);
	void snPort(Sprite *spr, int port);
	void snReach(Sprite *spr, int mode);
	void snRelZ(Sprite *spr, int z);
	void snRNNext(Sprite *spr, int p);
	void snRTNext(Sprite *spr, int p);
	void snSend(Sprite *spr, int val);
	void snRelX(Sprite *spr, int x);
	void snRelY(Sprite *spr, int y);
	void snRmNear(Sprite *spr);
	void snRmTake(Sprite *spr);
	void snRSeq(Sprite *spr, int val);
	void snSeq(Sprite *spr, int val);
	void snSetRef(Sprite *spr, int nr);
	void snSetX(Sprite *spr, int x);
	void snSetX0(int cav, int x0);
	void snSetXY(Sprite *spr, uint16 xy);
	void snSetY(Sprite *spr, int y);
	void snSetY0(int cav, int y0);
	void snSetZ(Sprite *spr, int z);
	void snSlave(Sprite *spr, int ref);
	void snSound(Sprite *spr, int wav);
	void snSwap(Sprite *spr, int xref);
	void snTNext(Sprite *spr, int p);
	void snTrans(Sprite *spr, int trans);
	void snUncover(Sprite *spr, Sprite *xspr);
	void snWalk(Sprite *spr, int x, int y);
	void snZTrim(Sprite *spr);
protected:
	int _recentStep;

private:
	CGEConsole *_console;
	void init();
	void deinit();
};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(CGEEngine *vm) {}
	virtual ~Console() {}
};

} // End of namespace CGE

#endif
