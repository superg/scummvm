#ifndef GAG_H__
#define GAG_H__



#include "common/archive.h"
#include "common/error.h"
#include "common/hashmap.h"
#include "common/list.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/system.h"
#include "engines/engine.h"
#include "graphics/pixelformat.h"
#include <test/cxxtest/cxxtest/Mock.h>



namespace Gag
{

class GagEngine : public Engine
{
public:
	GagEngine(OSystem *syst);
	~GagEngine();

	virtual Common::Error run();

private:
	enum GameState
	{
		GS_ACTIVE,
		GS_SCRIPT
	};

	enum ParserState
	{
		PS_SECTION_SEARCH,
		PS_SECTION_NAME,
		PS_SECTION_BODY,
		PS_COMMAND_NAME,
		PS_COMMAND_VALUE
	};

	static const Graphics::PixelFormat _SCREEN_FORMAT;
	static const int _SCREEN_WIDTH;
	static const int _SCREEN_HEIGHT;
	static const int _SCREEN_FPS;

	// ************ SCRIPT ************
	enum TransitionMode
	{
		TM_NOFADE,
		TM_PALFADE,
		TM_FRAMEFADE
	};

	struct Event
	{
		Common::String name;

		Common::String script;
		Common::String section;
		TransitionMode transition_mode;
	};


	static const Common::String _DEFAULT_SCRIPT;
	static const Common::String _DEFAULT_SECTION;
	static const Common::String _END_SECTION;
	static const char _OPTION_PREFIX;
	static const char _ARGUMENT_DELIMITER;

	Common::String _script;
	Common::String _section;
	Common::List<Event> _events;

	Common::HashMap<Common::String, Common::Error (GagEngine::*)(const Common::String &)> _commandCallbacks;
	// ********************************

	GameState _state;
	Common::ScopedPtr<Common::Archive> _archive;

	void Init();
	Common::Error Update();

	Common::Error StateActive();
	Common::Error StateScript();

	Common::String ParseOption(Common::Array<Common::String> &arguments, Common::String option);

	Common::Error ScriptCatch(const Common::String &value);
	Common::Error ScriptClass(const Common::String &value);
	Common::Error ScriptCommand(const Common::String &value);
	Common::Error ScriptDriveSpeed(const Common::String &value);
	Common::Error ScriptEvent(const Common::String &value);
	Common::Error ScriptException(const Common::String &value);
	Common::Error ScriptExFiles(const Common::String &value);
	Common::Error ScriptFadeMask(const Common::String &value);
	Common::Error ScriptFlags(const Common::String &value);
	Common::Error ScriptFont(const Common::String &value);
	Common::Error ScriptImage(const Common::String &value);
	Common::Error ScriptInventory(const Common::String &value);
	Common::Error ScriptLanguage(const Common::String &value);
	Common::Error ScriptLayer(const Common::String &value);
	Common::Error ScriptList(const Common::String &value);
	Common::Error ScriptLoad(const Common::String &value);
	Common::Error ScriptLocal(const Common::String &value);
	Common::Error ScriptMouse(const Common::String &value);
	Common::Error ScriptObject(const Common::String &value);
	Common::Error ScriptParams(const Common::String &value);
	Common::Error ScriptPath(const Common::String &value);
	Common::Error ScriptSource(const Common::String &value);
	Common::Error ScriptSubLocation(const Common::String &value);
	Common::Error ScriptTemplate(const Common::String &value);
	Common::Error ScriptText(const Common::String &value);
	Common::Error ScriptTry(const Common::String &value);
	Common::Error ScriptVolume(const Common::String &value);
	Common::Error ScriptZone(const Common::String &value);

	//DEBUG
	void ExtractCdf(const Common::String &a_fn);
/*
	void BitmapTest();
	void AnimationTest();
	void TestPlayAnimation(Common::String fn);
	void AnimationTuckerTest();
	void TestTuckerPlayAnimation(Common::String fn);
*/
};

}



#endif
