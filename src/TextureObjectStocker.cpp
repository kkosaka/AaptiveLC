#include "TextureObjectStocker.h"
#include <algorithm>

TextureObjectStocker::TextureObjectStocker(int width, int height, GLint internalFormat)
{
	texWidth = std::max(1, width);
	texHeight = std::max(1, height);
	internalFmt = internalFormat;
	usingTextures.clear();
	unusingTextures.clear();
}

TextureObjectStocker::~TextureObjectStocker()
{
	if (usingTextures.size() > 0)
	{
		glDeleteTextures(usingTextures.size(), &usingTextures[0]);
		usingTextures.clear();
	}

	if (unusingTextures.size() > 0)
	{
		glDeleteTextures(unusingTextures.size(), &unusingTextures[0]);
		unusingTextures.clear();
	}
}

GLuint TextureObjectStocker::getTextureObject()
{
	GLuint texobj;

	if (unusingTextures.size() > 0)
	{
		texobj = unusingTextures.back();
		unusingTextures.pop_back();
	}
	else
	{
		glGenTextures(1, &texobj);

		glBindTexture(GL_TEXTURE_2D, texobj);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	usingTextures.push_back(texobj);

	return texobj;
}

void TextureObjectStocker::releaseTextureObject(GLuint texobj)
{
	for (vector<GLuint>::iterator it = usingTextures.begin(); it != usingTextures.end(); it++)
	{
		if (*it == texobj)
		{
			usingTextures.erase(it);
			unusingTextures.push_back(texobj);
			return;
		}
	}

	cerr << "Error: usingTexturesにないテクスチャオブジェクトがreleaseされました (texobj = " << texobj << ")" << endl;
}

void TextureObjectStocker::uploadToTexture(GLuint texobj, GLenum format, GLenum type, const GLvoid *pixels)
{
	//このクラスが管理しているテクスチャオブジェクトかどうか調べる
	for (int i = 0; i < usingTextures.size(); i++)
	{
		if (usingTextures[i] == texobj)
		{
			glBindTexture(GL_TEXTURE_2D, texobj);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, format, type, pixels);
			glBindTexture(GL_TEXTURE_2D, 0);

			return;
		}
	}

	cerr << "Error: usingTexturesにないテクスチャオブジェクトがuploadされました (texobj = " << texobj << ")" << endl;
}



void TextureObjectStocker::bind(GLuint texobj, GLenum TextureUnit)
{
	//このクラスが管理しているテクスチャオブジェクトかどうか調べる
	for (int i = 0; i < usingTextures.size(); i++)
	{
		if (usingTextures[i] == texobj)
		{
			glActiveTexture(TextureUnit);
			glBindTexture(GL_TEXTURE_2D, texobj);

			return;
		}
	}
	cerr << "Error: usingTexturesにないテクスチャオブジェクトがbindされました (texobj = " << texobj << ")" << endl;
}

#include <iostream>
void TextureObjectStocker::debug()
{
	cout << "using: " << usingTextures.size();
	//for (int i = 0; i < usingTextures.size(); i++)
	//	cout << usingTextures[i] << "  ";
	
	cout << endl << "unusing: " << unusingTextures.size();
	//for (int i = 0; i < unusingTextures.size(); i++)
	//	cout << unusingTextures[i] << "  ";

	cout << endl;
}