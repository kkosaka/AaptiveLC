#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <iostream>

using namespace std;

//テクスチャオブジェクトを作成、管理するクラス
class TextureObjectStocker
{
protected:
	int texWidth, texHeight;		//テクスチャの幅と高さ
	GLint internalFmt;					//テクスチャの内部フォーマット

	vector<GLuint> usingTextures;		//使用中のテクスチャオブジェクト
	vector<GLuint> unusingTextures;		//未使用のテクスチャオブジェクト


public:
	//コンストラクタ
	//width, height: テクスチャの幅と高さ
	//internalFormat: テクスチャの内部フォーマット(GL_RGB, GL_RGBA32Fなど)
	TextureObjectStocker(int width, int height, GLint internalFormat);
	~TextureObjectStocker();

	//テクスチャオブジェクトを取得する
	//(未使用のテクスチャオブジェクトがあればそれを返し、無ければ新たに作成する)
	GLuint getTextureObject();

	//使用しなくなったテクスチャオブジェクトを帰す
	//(未使用リストに登録されてgetTextureObject()を呼び出した際に再利用される)
	void releaseTextureObject(GLuint texobj);

	//テクスチャに画像をアップロードする
	//(このクラスで管理しているテクスチャオブジェクトのみに対して有効)
	//texobj: テクスチャオブジェクト
	//format: pixelsのメモリ上の画像形式 (GL_RGB, GL_BGR, GL_RGBAなど)
	//type: pixelsの要素のデータ型 (GL_UNSIGNED_BYTE, GL_FLOATなど)
	void uploadToTexture(GLuint texobj, GLenum format, GLenum type, const GLvoid *pixels);

	void bind(GLuint texobj, GLenum TextureUnit);
	void debug();

};