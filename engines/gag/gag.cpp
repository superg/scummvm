#include <limits>
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include "engines/util.h"
//#include "graphics/decoders/bmp.h"
//#include "graphics/palette.h"
//#include "graphics/surface.h"
#include "cdf_archive.h"
#include "gag_flic_decoder.h"
#include "gag.h"


#define DEBUG_SKIM_SCRIPT

#ifdef DEBUG_SKIM_SCRIPT
#include <set>
std::set<Common::String> G_STRING_SET;
#endif



namespace Gag
{

const Graphics::PixelFormat GagEngine::_SCREEN_FORMAT(2, 5, 6, 5, 0, 11, 5, 0, 0);
const int GagEngine::_SCREEN_WIDTH(640);
const int GagEngine::_SCREEN_HEIGHT(480);
const int GagEngine::_SCREEN_FPS(60);
const Common::String GagEngine::_DEFAULT_SCRIPT("START.CFG");
const Common::String GagEngine::_DEFAULT_SECTION("CFG");
const Common::String GagEngine::_END_SECTION("END");
const char GagEngine::_OPTION_PREFIX = '/';
const char GagEngine::_ARGUMENT_DELIMITER = ':';



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
		uint time_for_frame = 1000 / _SCREEN_FPS;
		_system->delayMillis(time_spent < time_for_frame ? time_for_frame - time_spent : 0);
	}
	while(status.getCode() == Common::kNoError && !shouldQuit());

	return status;
}


void GagEngine::Init()
{
	initGraphics(_SCREEN_WIDTH, _SCREEN_HEIGHT, true, &_SCREEN_FORMAT);

	//DEBUG: load files from uncompressed file system
	_archive.reset(new Common::FSDirectory(ConfMan.get("path") + '/' + "Gag01", 2, false));
//	_archive.reset(new CdfArchive("Gag01.cdf", false));

	//DEBUG
#ifdef DEBUG_SKIM_SCRIPT
	_script = "GAG_CMD_CLEAN.CFG";
//	_section = "CFG";
	Common::Error script_error = StateScript();
//	debug("event options: ");
//	for(std::set<Common::String>::iterator it = G_STRING_SET.begin(); it != G_STRING_SET.end(); ++it)
//		debug("\t%s", it->c_str());
	quitGame();
	_state = GS_ACTIVE;
#endif

//	ExtractCdf("Gag01.cdf");
//	ExtractCdf("Gag02.cdf");
//	ExtractCdf("gag01.cdf");
//	ExtractCdf("GAG3.cdf");
//	ExtractCdf("GARY.cdf");
//	BitmapTest();
//	AnimationTest();
//	AnimationTuckerTest();
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
	if(file.open(_script, *_archive))
	{
		// simple script parsing: skim through, find section and execute commands
		ParserState parse_state(PS_SECTION_SEARCH);
		Common::String buffer;
		Common::String command_name;

		bool done(false);
		while(!file.eos())
		{
			// add space for multiline command support
			Common::String line = file.readLine() + ' ';
			if(file.err())
			{
				status = Common::Error(Common::kReadingFailed, _script + ", readLine()");
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
					{
#ifdef DEBUG_SKIM_SCRIPT
						parse_state = PS_SECTION_BODY;
#else
						if(buffer == _section)
							parse_state = PS_SECTION_BODY;
						else if(buffer == _END_SECTION)
						{
							status = Common::Error(Common::kUnknownError, "[" + _section + "] script section not found");
							skip_line = true;
							done = true;
						}
						else
							parse_state = PS_SECTION_SEARCH;
#endif
					}
					else
						buffer += *it;
					break;

				case PS_SECTION_BODY:
					// section
					if(*it == '[')
					{
#ifdef DEBUG_SKIM_SCRIPT
						buffer.clear();
						parse_state = PS_SECTION_NAME;
#else
						skip_line = true;
						done = true;
#endif
					}
					// comment
					else if(*it == '*')
					{
						skip_line = true;
					}
					//NOTE: invalid syntax
					else if(*it == '-' || *it == '/' || (*it >= 'A' && *it <= 'Z'))
					{
#ifndef DEBUG_SKIM_SCRIPT
						warning("invalid script syntax [file: %s, section: %s, line: \"%s\"], skipped", _script.c_str(), _section.c_str(), line.c_str());
#endif
						skip_line = true;
					}
					// command name
					else if((*it >= 'a' && *it <= 'z'))
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
						command_name.trim();
						buffer.trim();

#ifndef DEBUG_SKIM_SCRIPT
						debug("%s=%s;", command_name.c_str(), buffer.c_str());
#endif

						Common::HashMap<Common::String, Common::Error (GagEngine::*)(const Common::String &)>::const_iterator f = _commandCallbacks.find(command_name);
						if(f != _commandCallbacks.end())
						{
							status = (this->*f->_value)(buffer);
							if(status.getCode() != Common::kNoError)
							{
								skip_line = true;
								done = true;
							}
						}

						parse_state = PS_SECTION_BODY;
					}
					else
						buffer += *it;
					break;
				}

				if(skip_line)
					break;
			}

			if(done)
				break;
		}
	}
	else
	{
		status = Common::Error(Common::kReadingFailed, _script + ", open()");
	}

	return status;
}


Common::String GagEngine::ParseOption(Common::Array<Common::String> &arguments, Common::String option)
{
	//NOTE: unable to use StringTokenizer here, it doesn't support empty token extraction
#define EVENT_OPTIONS_TOKENIZE
#ifdef EVENT_OPTIONS_TOKENIZE
	Common::StringTokenizer tokenizer(option, Common::String(_ARGUMENT_DELIMITER));

	Common::String name(tokenizer.nextToken());

	while(!tokenizer.empty())
		arguments.push_back(tokenizer.nextToken());

	return name;
#else
	// make things easier
	option += _ARGUMENT_DELIMITER;
	const char *o = option.c_str();

	for(const char *p = o; *p != '\0'; ++p)
	{
		if(*p == _ARGUMENT_DELIMITER)
		{
			arguments.push_back(Common::String(o, p));
			o = p + 1;
		}
	}

	// separate option name
	Common::String name(arguments.front());
	arguments.remove_at(0);

	return name;
#endif
}


Common::Error GagEngine::ScriptCatch(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Catch] not implemented");
#endif
}


Common::Error GagEngine::ScriptClass(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Class] not implemented");
#endif
}


Common::Error GagEngine::ScriptCommand(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Command] not implemented");
#endif
}


Common::Error GagEngine::ScriptDriveSpeed(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script DriveSpeed] not implemented");
#endif
}


Common::Error GagEngine::ScriptEvent(const Common::String &value)
{
	Common::StringTokenizer tokenizer(value, Common::String(_OPTION_PREFIX));

	Event event;
	event.name = tokenizer.nextToken();
	event.name.trim();
	if(event.name.empty())
		return Common::Error(Common::kUnknownError, "[Script Event] unnamed event");

	if(tokenizer.empty())
		return Common::Error(Common::kUnknownError, "[Script Event] no options");

	do
	{
		Common::String option(tokenizer.nextToken());
		option.trim();
		Common::Array<Common::String> option_arguments;
		Common::String option_name = ParseOption(option_arguments, option);

		if(option_name == "PRELOAD")
		{
			switch(option_arguments.size())
			{
			case 1:
				event.section = option_arguments[0];
				break;

			case 2:
				;
				break;

			case 3:
				event.script = option_arguments[0];
				event.section = option_arguments[1];
				event.transition_mode = option_arguments[2];
				break;

			default:
				return Common::Error(Common::kUnknownError, "[Script Event] PRELOAD: invalid option arguments");
			}
		}
		else
		{
//			warning("[Script Event] option %s is unsupported", option_name.c_str());
		}

		//DEBUG
#ifdef DEBUG_SKIM_SCRIPT
		G_STRING_SET.insert(option_name);
		if(option_name == "PRELOAD")
		{
			debug("%s:%s:%s",
				  option_arguments.size() >= 1 ? option_arguments[0].c_str() : "",
				  option_arguments.size() >= 2 ? option_arguments[1].c_str() : "",
				  option_arguments.size() >= 3 ? option_arguments[2].c_str() : ""
 				);
//			for(uint i = 0; i < option_arguments.size(); ++i)
//				cout << ':' << option_arguments[i].c_str();
//			cout << endl;
//			debug("%s", option.c_str());
		}
#endif
	}
	while(!tokenizer.empty());

	_events.push_back(event);

	return Common::Error(Common::kNoError);
}


Common::Error GagEngine::ScriptException(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Exception] not implemented");
#endif
}


Common::Error GagEngine::ScriptExFiles(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script ExFiles] not implemented");
#endif
}


Common::Error GagEngine::ScriptFadeMask(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script FadeMask] not implemented");
#endif
}


Common::Error GagEngine::ScriptFlags(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Flags] not implemented");
#endif
}


Common::Error GagEngine::ScriptFont(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Font] not implemented");
#endif
}


Common::Error GagEngine::ScriptImage(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Image] not implemented");
#endif
}


Common::Error GagEngine::ScriptInventory(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Inventory] not implemented");
#endif
}


Common::Error GagEngine::ScriptLanguage(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Language] not implemented");
#endif
}


Common::Error GagEngine::ScriptLayer(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Layer] not implemented");
#endif
}


Common::Error GagEngine::ScriptList(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script List] not implemented");
#endif
}


Common::Error GagEngine::ScriptLoad(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Load] not implemented");
#endif
}


Common::Error GagEngine::ScriptLocal(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Local] not implemented");
#endif
}


Common::Error GagEngine::ScriptMouse(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Mouse] not implemented");
#endif
}


Common::Error GagEngine::ScriptObject(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Object] not implemented");
#endif
}


Common::Error GagEngine::ScriptParams(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Params] not implemented");
#endif
}


Common::Error GagEngine::ScriptPath(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Path] not implemented");
#endif
}


Common::Error GagEngine::ScriptSource(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Source] not implemented");
#endif
}


Common::Error GagEngine::ScriptSubLocation(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script SubLocation] not implemented");
#endif
}


Common::Error GagEngine::ScriptTemplate(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Template] not implemented");
#endif
}


Common::Error GagEngine::ScriptText(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Text] not implemented");
#endif
}


Common::Error GagEngine::ScriptTry(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Try] not implemented");
#endif
}


Common::Error GagEngine::ScriptVolume(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Volume] not implemented");
#endif
}


Common::Error GagEngine::ScriptZone(const Common::String &value)
{
#ifdef DEBUG_SKIM_SCRIPT
	return Common::Error(Common::kNoError);
#else
	return Common::Error(Common::kUnknownError, "[Script Zone] not implemented");
#endif
}




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

/*
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
