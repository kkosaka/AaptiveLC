#ifndef OFFSCREENRENDER_H
#define OFFSCREENRENDER_H

#pragma once

#include <iostream>
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#pragma comment (lib,"glew32.lib")

class offscreenRender
{
public:
	offscreenRender()
	{
		ready = false;
	};
	~offscreenRender()
	{
		glDeleteFramebuffers(1, &fboID);
		glDeleteTextures(2, texID);
		glDeleteRenderbuffers(2, rboID);
	};

	bool init(int width, int height);

	// オフスクリーンレンダリングのon/off
	void setEnabled(bool enabled);

	// 指定のアタッチメント先にテクスチャを登録する
	void attachmentTexture(GLenum ATTACHMENT, GLenum target, GLenum id);
	
	// 指定のアタッチメント先にレンダーバッファ登録する
	void attachmentRenderBuffer(GLenum ATTACHMENT, GLenum target, GLenum id);
	
	void clearAttachment();

	// 描画する
	void drawBuffers();

	int getWidth(){ return offscreen_width; };
	int getHeight(){ return offscreen_height; };
	GLuint getFbo(){ return fboID; };
	
private:
	// メンバ変数
	GLuint fboID;
	GLuint rboID[2];
	GLuint texID[2];

	// オフスクリーン先のスクリーンサイズ
	int offscreen_width;
	int offscreen_height;

	bool ready;

	// フレームバッファのステータスをチェックする関数
	bool checkFramebufferStatus();

	// アタッチメントがすでに登録されているか調べる関数
	bool checkAttachment(GLenum ATTACHMENT);

	// レンダリング先を登録するベクター
	std::vector<GLenum> renderTargets;

};

#endif
