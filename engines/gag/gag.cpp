#include <limits>
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "engines/util.h"
//#include "graphics/decoders/bmp.h"
//#include "graphics/palette.h"
//#include "graphics/surface.h"
#include "cdf_archive.h"
#include "gag_flic_decoder.h"
#include "gag.h"



namespace Gag
{

const Graphics::PixelFormat GagEngine::_SCREEN_FORMAT(2, 5, 6, 5, 0, 11, 5, 0, 0);
const int GagEngine::_SCREEN_WIDTH(640);
const int GagEngine::_SCREEN_HEIGHT(480);
const int GagEngine::_SCREEN_FPS(60);
const Common::String GagEngine::_DEFAULT_SCRIPT("START.CFG");
const Common::String GagEngine::_DEFAULT_SECTION("CFG");


GagEngine::GagEngine(OSystem *syst)
	: Engine(syst)
	, _state(GS_SCRIPT)
	, _script(_DEFAULT_SCRIPT)
	, _section(_DEFAULT_SECTION)
{
	_commandCallbacks["catch"      ] = &GagEngine::ScriptCatch;
	_commandCallbacks["class"      ] = &GagEngine::ScriptClass;
	_commandCallbacks["command"    ] = &GagEngine::ScriptCommand;
	_commandCallbacks["drivespeed" ] = &GagEngine::ScriptDriveSpeed;
	_commandCallbacks["event"      ] = &GagEngine::ScriptEvent;
	_commandCallbacks["exception"  ] = &GagEngine::ScriptException;
	_commandCallbacks["exfiles"    ] = &GagEngine::ScriptExFiles;
	_commandCallbacks["fademask"   ] = &GagEngine::ScriptFadeMask;
	_commandCallbacks["flags"      ] = &GagEngine::ScriptFlags;
	_commandCallbacks["font"       ] = &GagEngine::ScriptFont;
	_commandCallbacks["image"      ] = &GagEngine::ScriptImage;
	_commandCallbacks["inventory"  ] = &GagEngine::ScriptInventory;
	_commandCallbacks["language"   ] = &GagEngine::ScriptLanguage;
	_commandCallbacks["layer"      ] = &GagEngine::ScriptLayer;
	_commandCallbacks["list"       ] = &GagEngine::ScriptList;
	_commandCallbacks["load"       ] = &GagEngine::ScriptLoad;
	_commandCallbacks["local"      ] = &GagEngine::ScriptLocal;
	_commandCallbacks["mouse"      ] = &GagEngine::ScriptMouse;
	_commandCallbacks["object"     ] = &GagEngine::ScriptObject;
	_commandCallbacks["params"     ] = &GagEngine::ScriptParams;
	_commandCallbacks["path"       ] = &GagEngine::ScriptPath;
	_commandCallbacks["source"     ] = &GagEngine::ScriptSource;
	_commandCallbacks["sublocation"] = &GagEngine::ScriptSubLocation;
	_commandCallbacks["template"   ] = &GagEngine::ScriptTemplate;
	_commandCallbacks["text"       ] = &GagEngine::ScriptText;
	_commandCallbacks["try"        ] = &GagEngine::ScriptTry;
	_commandCallbacks["volume"     ] = &GagEngine::ScriptVolume;
	_commandCallbacks["zone"       ] = &GagEngine::ScriptZone;
}


GagEngine::~GagEngine()
{

}


Common::Error GagEngine::run()
{
	Init();

	Common::Error status;
	do
	{
		// do periodic processing
		uint32 time_start = _system->getMillis();
		status = Update();
		uint32 time_end = _system->getMillis();

		// wrap around check
		uint32 time_spent = time_end >= time_start ? time_end - time_start : std::numeric_limits<uint32>::max() - time_start + time_end + 1;

		// sleep remaining frame time
		_system->delayMillis(1000 / _SCREEN_FPS - time_spent);
	}
	while(status.getCode() == Common::kNoError && !shouldQuit());

	return status;
}


//#define GAG_INIT_DEBUG


void GagEngine::Init()
{
	initGraphics(_SCREEN_WIDTH, _SCREEN_HEIGHT, true, &_SCREEN_FORMAT);

	//DEBUG: load files from uncompressed file system
	_archive.reset(new Common::FSDirectory(ConfMan.get("path") + '/' + "Gag01", 2, false));
//	_archive.reset(new CdfArchive("Gag01.cdf", false));

	//DEBUG
#ifdef GAG_INIT_DEBUG
//	_script = "scripts_gagru.cfg";
	_section = "AURICP";
	Common::Error script_error = StateScript();
	debug("test");
	quitGame();
	_state = GS_ACTIVE;

//	ExtractCdf("Gag01.cdf");
//	BitmapTest();
//	AnimationTest();
//	AnimationTuckerTest();
#endif
}


Common::Error GagEngine::Update()
{
	Common::Error status(Common::kNoError);

	switch(_state)
	{
	case GS_ACTIVE:
		status = StateActive();
		break;

	case GS_SCRIPT:
		status = StateScript();
		_state = GS_ACTIVE;
		break;

	default:
		status = Common::kUnknownError;
	}

	return status;
}


Common::Error GagEngine::StateActive()
{
	Common::Error status(Common::kNoError);

	Common::Event event;
	_eventMan->pollEvent(event);
	switch(event.type)
	{
	case Common::EVENT_KEYDOWN:
		switch(event.kbd.keycode)
		{
		case Common::KEYCODE_q:
			if(event.kbd.hasFlags(Common::KBD_CTRL))
				quitGame();
			break;

		default:
			;
		}
		break;

	default:
		;
	}

	return status;
}


Common::Error GagEngine::StateScript()
{
	Common::Error status(Common::kNoError);

	Common::File file;
	if(!file.open(_script, *_archive))
		status = Common::kReadingFailed;

	// simple script parsing: skim through, find section and execute commands
	ParserState parse_state(PS_SECTION_SEARCH);
	Common::String buffer;
	Common::String command_name;

	bool done(false);
	while(!file.eos())
	{
		// add space multi line command support
		Common::String line = file.readLine() + ' ';
		if(file.err())
		{
			status = Common::kReadingFailed;
			break;
		}

		bool skip_line(false);
		for(Common::String::const_iterator it = line.begin(); it != line.end(); ++it)
		{
			switch(parse_state)
			{
			case PS_SECTION_SEARCH:
				// section
				if(*it == '[')
				{
					buffer.clear();

					parse_state = PS_SECTION_NAME;
				}
				break;

			case PS_SECTION_NAME:
				// section end
				if(*it == ']')
					parse_state = buffer == _section ? PS_SECTION_BODY : PS_SECTION_SEARCH;
				else
					buffer += *it;
				break;

			case PS_SECTION_BODY:
				// section
				if(*it == '[')
				{
					skip_line = true;
					done = true;
				}
				// comment
				else if(*it == '*')
				{
					skip_line = true;
				}
				// command name
				else if((*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z'))
				{
					buffer.clear();
					buffer += *it;

					parse_state = PS_COMMAND_NAME;
				}
				break;

			case PS_COMMAND_NAME:
				// command value
				if(*it == '=')
				{
					command_name = buffer;
					buffer.clear();

					parse_state = PS_COMMAND_VALUE;
				}
				else
					buffer += *it;
				break;

			case PS_COMMAND_VALUE:
				// command value end
				if(*it == ';')
				{
					debug("%s=%s;", command_name.c_str(), buffer.c_str());

					Common::HashMap<Common::String, Common::Error (GagEngine::*)(const Common::String &)>::const_iterator f = _commandCallbacks.find(command_name);
					if(f != _commandCallbacks.end())
						(this->*f->_value)(buffer);

					parse_state = PS_SECTION_BODY;
				}
				else
					buffer += *it;
				break;

			default:
				;
			}

			if(skip_line)
				break;
		}

		if(done)
			break;
	}

	return status;
}


Common::Error GagEngine::ScriptCatch(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptClass(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptCommand(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptDriveSpeed(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptEvent(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptException(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptExFiles(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptFadeMask(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptFlags(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptFont(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptImage(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptInventory(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptLanguage(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptLayer(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptList(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptLoad(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptLocal(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptMouse(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptObject(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptParams(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptPath(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptSource(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptSubLocation(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptTemplate(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptText(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptTry(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptVolume(const Common::String &value)
{
	return Common::kUnknownError;
}


Common::Error GagEngine::ScriptZone(const Common::String &value)
{
	return Common::kUnknownError;
}



/*
void GagEngine::ExtractCdf(const Common::String &a_fn)
{
	CdfArchive archive(a_fn, true);

	uint pos = a_fn.size();
	for(uint i = 0; i < a_fn.size(); ++i)
	{
		if(a_fn[i] == '.')
			pos = i;
	}

	//TODO: figure out how to create directory
	Common::String dn(ConfMan.get("path") + '/' + Common::String(a_fn.c_str(), pos) + '/');

	Common::ArchiveMemberList member_list;
	archive.listMembers(member_list);
	for(Common::ArchiveMemberList::iterator it = member_list.begin(); it != member_list.end(); ++it)
	{
		Common::ScopedPtr<Common::SeekableReadStream> stream((*it)->createReadStream());

		if(stream)
		{
			uint8 *buffer = new uint8[stream->size()];
			stream->read(buffer, stream->size());

			Common::DumpFile file;
			if(file.open(dn + (*it)->getName()))
				file.write(buffer, stream->size());

			delete [] buffer;
		}
	}
}


void GagEngine::BitmapTest()
{
//	Graphics::BitmapDecoder bitmap_decoder;
//	Common::File file;
//	file.open("AUTORUN.BMP", *_archive);
//	if(bitmap_decoder.loadStream(file))
//	{
//		Graphics::Surface *surface = bitmap_decoder.getSurface()->convertTo(_system->getScreenFormat(), bitmap_decoder.getPalette());
//		_system->copyRectToScreen((const byte *)surface->getPixels(), surface->pitch, 0, 0, surface->w, surface->h);
//		_system->updateScreen();
//		surface->free();
//		delete surface;
//	}
}


void GagEngine::AnimationTest()
{
//	TestPlayAnimation("LOGO1.MVZ");
//	TestPlayAnimation("WC0501.MOV");

	Common::ArchiveMemberList member_list;
	_archive->listMembers(member_list);
	for(Common::ArchiveMemberList::iterator it = member_list.begin(); it != member_list.end(); ++it)
	{
//		if(!(*it)->getName().hasSuffix(".FLC") && !(*it)->getName().hasSuffix(".MOV") && !(*it)->getName().hasSuffix(".MVZ"))
		if(!(*it)->getName().hasSuffix(".MOV") && !(*it)->getName().hasSuffix(".MVZ"))
			continue;

		debug("playing: %s", (*it)->getName().c_str());

		TestPlayAnimation((*it)->getName());
	}

//	quitGame();
}


//#define ANIMATION_FAST_TEST


void GagEngine::TestPlayAnimation(Common::String fn)
{
	// file will be freed inside flic_decoder
	Common::File *file = new Common::File;
	if(file->open(fn, *_archive))
	{
		GagFlicDecoder flic_decoder;

		//NOTE: VideoDecoder frees stream on destruction
		if(flic_decoder.loadStream(file))
		{
			flic_decoder.start();
			while(!flic_decoder.endOfVideo() && flic_decoder.isPlaying())
			{
#ifndef ANIMATION_FAST_TEST
				if(flic_decoder.needsUpdate())
#endif
				{
					const Graphics::Surface *video_surface = flic_decoder.decodeNextFrame();
					if(video_surface != nullptr)
					{
						Graphics::Surface *surface = video_surface->convertTo(_system->getScreenFormat(), flic_decoder.getPalette());
						_system->copyRectToScreen((const byte *)surface->getPixels(), surface->pitch, 0, 0, surface->w, surface->h);
						_system->updateScreen();
						surface->free();
						delete surface;
					}
				}

#ifndef ANIMATION_FAST_TEST
				_system->delayMillis(flic_decoder.getTimeToNextFrame());
#endif
			}
		}
		else
		{
			debug("error loading file");
		}
	}
}


void GagEngine::AnimationTuckerTest()
{
	std::vector<Common::String> tucker_flics;
	tucker_flics.push_back("tucker/BACKGRND.FLC");
	tucker_flics.push_back("tucker/BUDTTLE2.FLC");
	tucker_flics.push_back("tucker/COGBACK.FLC");
	tucker_flics.push_back("tucker/INTRO1.FLC");
	tucker_flics.push_back("tucker/INTRO2.FLC");
	tucker_flics.push_back("tucker/INTRO3.FLC");
	tucker_flics.push_back("tucker/MACHINE.FLC");
	tucker_flics.push_back("tucker/MERIT.FLC");

	for(size_t i = 0; i < tucker_flics.size(); ++i)
	{
		debug("playing: %s", tucker_flics[i].c_str());

		TestTuckerPlayAnimation(tucker_flics[i]);
	}

//	quitGame();
}


void GagEngine::TestTuckerPlayAnimation(Common::String fn)
{
	// file will be freed inside flic_decoder
	Common::File *file = new Common::File;
	if(file->open(fn, *_archive))
	{
		Video::FlicDecoder flic_decoder;

		//NOTE: VideoDecoder frees stream on destruction
		if(flic_decoder.loadStream(file))
		{
			flic_decoder.start();
			while(!flic_decoder.endOfVideo() && flic_decoder.isPlaying())
			{
				const Graphics::Surface *video_surface = flic_decoder.decodeNextFrame();
				if(video_surface != nullptr)
				{
					Graphics::Surface *surface = video_surface->convertTo(_system->getScreenFormat(), flic_decoder.getPalette());
					_system->copyRectToScreen((const byte *)surface->getPixels(), surface->pitch, 0, 0, surface->w, surface->h);
					_system->updateScreen();
					surface->free();
					delete surface;
				}
			}
		}
		else
		{
			debug("error loading file");
		}
	}
}
*/
}
