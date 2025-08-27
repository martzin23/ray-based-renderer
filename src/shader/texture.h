#pragma once
#include "../utility/includes.h"
#include "../utility/stb_image.h"

class Texture {
private:
	unsigned int texture;
	unsigned int unit;
public:
	Texture(const char* filepath, unsigned int unit, int gl_interpolation);
	~Texture();
	void Bind();
};