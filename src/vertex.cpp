#include "vertex.h"

// テクスチャ描画用のフレームサイズのポリゴンを用意
void Vertex::init()
{
	vertecies.clear();
	indecies.clear();

	// ピクセル値をずらすための変数
	//texel_x = 0.0f;
	//texel_y = 0.0f;

	// 頂点座標の設定
	GLfloat g_vertex_buffer_data[][2] = {
		{ -1.0f, 1.0f},	// 左上 0
		{ -1.0f, -1.0f },	// 左下 1
		{ 1.0f, 1.0f },	// 右上 2
		{ 1.0f, -1.0f },	// 右下 3
	};
	//GLfloat g_vertex_buffer_data[][2] = {
	//	{ 0.0f, 0.0f },	// 左下 1
	//	{ 0.0f, (float)windowHeight },// 左上 0
	//	{ (float)windowWidth, 0.0f },// 右下 3
	//	{ (float)windowWidth, (float)windowHeight },// 右上 2
	//};
	int vertex_num = sizeof(g_vertex_buffer_data) / sizeof(g_vertex_buffer_data[0]);
	// 各頂点座標の値と，それに対応するテクスチャ座標を配列に格納
	for (int i = 0; i < vertex_num; ++i) {
		//// 頂点座標
		//cv::Vec2f pos = cv::Vec2f(g_vertex_buffer_data[i]);
		//vertecies.emplace_back(pos);
		cv::Vec2f pos = cv::Vec2f(g_vertex_buffer_data[i]);
		// ピクセルとテクセルの違いを考慮し，座標値をずらす
		cv::Vec2f pos_texel = cv::Vec2f(pos[0] - texel_x, pos[1] - texel_y);
		// 頂点座標に対応するテクスチャ座標を計算
		cv::Vec2f tex = cv::Vec2f(pos[0] * 0.5f + 0.5f, pos[1] * 0.5f + 0.5f);
		// 配列に格納
		vertecies.emplace_back(pos_texel, tex);
	}

	// 頂点座標に対する描画順を設定(三角メッシュで表現)
	GLuint g_index_buffer_data_triangles[][3] = {
		{ 0, 1, 2 },
		{ 3, 2, 1 },
	};
	int num = sizeof(g_index_buffer_data_triangles) / sizeof(g_index_buffer_data_triangles[0]);
	for (int i = 0; i < num; ++i) {
		indecies.emplace_back(
			MyIndex_TRIANGLES(
			g_index_buffer_data_triangles[i][0],
			g_index_buffer_data_triangles[i][1],
			g_index_buffer_data_triangles[i][2]
			)
			);
	}

	this->genVAO();
}

void Vertex::genVAO()
{
	if (ready){
		// 頂点配列オブジェクトを削除する
		glDeleteVertexArrays(1, &vao);
		// 頂点バッファオブジェクトを削除する
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &indexbuffer);
	}

	// 頂点配列オブジェクトの生成
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// 頂点バッファオブジェクトの生成
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MyVertex)* vertecies.size(), &vertecies[0], GL_DYNAMIC_DRAW);

	// バーテックスシェーダの０番目のattribute変数を取り出す(location = 0)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void*)0); // location番号, サイズ, 型, 正規化, ストライド, バッファオフセット
	// バーテックスシェーダの１番目のattribute変数を取り出す(location = 1)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), BUFFER_OFFSET(sizeof(cv::Vec2f)));

	// インデックスバッファオブジェクトの生成
	index_num = (int)(indecies.size() * sizeof(MyIndex_TRIANGLES) / sizeof(GLuint));
	glGenBuffers(1, &indexbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indecies.size() * sizeof(MyIndex_TRIANGLES), &indecies[0], GL_STATIC_DRAW);

	// この頂点配列オブジェクトの結合を解除する
	glBindVertexArray(0);

	ready = true;

}

void Vertex::render_mesh()
{
	if (!ready){
		std::cout << "vertex.cpp -> draw() : error" << std::endl;
		return;
	}

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawElements(GL_TRIANGLES, index_num, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

}


void Vertex::close()
{
	// 頂点配列オブジェクトを削除する
	glDeleteVertexArrays(1, &vao);

	// 頂点バッファオブジェクトを削除する
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &indexbuffer);
}

