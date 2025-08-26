#include "includes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//BAD FIX https://github.com/nothings/stb/issues/1446
//#ifndef __STDC_LIB_EXT1__
//#define __STDC_LIB_EXT1__
//#define sprintf_s snprintf
//#endif
#define __STDC_LIB_EXT1__
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"