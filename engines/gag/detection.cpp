#include "advancedDetector.h"
#include "detection_tables.h"
#include "gag/gag.h"



namespace Gag
{

static const PlainGameDescriptor neverhoodGames[] = {
	{"gag", "GAG: The Impotent Mystery"},
	{0, 0}
};


class GagMetaEngine : public AdvancedMetaEngine
{
public:
	GagMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(GagGameDescription), neverhoodGames) {}

	const char *getName() const
	{
		return "Gag";
	}

	const char *getOriginalCopyright() const
	{
		return "Gag Engine (C) 1997-1998 Auric Vision";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const
	{
		const GagGameDescription *gd = (const GagGameDescription *)desc;
		if(gd != NULL)
			*engine = new GagEngine(syst/*, gd*/);

		return gd != 0;
	}



//	private:
//		;
};
}



#if PLUGIN_ENABLED_DYNAMIC(GAG)
	REGISTER_PLUGIN_DYNAMIC(GAG, PLUGIN_TYPE_ENGINE, Gag::GagMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GAG, PLUGIN_TYPE_ENGINE, Gag::GagMetaEngine);
#endif
