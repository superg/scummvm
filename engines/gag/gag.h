#ifndef GAG_H__
#define GAG_H__



#include "common/archive.h"
#include "common/error.h"
#include "common/ptr.h"
#include "graphics/pixelformat.h"
#include "engines/engine.h"



namespace Gag
{

class GagEngine : public Engine
{
public:
	GagEngine(OSystem *syst);
	~GagEngine();

	virtual Common::Error run();

private:
	static const int m_SCREEN_WIDTH;
	static const int m_SCREEN_HEIGHT;
	static const Graphics::PixelFormat m_SCREEN_FORMAT;
	static const int m_SCREEN_FPS;

	Common::ScopedPtr<Common::Archive> m_Archive;

	void Initialize();
	Common::Error Run();
	Common::Error Update();

	void ExtractCdf(const Common::String &a_fn);

	//DEBUG
	void BitmapTest();
	void AnimationTest();
	void TestPlayAnimation(Common::String fn);

	void AnimationTuckerTest();
	void TestTuckerPlayAnimation(Common::String fn);
};

}



#endif
