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

#ifndef SCUMM_H
#define SCUMM_H

#include "common/engine.h"
#include "common/gameDetector.h"
#include "common/file.h"
#include "common/map.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/timer.h"
#include "common/util.h"

class CharsetRenderer;
class GameDetector;
class NewGui;
class Dialog;
class ConsoleDialog;
class Scumm;
class IMuse;
class IMuseDigital;
class Actor;
class Sound;
class Bundle;
class ScummDebugger;
class Serializer;
struct FindObjectInRoom;

typedef ScummVM::Map<ScummVM::String, int> ObjectIDMap;

// Use g_scumm from error() ONLY
extern Scumm *g_scumm;

/* System Wide Constants */
enum {
	NUM_SCRIPT_SLOT = 40,
	NUM_LOCALSCRIPT = 60,
	NUM_SENTENCE = 6,
	NUM_SHADOW_PALETTE = 8,
	KEY_SET_OPTIONS = 3456 // WinCE
};

enum ObjectClass {
	kObjectClassNeverClip = 20,
	kObjectClassAlwaysClip = 21,
	kObjectClassIgnoreBoxes = 22,
	kObjectClassYFlip = 29,
	kObjectClassXFlip = 30,
	kObjectClassPlayer = 31,	// Actor is controlled by the player
	kObjectClassUntouchable = 32
};

#include "gfx.h"
#include "boxes.h"

struct MemBlkHeader {
	uint32 size;
};

struct VerbSlot;

struct ObjectData {
	uint32 OBIMoffset;
	uint32 OBCDoffset;
	int16 walk_x, walk_y;
	uint16 obj_nr;
	int16 x_pos;
	int16 y_pos;
	uint16 width;
	uint16 height;
	byte actordir;
	byte parent;
	byte parentstate;
	byte state;
	byte fl_object_index;
};

struct ScriptSlot {
	uint32 offs;
	int32 delay;
	uint16 number;
	uint16 delayFrameCount;
	bool freezeResistant, recursive;
	bool didexec;
	byte status;
	byte where;
	byte freezeCount;
	byte cutsceneOverride;
};

struct NestedScript {
	uint16 number;
	uint8 where;
	uint8 slot;
};

struct BlastText {
	int16 xpos, ypos;
	ScummVM::Rect rect;
	byte color;
	byte charset;
	bool center;
	byte text[256];
};

enum ResTypes {
	rtFirst = 1,
	rtRoom = 1,
	rtScript = 2,
	rtCostume = 3,
	rtSound = 4,
	rtInventory = 5,
	rtCharset = 6,
	rtString = 7,
	rtVerb = 8,
	rtActorName = 9,
	rtBuffer = 10,
	rtScaleTable = 11,
	rtTemp = 12,
	rtFlObject = 13,
	rtMatrix = 14,
	rtBox = 15,
	rtObjectName = 16,
	rtRoomScripts = 17,
	rtLast = 17,
	rtNumTypes = 18
};

enum {
	LIGHTMODE_dark			= 0,
	LIGHTMODE_actor_base	= 1,
	LIGHTMODE_screen		= 2,
	LIGHTMODE_flashlight	= 4,
	LIGHTMODE_actor_color	= 8
};

enum {
	MBS_LEFT_CLICK = 0x8000,
	MBS_RIGHT_CLICK = 0x4000,
	MBS_MOUSE_MASK = (MBS_LEFT_CLICK | MBS_RIGHT_CLICK),
	MBS_MAX_KEY	= 0x0200
};

enum ScummGameId {
	GID_TENTACLE = GID_SCUMM_FIRST,
	GID_MONKEY2,
	GID_INDY4,
	GID_MONKEY,
	GID_SAMNMAX,
	GID_MONKEY_EGA,
	GID_LOOM256,
	GID_ZAK256,
	GID_INDY3_256,
	GID_INDY3,
	GID_LOOM,
	GID_FT,
	GID_DIG,
	GID_MONKEY_VGA,
	GID_CMI,
	GID_MANIAC,
	GID_ZAK,
	//GID_MANIAC64,
	//GID_ZAK64,
	GID_PUTTDEMO
};

#define _maxRooms res.num[rtRoom]
#define _maxScripts res.num[rtScript]
#define _maxCostumes res.num[rtCostume]
#define _maxInventoryItems res.num[rtInventory]
#define _maxCharsets res.num[rtCharset]
#define _maxStrings res.num[rtString]
#define _maxVerbs res.num[rtVerb]
#define _maxActorNames res.num[rtActorName]
#define _maxBuffer res.num[rtBuffer]
#define _maxScaleTable res.num[rtScaleTable]
#define _maxTemp res.num[rtTemp]
#define _maxFLObject res.num[rtFlObject]
#define _maxMatrixes res.num[rtMatrix]
#define _maxBoxes res.num[rtBox]

#define _baseRooms res.address[rtRoom]
#define _baseScripts res.address[rtScript]
#define _baseInventoryItems res.address[rtInventory]
#define _baseFLObject res.address[rtFlObject]
#define _baseArrays res.address[rtString]

#define _roomFileOffsets res.roomoffs[rtRoom]

#define ARRAY_HDR_SIZE 6
struct ArrayHeader {
	int16 dim1_size;
	int16 type;
	int16 dim2_size;
	byte data[1];
};

struct SentenceTab {
	byte verb;
	byte unk2;
	uint16 objectA;
	uint16 objectB;
	uint8 freezeCount;
};

// TODO / FIXME: next time save game format changes, Fingolfin would like to
// revise StringTab. In particular, all t_* fields can be removed, making some
// code a bit cleaner & easier to understand.
struct StringTab {
	int16 t_xpos, t_ypos;
	int16 t_right;
	int16 xpos, ypos;
	int16 right;
	byte color, t_color;
	byte charset, t_charset;
	bool center, t_center;
	bool overhead, t_overhead;
	bool no_talk_anim, t_no_talk_anim;
};

enum WhereIsObject {
	WIO_NOT_FOUND = -1,
	WIO_INVENTORY = 0,
	WIO_ROOM = 1,
	WIO_GLOBAL = 2,
	WIO_LOCAL = 3,
	WIO_FLOBJECT = 4
};

enum MouseButtonStatus {
	msDown = 1,
	msClicked = 2
};

struct langIndexNode {
	char tag[9];
	int32 offset;
};

class Scumm : public Engine {
	friend class ScummDebugger;
	friend class SmushPlayer;
	void errorString(const char *buf_input, char *buf_output);
public:
	/* Put often used variables at the top.
	 * That results in a shorter form of the opcode
	 * on some architectures. */
	IMuse *_imuse;
	IMuseDigital *_imuseDigital;
	uint32 _features;
	VerbSlot *_verbs;
	ObjectData *_objs;
	ScummDebugger *_debugger;
	Bundle *_bundle;
	Timer *_timer;
	Sound *_sound;

	struct {
		byte mode[rtNumTypes];
		uint16 num[rtNumTypes];
		uint32 tags[rtNumTypes];
		const char *name[rtNumTypes];
		byte **address[rtNumTypes];
		byte *flags[rtNumTypes];
		byte *roomno[rtNumTypes];
		uint32 *roomoffs[rtNumTypes];
	} res;

	struct {
		uint32 cutScenePtr[5];
		byte cutSceneScript[5];
		int16 cutSceneData[5];
		int16 cutSceneScriptIndex;
		byte cutSceneStackPointer;
		ScriptSlot slot[NUM_SCRIPT_SLOT];
		NestedScript nest[15];
		int32 localvar[NUM_SCRIPT_SLOT][26];
	} vm;

	// Constructor / Destructor
	Scumm(GameDetector *detector, OSystem *syst);
	virtual ~Scumm();

	// Init functions
	void scummInit();
	void initScummVars();
	virtual void setupScummVars();

	// Startup functions
	void main();
	void parseCommandLine(int argc, char **argv);
	void showHelpAndExit();
	bool detectGame();
	void launch();
	void go();

	// Scumm main loop
	void mainRun();
	int scummLoop(int delta);

	// Event handling
	void parseEvents();
	void waitForTimer(int msec_delay);
	void processKbd();
	int checkKeyHit();
	void convertKeysToClicks();
	int getKeyInput();
	void clearClickedStatus();

	// Misc utility functions
	void checkRange(int max, int min, int no, const char *str);
	const char *getExeName() const { return _exe_name; }
	const char *getGameDataPath() const { return _gameDataPath; }

	// Cursor/palette
	void updateCursor();
	void animateCursor();
	void updatePalette();

	/* _insane vars */
	int _smushFrameRate;
	bool _insaneState;
	bool _videoFinished;
	bool _smushPlay;
	
	void pauseGame(bool user);
	void shutDown(int i);
	void setOptions(void);

#ifdef __PALM_OS__
	bool _quit;		// try to exit properly
#endif
	// GUI
	NewGui *_newgui;

	Dialog *_pauseDialog;
	Dialog *_optionsDialog;
	Dialog *_saveLoadDialog;
	// Debugger access this one, too...
	ConsoleDialog *_debuggerDialog;

	int runDialog(Dialog *dialog);
	void pauseDialog();
	void saveloadDialog();
	void optionsDialog();
	char displayError(bool showCancel, const char *message, ...);

protected:
	byte _fastMode;

	/* Random number generation */
	RandomSource _rnd;

public:
	/* Core variable definitions */
	byte _gameId;

	/* Core class/array definitions */
	Gdi gdi;

protected:
	Actor *_actors;	// Has _numActors elements
	
	uint16 *_inventory;
	byte *_arrays;
	uint16 *_newNames;
public:
	// VAR is a wrapper around scummVar, which attempts to include additional
	// useful information should an illegal var access be detected.
	#define VAR(x)	scummVar(x, #x, __FILE__, __LINE__)
	inline int32& scummVar(byte var, const char *varName, const char *file, int line)
	{
		if (var == 0xFF) {
			warning("Illegal access to variable %s in file %s, line %d", varName, file, line);
			// Return a fake memory location, so that at least no innocent variable
			// gets overwritten.
			static int32 fake;
			fake = 0;
			return fake;
		}
		return _scummVars[var];
	}

protected:
	int16 _varwatch;
	int32 *_scummVars;
	byte *_bitVars;

	/* Global resource tables */
	int _numVariables, _numBitVariables, _numLocalObjects;
	int _numGlobalObjects, _numArray, _numVerbs, _numFlObject;
	int _numInventory, _numRooms, _numScripts, _numSounds;
	int _numCharsets, _numNewNames, _numGlobalScripts;
	int _numActors;
public:
	int _numCostumes;	// FIXME - should be protected, used by Actor::remapActorPalette
	
	char *_audioNames;
	int32 _numAudioNames;

protected:
	/* Current objects - can go in their respective classes */
	byte _curActor;
	int _curVerb;
	int _curVerbSlot;
	int _curPalIndex;
public:
	byte _currentRoom;

	bool _egoPositioned;	// Used by Actor::putActor, hence public
protected:
	int _keyPressed;
	uint16 _lastKeyHit;

	ScummVM::Point _mouse;
	ScummVM::Point _virtualMouse;

	uint16 _mouseButStat;
	byte _leftBtnPressed, _rightBtnPressed;

	int _bootParam;		// The bootparam, to be passed to the script 1, the bootscript
	
	// Various options useful for debugging
	bool _dumpScripts;
	bool _hexdumpScripts;
	bool _showStack;
	uint16 _debugMode;

protected:
	/* Save/Load class - some of this may be GUI */
	byte _saveLoadFlag, _saveLoadSlot;
	uint32 _lastSaveTime;
	bool _saveLoadCompatible;
	char _saveLoadName[32];

	bool saveState(int slot, bool compat, SaveFileManager *mgr);
	bool loadState(int slot, bool compat, SaveFileManager *mgr);
	bool saveState(int slot, bool compat) {
		SaveFileManager *mgr = _system->get_savefile_manager();
		bool result = saveState(slot, compat, mgr);
		delete mgr;
		return result;
	}
	bool loadState(int slot, bool compat) {
		SaveFileManager *mgr = _system->get_savefile_manager();
		bool result = loadState(slot, compat, mgr);
		delete mgr;
		return result;
	}
	void saveOrLoad(Serializer *s, uint32 savegameVersion);

public:
	bool getSavegameName(int slot, char *desc, SaveFileManager *mgr);
	void makeSavegameName(char *out, int slot, bool compatible);
	void saveLoadResource(Serializer *ser, int type, int index);
	void listSavegames(bool *marks, int num, SaveFileManager *mgr);
	
	void requestSave(int slot, const char *name);
	void requestLoad(int slot);

protected:
	/* Heap and memory management */
	uint32 _maxHeapThreshold, _minHeapThreshold;
	void lock(int type, int i);
	void unlock(int type, int i);

	/* Script VM - should be in Script class */
	uint32 _localScriptList[NUM_LOCALSCRIPT];
	const byte *_scriptPointer, *_scriptOrgPointer;
	byte _opcode, _numNestedScripts, _currentScript;
	uint16 _curExecScript;
	byte **_lastCodePtr;
	int _resultVarNumber, _scummStackPos;
	int _localParamList[16],  _scummStack[150];
	int _keyScriptKey, _keyScriptNo;
	
	virtual void setupOpcodes() = 0;
	virtual void executeOpcode(byte i) = 0;
	virtual const char *getOpcodeDesc(byte i) = 0;

	void initializeLocals(int slot, int *vars);
	int	getScriptSlot();

	void startScene(int room, Actor *a, int b);
	void startManiac();

public:
	void runScript(int script, bool freezeResistant, bool recursive, int *lvarptr);
	void stopScript(int script);

protected:
	void runScriptNested(int script);
	void executeScript();	
	void updateScriptPtr();
	void getScriptBaseAddress();
	void getScriptEntryPoint();
	byte fetchScriptByte();
	virtual uint fetchScriptWord();
	virtual int fetchScriptWordSigned();
	void ignoreScriptWord() { fetchScriptWord(); }
	void ignoreScriptByte() { fetchScriptByte(); }
	virtual void getResultPos();
	void setResult(int result);
	void push(int a);
	int pop();
	virtual int readVar(uint var);
	virtual void writeVar(uint var, int value);
	void runHook(int i);
	bool isScriptInUse(int script);
	
	void freezeScripts(int scr);
	void unfreezeScripts();
	void runAllScripts();
	void beginCutscene(int *args);
	void endCutscene();
	void abortCutscene();
	void runExitScript();
	void runEntryScript();

	void beginOverride();
	void endOverride();
	void killAllScriptsExceptCurrent();
	void killScriptsAndResources();
	void checkAndRunSentenceScript();
	void decreaseScriptDelay(int amount);
public:
	bool isScriptRunning(int script);	// FIXME - should be protected, used by Sound::startTalkSound
protected:
	bool isRoomScriptRunning(int script);
	void copyScriptString(byte *dst);
	int resStrLen(const byte *src) const;
	void doSentence(int c, int b, int a);
	void setStringVars(int i);

	/* Script VM or Object class? */
	void stopObjectCode();
	void stopObjectScript(int script);

	/* Should be in Resource class */
	byte _encbyte;
	File _fileHandle;
	uint32 _fileOffset;
	int _resourceHeaderSize;
	char *_exe_name;	// This is the name we use for opening resource files
	char *_game_name;	// This is the game the user calls it, so use for saving
	bool _dynamicRoomOffsets;
	byte _resourceMapper[128];
	uint32 _allocatedSize;
	byte _expire_counter;
	byte *_HEV7RoomOffsets;

	void allocateArrays();
	void openRoom(int room);
	void closeRoom();
	void deleteRoomOffsets();
	void readRoomsOffsets();
	void askForDisk(const char *filename, int disknum);
	bool openResourceFile(const char *filename);

protected:
	void loadPtrToResource(int type, int i, const byte *ptr);
	void readResTypeList(int id, uint32 tag, const char *name);
	char *resTypeFromId(int id);
	void allocResTypeData(int id, uint32 tag, int num, const char *name, int mode);
	byte *createResource(int type, int index, uint32 size);
	int loadResource(int type, int i);
	void nukeResource(int type, int i);	

public:
	bool isGlobInMemory(int type, int index);
	bool isResourceLoaded(int type, int index);
	byte *getResourceAddress(int type, int i);
	byte *getStringAddress(int i);
	byte *getStringAddressVar(int i);
	void ensureResourceLoaded(int type, int i);
	int getResourceRoomNr(int type, int index);

protected:
	int readSoundResource(int type, int index);
	int readSoundResourceSmallHeader(int type, int index);
	void setResourceCounter(int type, int index, byte flag);
	bool validateResource(const char *str, int type, int index);
	void increaseResourceCounter();
	bool isResourceInUse(int type, int i);
	void initRoomSubBlocks();
	void clearRoomObjects();
	void loadRoomObjects();
	void loadRoomObjectsSmall();
	void loadRoomObjectsOldBundle();

	void readArrayFromIndexFile();
	virtual void readMAXS();
	virtual void readIndexFile();
	virtual void loadCharset(int i);
	void nukeCharset(int i);

	int _lastLoadedRoom;
public:
	int _roomResource;  // FIXME - should be protected but Sound::pauseSounds accesses it
	byte *findResourceData(uint32 tag, byte *ptr);
	int getResourceDataSize(byte *ptr);
	void dumpResource(char *tag, int index, byte *ptr, int length = -1);

protected:
	int getArrayId();
	void nukeArray(int a);
	int defineArray(int a, int b, int c, int d);
	int readArray(int array, int index, int base);
	void writeArray(int array, int index, int base, int value);

	void resourceStats();
	void expireResources(uint32 size);
	void freeResources();
	void destroy();

public:
	/* Should be in Object class */
	byte OF_OWNER_ROOM;
	int getInventorySlot();
	void SamInventoryHack(int obj);	// FIXME: Sam and Max hack
	int findInventory(int owner, int index);
	int getInventoryCount(int owner);

protected:
	byte *_objectOwnerTable, *_objectRoomTable, *_objectStateTable;
	ObjectIDMap _objectIDMap;
	byte _numObjectsInRoom;

	void setupRoomObject(ObjectData *od, byte *room, byte *searchptr = NULL);
	void removeObjectFromRoom(int obj);
	void loadFlObject(uint object, uint room);
	void nukeFlObjects(int min, int max);
	int findFlObjectSlot();
	int findLocalObjectSlot();
	void addObjectToInventory(uint obj, uint room);
	void fixObjectFlags();
public:
	bool getClass(int obj, int cls);		// Used in actor.cpp, hence public
protected:
	void putClass(int obj, int cls, bool set);
	int getState(int obj);
	void putState(int obj, int state);
	void setObjectState(int obj, int state, int x, int y);
	int getOwner(int obj);
	void putOwner(int obj, int owner);
	void setOwnerOf(int obj, int owner);
	void clearOwnerOf(int obj);
	int getObjectRoom(int obj);
	int getObjX(int obj);
	int getObjY(int obj);
	void getObjectXYPos(int object, int &x, int &y)	{ int dir; getObjectXYPos(object, x, y, dir); }
	void getObjectXYPos(int object, int &x, int &y, int &dir);
	int getObjOldDir(int obj);
	int getObjNewDir(int obj);
	int getObjectIndex(int object);
	int whereIsObject(int object);
	int findObject(int x, int y);
	void findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint object, uint room);	
public:
	int getObjectOrActorXY(int object, int &x, int &y);	// Used in actor.cpp, hence public
protected:
	int getObjActToObjActDist(int a, int b); // Not sure how to handle
	byte *getObjOrActorName(int obj);		 // these three..

	void addObjectToDrawQue(int object);
	void clearDrawObjectQueue();
	void processDrawQue();

	uint32 getOBCDOffs(int object);
	byte *getOBCDFromObject(int obj);	
	int getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f);


protected:
	/* Should be in Verb class */
	uint16 _verbMouseOver;
	int _inventoryOffset;
	int8 _userPut;
	uint16 _userState;

	void redrawVerbs();
	void checkExecVerbs();
	void verbMouseOver(int verb);
	int checkMouseOver(int x, int y);
	void drawVerb(int verb, int mode);
	void runInputScript(int a, int cmd, int mode);
	void restoreVerbBG(int verb);
	void drawVerbBitmap(int verb, int x, int y);
	int getVerbEntrypoint(int obj, int entry);
	int getVerbSlot(int id, int mode);
	void killVerb(int slot);
	void runObjectScript(int script, int entry, bool freezeResistant, bool recursive, int *vars);
	void setVerbObject(uint room, uint object, uint verb);

	void checkV2Inventory(int x, int y);
	void redrawV2Inventory();

public:
	/* Should be in Actor class */
	Actor *derefActor(int id);
	Actor *derefActorSafe(int id, const char *errmsg);
	void showActors();

	uint32 *_classData;

	int getAngleFromPos(int x, int y);

	void walkActors();
	void playActorSounds();
	void setActorRedrawFlags(bool fg, bool bg);
	void resetActorBgs();
	void processActors();
	void processUpperActors();
	int getActorFromPos(int x, int y);
	void actorFollowCamera(int act);
	
	bool isCostumeInUse(int i);

	/* Actor talking stuff */
	byte _actorToPrintStrFor;
	int _sentenceNum;
	SentenceTab _sentence[NUM_SENTENCE];
	StringTab _string[6];
	void actorTalk();
	void stopTalk();
	
	// Costume class
	void cost_decodeData(Actor *a, int frame, uint usemask);
	int cost_frameToAnim(Actor *a, int frame);

	// Akos Class
	bool akos_increaseAnims(byte *akos, Actor *a);
	bool akos_increaseAnim(Actor *a, int i, byte *aksq, uint16 *akfo, int numakfo);
	void akos_queCommand(byte cmd, Actor *a, int param_1, int param_2);
	bool akos_compare(int a, int b, byte cmd);
	void akos_decodeData(Actor *a, int frame, uint usemask);
	int akos_frameToAnim(Actor *a, int frame);
	bool akos_hasManyDirections(Actor *a);

	/* Should be in Graphics class? */
	uint16 _screenB, _screenH;
	int _roomHeight, _roomWidth, _screenHeight, _screenWidth;
	VirtScreen virtscr[4];		// Virtual screen areas
	CameraData camera;			// 'Camera' - viewport
	ColorCycle _colorCycle[16];	// Palette cycles

	uint32 _ENCD_offs, _EXCD_offs;
	uint32 _CLUT_offs;
	uint32 _IM00_offs, _PALS_offs;

	//ender: fullscreen
	bool _fullRedraw, _BgNeedsRedraw, _verbRedraw;
	bool _screenEffectFlag, _completeScreenRedraw;

	struct {
		int hotspotX, hotspotY, width, height;
		byte animate, animateIndex;
		int8 state;
	} _cursor;
	byte _grabbedCursor[8192];
	byte _currentCursor;

	byte _newEffect, _switchRoomEffect2, _switchRoomEffect;
	bool _doEffect;
	
	struct {
		int x, y, w, h;
		byte *buffer;
		uint16 xStrips, yStrips;
		bool isDrawn;
	} _flashlight;

	void getGraphicsPerformance();
	void initScreens(int a, int b, int w, int h);
	void initVirtScreen(int slot, int number, int top, int width, int height, bool twobufs, bool scrollable);
	void initBGBuffers(int height);
	void initCycl(byte *ptr);	// Color cycle

	void createSpecialPalette(int16 a, int16 b, int16 c, int16 d, int16 e, int16 colorMin, int16 colorMax);

	void drawObject(int obj, int arg);	
	void drawRoomObjects(int arg);
	void drawRoomObject(int i, int arg);
	void drawBox(int x, int y, int x2, int y2, int color);

	void restoreBG(ScummVM::Rect rect, byte backColor = 0);
	void redrawBGStrip(int start, int num);	
	void redrawBGAreas();	
	
	void moveCamera();
	void cameraMoved();
	void setCameraAtEx(int at);
	void setCameraAt(int pos_x, int pos_y);
	void panCameraTo(int x, int y);
	void setCameraFollows(Actor *a);
	void clampCameraPos(ScummVM::Point *pt);

	byte *getPalettePtr();
	void setupEGAPalette();
	void setPalette(int pal);
	void setPaletteFromPtr(byte *ptr);
	void setPaletteFromRes();
	void setPalColor(int index, int r, int g, int b);
	void setDirtyColors(int min, int max);
	byte *findPalInPals(byte *pal, int index);
	void swapPalColors(int a, int b);
	void copyPalColor(int dst, int src);
	void cyclePalette();
	void stopCycle(int i);
	void palManipulateInit(int start, int end, int string_id, int time);
	void palManipulate();
	int remapPaletteColor(int r, int g, int b, uint threshold);
	void moveMemInPalRes(int start, int end, byte direction);
	void setupShadowPalette(int slot, int redScale, int greenScale, int blueScale, int startColor, int endColor);
	void setupShadowPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor);
	void darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor);
	void desaturatePalette(int hueScale, int satScale, int lightScale, int startColor, int endColor);

	void setCursor(int cursor);
	void setCursorImg(uint img, uint room, uint imgindex);
	void setCursorHotspot(int x, int y);
	void grabCursor(int x, int y, int w, int h);
	void grabCursor(byte *ptr, int width, int height);
	void makeCursorColorTransparent(int a);
	void setupCursor() { _cursor.animate = 1; }
	void decompressDefaultCursor(int index);
	void useIm01Cursor(byte *im, int w, int h);
	void useBompCursor(byte *im, int w, int h);


	void updateDirtyRect(int virt, int left, int right, int top, int bottom, int dirtybit);
	void setDirtyRange(int slot, int a, int height);
	void drawDirtyScreenParts();
	void updateDirtyScreen(int slot);

	VirtScreen *findVirtScreen(int y);
	void setVirtscreenDirty(VirtScreen *vs, int left, int top, int right, int bottom);

	bool isMaskActiveAt(int l, int t, int r, int b, byte *mem);

	void drawFlashlight();
	
	void fadeIn(int effect);
	void fadeOut(int effect);

	void unkScreenEffect5(int a);
	void unkScreenEffect6();
	void transitionEffect(int a);
	void dissolveEffect(int width, int height);
	void scrollEffect(int dir);

	void blit(byte *dst, byte *src, int w, int h);

	// bomp
	void decompressBomp(byte *dst, byte *src, int w, int h);
	void drawBomp(BompDrawData *bd, int decode_mode, int mask);
	int32 setupBompScale(byte *scalling, int32 size, byte scale);
	void bompScaleFuncX(byte *line_buffer, byte *scalling_x_ptr, byte skip, int32 size);
	int32 bompDecodeLineMode0(byte *src, byte *line_buffer, int32 size);
	int32 bompDecodeLineMode1(byte *src, byte *line_buffer, int32 size);
	int32 bompDecodeLineMode3(byte *src, byte *line_buffer, int32 size);
	void bompApplyMask(byte *line_buffer, byte *mask_out, byte bits, int32 size);
	void bompApplyShadow0(byte *line_buffer, byte *dst, int32 size);
	void bompApplyShadow1(byte *line_buffer, byte *dst, int32 size);
	void bompApplyShadow3(byte *line_buffer, byte *dst, int32 size);
	void bompApplyActorPalette(byte *line_buffer, int32 size);

	int32 _bompScaleRight, _bompScaleBottom;
	byte *_bompScallingXPtr, *_bompScallingYPtr;
	byte *_bompMaskPtr;
	byte *_bompActorPalletePtr;

	bool _shakeEnabled;
	uint _shakeFrame;
	void setShake(int mode);

	int _screenStartStrip, _screenEndStrip;
	int _screenLeft, _screenTop;

	int _blastObjectQueuePos; 
	BlastObject _blastObjectQueue[128];

	int _blastTextQueuePos;
	BlastText _blastTextQueue[32];	// FIXME - how many blast texts can there be at once?

	void enqueueText(byte *text, int x, int y, byte color, byte charset, bool center);
	void drawBlastTexts();
	void removeBlastTexts();

	void enqueueObject(int objectNumber, int objectX, int objectY, int objectWidth,
	                   int objectHeight, int scaleX, int scaleY, int image, int mode);
	void clearEnqueue() { _blastObjectQueuePos = 0; }
	void drawBlastObjects();
	void drawBlastObject(BlastObject *eo);
	void removeBlastObjects();
	void removeBlastObject(BlastObject *eo);

	int _drawObjectQueNr;
	byte _drawObjectQue[200];
	byte _palManipStart, _palManipEnd;
	uint16 _palManipCounter;
	byte *_palManipPalette;
	byte *_palManipIntermediatePal;
	
	/* For each of the 410 screen strips, gfxUsageBits contains a
	 * bitmask. The lower 80 bits each correspond to one actor and
	 * signify if any part of that actor is currently contained in
	 * that strip.
	 * 
	 * If the leftmost bit is set, the strip (background) is dirty
	 * needs to be redrawn.
	 * 
	 * The second leftmost bit is set by removeBlastObject() and
	 * restoreBG(), but I'm not yet sure why.
	 */
	uint32 gfxUsageBits[410 * 3];
	
	byte *_shadowPalette;
	int _shadowPaletteSize;
	byte _currentPalette[3 * 256];

	byte _proc_special_palette[256];
	int _palDirtyMin, _palDirtyMax;

	byte _haveMsg;
	bool _useTalkAnims;
	uint16 _defaultTalkDelay;
	bool _use_adlib;
	int tempMusic;
	bool _silentDigitalImuse, _noDigitalSamples;
	int _saveSound;
	int current_cd_sound;

	/* Walkbox / Navigation class */
	int _maxBoxVertexHeap, _boxPathVertexHeapIndex, _boxMatrixItem;
	byte *_boxPathVertexHeap, *_boxMatrixPtr1, *_boxMatrixPtr3;	

	uint16 _extraBoxFlags[65];

	PathNode *unkMatrixProc2(PathVertex *vtx, int i);
	bool areBoxesNeighbours(int i, int j);
	void addToBoxMatrix(byte b);
	bool compareSlope(int X1, int Y1, int X2, int Y2, int X3, int Y3);
	void *addToBoxVertexHeap(int size);
	PathVertex *addPathVertex();
	bool checkXYInBoxBounds(int box, int x, int y);
	uint distanceFromPt(int x, int y, int ptx, int pty);
	ScummVM::Point closestPtOnLine(int ulx, int uly, int llx, int lly, int x, int y);
	void getBoxCoordinates(int boxnum, BoxCoords *bc);
	byte getMaskFromBox(int box);
	Box *getBoxBaseAddr(int box);
	byte getBoxFlags(int box);
	int getBoxScale(int box);

	int getScale(int box, int x, int y);
	void setScaleItem(int slot, int a, int b, int c, int d);

	// V8 scaling stuff: should be in V8 class
	struct ScaleSlot {
		int x1, y1, scale1;
		int x2, y2, scale2;
	};
	ScaleSlot _scaleSlots[20];	// FIXME - not sure if this limit is right, but based on my observations it is
	void setScaleSlot(int slot, int x1, int y1, int scale1, int x2, int y2, int scale2);
	void setBoxScaleSlot(int box, int slot);

	byte getNumBoxes();
	byte *getBoxMatrixBaseAddr();
	int getPathToDestBox(byte from, byte to);
	bool findPathTowards(Actor *a, byte box, byte box2, byte box3, int16 &foundPathX, int16 &foundPathY);
	void findPathTowardsOld(Actor *a, byte box, byte box2, byte box3, ScummVM::Point gateLoc[5]);
	void getGates(int trap1, int trap2, ScummVM::Point gateA[2], ScummVM::Point gateB[2]);
	bool inBoxQuickReject(int box, int x, int y, int threshold);
	AdjustBoxResult getClosestPtOnBox(int box, int x, int y);
	int getSpecialBox(int param1, int param2);
	
	void setBoxFlags(int box, int val);
	void setBoxScale(int box, int b);
	void createBoxMatrix();


	/* String class */
	CharsetRenderer *_charset;
	byte _charsetColor;
	byte _charsetColorMap[16];
	byte _charsetData[15][16];

	int _charsetBufPos;
	byte _charsetBuffer[512];

	bool _noSubtitles;	// Whether to skip all subtitles

	void initCharset(int charset);
	void restoreCharsetBg();
	bool hasCharsetMask(int left, int top, int right, int bottom);
	void CHARSET_1();
	void drawString(int a);
	void drawDescString(byte *msg);
	const byte *addMessageToStack(const byte *msg);
	void addIntToStack(int var);
	void addVerbToStack(int var);
	void addNameToStack(int var);
	void addStringToStack(int var);
	void unkMessage1();
	void unkMessage2();
	void clearMsgQueue();

	int _numInMsgStack;
	byte *_msgPtrToAdd;
	const byte *_messagePtr;
	int16 _talkDelay;
	bool _keepText;
	uint16 _language;
	bool _existLanguageFile;
	char *_languageBuffer;
	struct langIndexNode *_languageIndex;
	int _languageStrCount;
	void loadLanguageBundle();
	void translateText(const byte *text, byte *trans_buff);
	byte _transText[500];

#if defined(SCUMM_LITTLE_ENDIAN)
	uint32 fileReadDword() { return _fileHandle.readUint32LE(); }
#elif defined(SCUMM_BIG_ENDIAN)
	uint32 fileReadDword() { return _fileHandle.readUint32BE(); }
#endif

	void upgradeGfxUsageBits();
	void setGfxUsageBit(int strip, int bit);
	void clearGfxUsageBit(int strip, int bit);
	bool testGfxUsageBit(int strip, int bit);
	bool testGfxAnyUsageBits(int strip);
	bool testGfxOtherUsageBits(int strip, int bit);

	/* Scumm Vars */
	byte VAR_LANGUAGE;
	byte VAR_KEYPRESS;
	byte VAR_EGO;
	byte VAR_CAMERA_POS_X;
	byte VAR_HAVE_MSG;
	byte VAR_ROOM;
	byte VAR_OVERRIDE;
	byte VAR_MACHINE_SPEED;
	byte VAR_ME;
	byte VAR_NUM_ACTOR;
	byte VAR_CURRENT_LIGHTS;
	byte VAR_CURRENTDRIVE;	// How about merging this with VAR_CURRENTDISK?
	byte VAR_CURRENTDISK;
	byte VAR_TMR_1;
	byte VAR_TMR_2;
	byte VAR_TMR_3;
	byte VAR_MUSIC_TIMER;
	byte VAR_ACTOR_RANGE_MIN;
	byte VAR_ACTOR_RANGE_MAX;
	byte VAR_CAMERA_MIN_X;
	byte VAR_CAMERA_MAX_X;
	byte VAR_TIMER_NEXT;
	byte VAR_VIRT_MOUSE_X;
	byte VAR_VIRT_MOUSE_Y;
	byte VAR_ROOM_RESOURCE;
	byte VAR_LAST_SOUND;
	byte VAR_CUTSCENEEXIT_KEY;
	byte VAR_OPTIONS_KEY;
	byte VAR_TALK_ACTOR;
	byte VAR_CAMERA_FAST_X;
	byte VAR_SCROLL_SCRIPT;
	byte VAR_ENTRY_SCRIPT;
	byte VAR_ENTRY_SCRIPT2;
	byte VAR_EXIT_SCRIPT;
	byte VAR_EXIT_SCRIPT2;
	byte VAR_VERB_SCRIPT;
	byte VAR_SENTENCE_SCRIPT;
	byte VAR_HOOK_SCRIPT;
	byte VAR_CUTSCENE_START_SCRIPT;
	byte VAR_CUTSCENE_END_SCRIPT;
	byte VAR_CHARINC;
	byte VAR_WALKTO_OBJ;
	byte VAR_DEBUGMODE;
	byte VAR_HEAPSPACE;
	byte VAR_RESTART_KEY;
	byte VAR_PAUSE_KEY;
	byte VAR_MOUSE_X;
	byte VAR_MOUSE_Y;
	byte VAR_TIMER;
	byte VAR_TMR_4;
	byte VAR_SOUNDCARD;
	byte VAR_VIDEOMODE;
	byte VAR_SAVELOADDIALOG_KEY;
	byte VAR_FIXEDDISK;
	byte VAR_CURSORSTATE;
	byte VAR_USERPUT;
	byte VAR_SOUNDRESULT;
	byte VAR_TALKSTOP_KEY;
	byte VAR_59;

	byte VAR_SOUNDPARAM;
	byte VAR_SOUNDPARAM2;
	byte VAR_SOUNDPARAM3;
	byte VAR_MOUSEPRESENT;
	byte VAR_PERFORMANCE_1;
	byte VAR_PERFORMANCE_2;
	byte VAR_ROOM_FLAG;
	byte VAR_GAME_LOADED;
	byte VAR_NEW_ROOM;
	byte VAR_VERSION;

	byte VAR_V5_TALK_STRING_Y;

	byte VAR_V6_SCREEN_WIDTH;
	byte VAR_V6_SCREEN_HEIGHT;
	byte VAR_V6_EMSSPACE;
	byte VAR_V6_RANDOM_NR;

	byte VAR_STRING2DRAW;
	byte VAR_CAMERA_POS_Y;

	byte VAR_CAMERA_MIN_Y;
	byte VAR_CAMERA_MAX_Y;
	byte VAR_CAMERA_THRESHOLD_X;
	byte VAR_CAMERA_THRESHOLD_Y;
	byte VAR_CAMERA_SPEED_X;
	byte VAR_CAMERA_SPEED_Y;
	byte VAR_CAMERA_ACCEL_X;
	byte VAR_CAMERA_ACCEL_Y;

	byte VAR_CAMERA_DEST_X;

	byte VAR_CAMERA_DEST_Y;

	byte VAR_CAMERA_FOLLOWED_ACTOR;

	byte VAR_LEFTBTN_DOWN;
	byte VAR_RIGHTBTN_DOWN;
	byte VAR_LEFTBTN_HOLD;
	byte VAR_RIGHTBTN_HOLD;
	byte VAR_MOUSE_BUTTONS;
	byte VAR_MOUSE_HOLD;
	byte VAR_SAVELOAD_SCRIPT;
	byte VAR_SAVELOAD_SCRIPT2;

	byte VAR_DEFAULT_TALK_DELAY;
	byte VAR_CHARSET_MASK;

	byte VAR_CUSTOMSCALETABLE;
	byte VAR_V6_SOUNDMODE;

	byte VAR_CHARCOUNT;
	byte VAR_VERB_ALLOWED;
	byte VAR_ACTIVE_VERB;
	byte VAR_ACTIVE_OBJECT1;
	byte VAR_ACTIVE_OBJECT2;
	byte VAR_CLICK_AREA;
};

// This is a constant lookup table of reverse bit masks
extern const byte revBitMask[8];

/* Direction conversion functions (between old dir and new dir format) */
int newDirToOldDir(int dir);
int oldDirToNewDir(int dir);

int normalizeAngle(int angle);
int fromSimpleDir(int dirtype, int dir);
int toSimpleDir(int dirtype, int dir);


#endif
