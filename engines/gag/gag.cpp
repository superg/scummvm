#include <limits>

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "engines/util.h"
//#include "graphics/decoders/bmp.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "video/flic_decoder.h"

#include "cdf_archive.h"
#include "gag_flic_decoder.h"
#include "gag.h"


//GGG
#include <vector>



namespace Gag
{

const int GagEngine::m_SCREEN_WIDTH(640);
const int GagEngine::m_SCREEN_HEIGHT(480);
const Graphics::PixelFormat GagEngine::m_SCREEN_FORMAT(2, 5, 6, 5, 0, 11, 5, 0, 0);
const int GagEngine::m_SCREEN_FPS(120);


GagEngine::GagEngine(OSystem *syst)
	: Engine(syst)
{
	;
}


GagEngine::~GagEngine()
{

}


Common::Error GagEngine::run()
{
	Initialize();

	Common::Error status(Run());

	return status;
}


void GagEngine::Initialize()
{
	initGraphics(m_SCREEN_WIDTH, m_SCREEN_HEIGHT, true, &m_SCREEN_FORMAT);

	//DEBUG
	m_Archive.reset(new Common::FSDirectory(ConfMan.get("path") + '/' + "Gag01", 2, false));

//	m_Archive.reset(new CdfArchive("Gag01.cdf", false));

//	ExtractCdf("Gag01.cdf");

//	BitmapTest();
	AnimationTest();
//	AnimationTuckerTest();
}


Common::Error GagEngine::Run()
{
	Common::Error status;

	do
	{
		// do periodic processing
		uint32 time_start = _system->getMillis();
		status = Update();
		uint32 time_end = _system->getMillis();

		// wrap around check
		uint32 time_spent = time_end > time_start ? time_end - time_start : std::numeric_limits<uint32>::max() - time_start + time_end + 1;

		// sleep remaining frame time
		_system->delayMillis(1000 / m_SCREEN_FPS - time_spent);
	}
	while(status.getCode() == Common::kNoError && !shouldQuit());

	return status;
}


Common::Error GagEngine::Update()
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
{/*
	Graphics::BitmapDecoder bitmap_decoder;
	Common::File file;
	file.open("AUTORUN.BMP", *m_Archive);
	if(bitmap_decoder.loadStream(file))
	{
		Graphics::Surface *surface = bitmap_decoder.getSurface()->convertTo(_system->getScreenFormat(), bitmap_decoder.getPalette());
		_system->copyRectToScreen((const byte *)surface->getPixels(), surface->pitch, 0, 0, surface->w, surface->h);
		_system->updateScreen();
		surface->free();
		delete surface;
	}*/
}


void GagEngine::AnimationTest()
{
//	TestPlayAnimation("LOGO1.MVZ");
//	TestPlayAnimation("WC0501.MOV");

	Common::ArchiveMemberList member_list;
	m_Archive->listMembers(member_list);
	for(Common::ArchiveMemberList::iterator it = member_list.begin(); it != member_list.end(); ++it)
	{
		if(/*!(*it)->getName().hasSuffix(".FLC") &&*/ !(*it)->getName().hasSuffix(".MOV") && !(*it)->getName().hasSuffix(".MVZ"))
			continue;

		debug("playing: %s", (*it)->getName().c_str());

		TestPlayAnimation((*it)->getName());
	}

	quitGame();
}


//#define ANIMATION_FAST_TEST


void GagEngine::TestPlayAnimation(Common::String fn)
{
	// file will be freed inside flic_decoder
	Common::File *file = new Common::File;
	if(file->open(fn, *m_Archive))
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

	quitGame();
}


void GagEngine::TestTuckerPlayAnimation(Common::String fn)
{
	// file will be freed inside flic_decoder
	Common::File *file = new Common::File;
	if(file->open(fn, *m_Archive))
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

}
