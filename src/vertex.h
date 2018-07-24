#ifndef VERTEX_H
#define VERTEX_H
#pragma once

#include <iostream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

// glfwのpragma文
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glewのpragma文
#pragma comment(lib,"glew32.lib")

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define BUFFER_OFFSET(bytes) ((GLubyte *)NULL + (bytes))

#include "FPSMajor.h"

class Vertex {

public:
	Vertex(unsigned int width, unsigned int height)
	{
		windowWidth = width;
		windowHeight = height;

		texel_x = 0.5f / (((float)windowWidth - 1.0f) * 2.0f);
		texel_y = 0.5f / (((float)windowHeight - 1.0f) * 2.0f);

		fps = new myUtl::FPSMajor(1);

		ready = false;
	};

	// 頂点情報の頂点座標とテクスチャ座標をまとめるための構造体
	virtual struct MyVertex{
		MyVertex(cv::Vec2f pos = cv::Vec2f(0, 0), cv::Vec2f tex = cv::Vec2f(0, 0))
		: Pos(pos), Tex(tex){};
		cv::Vec2f Pos;
		cv::Vec2f Tex;
	};

	// 三角メッシュの頂点インデックスを保持する構造体
	struct MyIndex_TRIANGLES{
		MyIndex_TRIANGLES(GLuint v1, GLuint v2, GLuint v3)
		{
			index[0] = v1;
			index[1] = v2;
			index[2] = v3;
		};
		GLuint index[3];
	};

	// 初期化
	// glewを初期化してから使用すること
	virtual void init();

	// メンバ変数のvertex-index buffer からVBOを生成する
	virtual void genVAO();

	// display関数内で呼ばれる関数
	virtual void render_mesh();

	// 終了時に呼ばれる関数
	virtual void close();

	virtual cv::Point2f getTexel(){ return texel_x, texel_y; };

	GLuint getVAO(){ return vao; };
	GLuint getVBO(){ return vertexbuffer; };
	GLuint getIBO(){ return indexbuffer; };
	bool getReady(){ return ready; }
	int  getVertexSize(){ return vertecies.size(); };


	myUtl::FPSMajor *fps;

private:
	unsigned int windowWidth;
	unsigned int windowHeight;

	// テクスチャマッピング時のピクセルとテクセルのずれ量
	float texel_x;
	float texel_y;

	// 頂点配列オブジェクト(0：幾何補正、1:幾何補正なし)
	GLuint vao;
	// 頂点バッファオブジェクト
	GLuint vertexbuffer;
	// インデックスバッファオブジェクト
	GLuint indexbuffer;
	//std::vector<GLuint> shift_vbo;

	// 頂点情報
	std::vector<MyVertex> vertecies;
	std::vector<MyIndex_TRIANGLES> indecies;

	// 描画するインデックス数
	int index_num;

	// 描画準備ができているかどうか
	bool ready;

};

#endif