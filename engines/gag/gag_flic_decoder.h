#ifndef GAG_FLIC_DECODER_H__
#define GAG_FLIC_DECODER_H__



#include "video/flic_decoder.h"



namespace Gag
{

class GagFlicDecoder : public Video::FlicDecoder
{
public:
	GagFlicDecoder();
	virtual ~GagFlicDecoder();
};

}



#endif
