#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "gui/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"
#include "engines/util.h"

#include "video/flic_decoder.h"

#include "cdf_archive.h"
#include "gag.h"



namespace Gag
{

GagEngine::GagEngine(OSystem *syst)
	: Engine(syst)
{
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().

	// Do not initialize graphics here

	// However this is the place to specify all default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	// Here is the right place to set up the engine specific debug channels
	DebugMan.addDebugChannel(kQuuxDebugExample, "example", "this is just an example for a engine specific debug channel");
	DebugMan.addDebugChannel(kQuuxDebugExample2, "example2", "also an example");

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("quux");

	debug("GagEngine::GagEngine");
}


GagEngine::~GagEngine()
{
	debug("GagEngine::~GagEngine");

	// Dispose your resources here
	delete _rnd;

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}


Common::Error GagEngine::run()
{
	// Initialize graphics using following:
	initGraphics(320, 200, false);

	// You could use backend transactions directly as an alternative,
	// but it isn't recommended, until you want to handle the error values
	// from OSystem::endGFXTransaction yourself.
	// This is just an example template:
	//_system->beginGFXTransaction();
	//	// This setup the graphics mode according to users seetings
	//	initCommonGFX(false);
	//
	//	// Specify dimensions of game graphics window.
	//	// In this example: 320x200
	//	_system->initSize(320, 200);
	//FIXME: You really want to handle
	//OSystem::kTransactionSizeChangeFailed here
	//_system->endGFXTransaction();

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);

/*
//	Common::Archive *archive = new Common::FSDirectory(ConfMan.get("path") + '/' + "Gag01", 1, false);
	Common::Archive *archive = new CdfArchive("Gag01.cdf", false);
	Common::SeekableReadStream *srstream = archive->createReadStreamForMember("VERSIOn.TXT");

	if(srstream == NULL)
	{
		debug("error: failed to open file");
	}
	else
	{
		debug("file opened successfully");
	}
*/

	//TODO: find out how to create directory
//	ExtractCdf("Gag01.cdf");

	AnimationTest();

	// Additional setup.
	debug("GagEngine::init");

	// Your main even loop should be (invoked from) here.
	debug("GagEngine::go: Hello, World!");

	// This test will show up if -d1 and --debugflags=example are specified on the commandline
	debugC(1, kQuuxDebugExample, "Example debug call");

	// This test will show up if --debugflags=example or --debugflags=example2 or both of them and -d3 are specified on the commandline
	debugC(3, kQuuxDebugExample | kQuuxDebugExample2, "Example debug call two");

	return Common::kNoError;
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


void GagEngine::AnimationTest()
{
	Video::FlicDecoder flic_decoder;
	bool success = flic_decoder.loadFile("Gag01/CP0402.FLC");

	debug("video opened: %d", (int)success);

	debug("%d", flic_decoder.getCurFrame());
	debug("%d", flic_decoder.getFrameCount());
	;

	debug("video end");
/*

void AnimationSequencePlayer::getRGBPalette(int index) {
	memcpy(_animationPalette, _flicPlayer[index].getPalette(), 3 * 256);
}

void AnimationSequencePlayer::openAnimation(int index, const char *fileName) {
	if (!_flicPlayer[index].loadFile(fileName)) {
		warning("Unable to open flc animation file '%s'", fileName);
		_seqNum = 1;
		return;
	}
	_flicPlayer[index].start();
	_flicPlayer[index].decodeNextFrame();
	if (index == 0) {
		getRGBPalette(index);
		_flicPlayer[index].copyDirtyRectsToBuffer(_offscreenBuffer, kScreenWidth);
	}
}

bool AnimationSequencePlayer::decodeNextAnimationFrame(int index, bool copyDirtyRects) {
	const ::Graphics::Surface *surface = _flicPlayer[index].decodeNextFrame();

	if (!copyDirtyRects) {
		for (uint16 y = 0; (y < surface->h) && (y < kScreenHeight); y++)
			memcpy(_offscreenBuffer + y * kScreenWidth, (const byte *)surface->getBasePtr(0, y), surface->w);
	} else {
		_flicPlayer[index].copyDirtyRectsToBuffer(_offscreenBuffer, kScreenWidth);
	}

	++_frameCounter;

	if (index == 0 && _flicPlayer[index].hasDirtyPalette())
		getRGBPalette(index);

	return !_flicPlayer[index].endOfVideo();
}


	if (_flicPlayer[0].getCurFrame() >= 115) {
		surface = _flicPlayer[1].decodeNextFrame();
		if (_flicPlayer[1].endOfVideo())
			_flicPlayer[1].rewind();
	}

*/
}

}
