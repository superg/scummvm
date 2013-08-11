#ifndef GAG_H__
#define GAG_H__



#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"



namespace Gag
{

class Console;

// our engine debug channels
enum
{
	kQuuxDebugExample = 1 << 0,
	kQuuxDebugExample2 = 1 << 1
	// next new channel must be 1 << 2 (4)
	// the current limitation is 32 debug channels (1 << 31 is the last one)
};

class GagEngine : public Engine {
public:
	GagEngine(OSystem *syst);
	~GagEngine();

	virtual Common::Error run();

private:
	Console *_console;

	// We need random numbers
	Common::RandomSource *_rnd;
};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(GagEngine *vm) {}
	virtual ~Console(void) {}
};

}



#endif
