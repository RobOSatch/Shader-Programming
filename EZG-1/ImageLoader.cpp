#include "imageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

unsigned char* ImageLoader::loadImageData(char const* filename, int* x, int* y, int* channelsInFile, int desiredChannels) {
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load(filename, x, y, channelsInFile, desiredChannels);
	if (!data)
	{
		std::cout << "Failed to load texture \"" << filename << "\"" << std::endl;
	}
	return data;;
}


void ImageLoader::freeImage(unsigned char* data)
{
	stbi_image_free(data);
}

void ImageLoader::setDefault2DTextureFromData(unsigned int texture, int texWidth, int texHeight, unsigned char* data)
{

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

