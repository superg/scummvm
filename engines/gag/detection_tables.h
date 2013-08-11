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
				{ "Gag01.cdf", 0, "d45300e994683cced978328b54ccd116", 421461821},
				{ "Gag02.cdf", 0, "ca055dd111cf79661471162ddc90d572", 628343005},
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
				{ "gag01.cdf", 0, "96313dddbf93fd1143f725eb9b9a913f", 422502015},
				{ "Gag02.cdf", 0, "b6dd5122cfc137dce4c255c25d05b8ea", 634529036},
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
