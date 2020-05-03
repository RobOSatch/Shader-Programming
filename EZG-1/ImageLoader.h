#pragma once

#include <ostream>
#include <iostream>
#include "glad/glad.h"
#include "glfw/glfw3.h"
static class ImageLoader
{
public:
	static unsigned char* loadImageData(char const* filename, int* x, int* y, int* channelsInFile, int desiredChannels);
	static void freeImage(unsigned char* data);
	static void setDefault2DTextureFromData(unsigned int texture, int texWidth, int texHeight, unsigned char* data);
};