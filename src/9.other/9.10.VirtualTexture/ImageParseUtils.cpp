#include "ImageParseUtils.h"

ImageContainer* ImageParseUtils::ImageParse(const ImageContainer& _data, EPixelFormat _dstFormat,EPixelFormat _srcFormat)
{
	if (_dstFormat == _srcFormat)
	{
		return nullptr;
	}
}