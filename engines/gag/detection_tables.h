#ifndef GAG_DETECTION_TABLES_H__
#define GAG_DETECTION_TABLES_H__



#include "common/gui_options.h"



namespace Gag
{

struct GagGameDescription
{
	ADGameDescription desc;
};

static const GagGameDescription gameDescriptions[] =
{
	{
		// ГЭГ: Отвязное Приключение
		{
			"gag",
			0,
			{
				{ "Gag01.cdf", 0, "e12804933212bf1d0cdb475fce22f1e4", 421461821},
				{ "Gag02.cdf", 0, "f885d86a227028c04c0b50fb9c82529a", 628343005},
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_CD,
			GUIO0()
		}
	},

	{
		// GAG: The Impotent Mystery
		{
			"gag",
			"English version",
			{
				{ "gag01.cdf", 0, "8b9f9ce46ce212162e7ea65eb62eeadb", 422502015},
				{ "Gag02.cdf", 0, "dd9e339a077c522c8b2ab903f38e772f", 634529036},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_CD,
			GUIO0()
		}
	},

	{ AD_TABLE_END_MARKER }
};

}



#endif
