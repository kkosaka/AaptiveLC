#include "PS_Vertex.h"

// constractor
PS_Vertex::PS_Vertex(unsigned int window_w, unsigned int window_h) : Vertex(window_w, window_h)
{
	windowWidth = window_w;
	windowHeight = window_h;
}


PS_Vertex::~PS_Vertex()
{
}

void PS_Vertex::setBuffers(std::vector<MyVertex> _vertecies, std::vector<MyIndex_TRIANGLES> _indecies)
{

	if (ready){
		// 頂点配列オブジェクトを削除する
		glDeleteVertexArrays(1, &vao);
		// 頂点バッファオブジェクトを削除する
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &indexbuffer);
		glDeleteBuffers(1, &vbo_offset_copy);
	}

	vertex_num = _vertecies.size();

	std::vector<cv::Vec2f> offsetVec;
	std::vector<GLfloat> status;
	for (int i = 0; i < vertex_num; ++i){
		offsetVec.emplace_back(cv::Vec2f(0.0f, 0.0f));
		status.emplace_back(255.0f);
	}

	// 頂点配列オブジェクトの生成
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// 頂点バッファオブジェクトの生成
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(MyVertex), &_vertecies[0], GL_DYNAMIC_DRAW);

	// オフセット用頂点バッファオブジェクトの生成
	glGenBuffers(1, &vbo_offset);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(cv::Vec2f), &offsetVec[0], GL_DYNAMIC_DRAW);

	// オフセットの結果用頂点バッファオブジェクト(コピー用)の生成
	glGenBuffers(1, &vbo_offset_result);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_result);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(cv::Vec2f), nullptr, GL_DYNAMIC_COPY);

	// コピー用頂点バッファオブジェクトの生成
	glGenBuffers(1, &vbo_offset_copy);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(cv::Vec2f), nullptr, GL_DYNAMIC_COPY);

	// 状態管理用の頂点バッファオブジェクトの生成
	glGenBuffers(1, &vbo_status);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_status);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);

	// インデックスバッファオブジェクトの生成
	GLuint size;
	index_num = (int)(_indecies.size() * sizeof(MyIndex_TRIANGLES) / sizeof(GLuint));
	glGenBuffers(1, &indexbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indecies.size() * sizeof(MyIndex_TRIANGLES), &_indecies[0], GL_STATIC_DRAW);

	// マスク用
	VBO_mask_camsize = new glUtl::glBuffer<GLfloat>(GL_ARRAY_BUFFER, vertex_num, 0, GL_DYNAMIC_COPY);
	
	// この頂点配列オブジェクトの結合を解除する
	glBindVertexArray(0);

	// transformFeedback用のオブジェクトを生成する
	glGenTransformFeedbacks(1, &feedback);

	
	setDefaultVertexPointer();

	ready = true;
}
void PS_Vertex::setCameraIndex(std::vector<float> _cameraRefer)
{
	if (!ready) {
		std::cerr << "error vbo for camera index is not generate" << std::endl;
		return;
	}

	//glBindVertexArray(vao);
	//// 頂点バッファオブジェクトの生成
	//glGenBuffers(1, &vbo_camIndex);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo_camIndex);
	//glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(float), &_cameraRefer[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
}

// 描画処理
void PS_Vertex::render_mesh()
{
	if (!ready){
		std::cout << "PS_Vertex.cpp -> draw() : error" << std::endl;
		return;
	}

	glBindVertexArray(vao);
	glEnableVertexAttribArray(LOCATION_PROJECTOR);
	glEnableVertexAttribArray(LOCATION_CAMERA);
	glEnableVertexAttribArray(LOCATION_CAMERAOFFSET);

	glDrawElements(GL_TRIANGLES, index_num, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(LOCATION_PROJECTOR);
	glDisableVertexAttribArray(LOCATION_CAMERA);
	glDisableVertexAttribArray(LOCATION_CAMERAOFFSET);
	glBindVertexArray(0);

}
void PS_Vertex::render_point()
{
	if (!ready){
		std::cout << "PS_Vertex.cpp -> draw() : error" << std::endl;
		return;
	}

	glBindVertexArray(vao);
	glEnableVertexAttribArray(LOCATION_PROJECTOR);
	glEnableVertexAttribArray(LOCATION_CAMERA);
	glEnableVertexAttribArray(LOCATION_CAMERAOFFSET);

	glDrawArrays(GL_POINTS, 0, vertex_num);

	glDisableVertexAttribArray(LOCATION_PROJECTOR);
	glDisableVertexAttribArray(LOCATION_CAMERA);
	glDisableVertexAttribArray(LOCATION_CAMERAOFFSET);
}

// 終了処理
void PS_Vertex::close()
{
	// 頂点配列オブジェクトを削除する
	glDeleteVertexArrays(1, &vao);

	// 頂点バッファオブジェクトを削除する
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &vbo_offset);
	glDeleteBuffers(1, &indexbuffer);
	glDeleteBuffers(shift_num, vbo_shift);
}


//void PS_Vertex::copyVertexBuffer()
//{
//	// 修正する頂点バッファをバインド
//	glBindVertexArray(vao);
//	GLint size = 0;
//	glBindBuffer(GL_COPY_READ_BUFFER, vbo);
//	glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);
//
//	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo_offset_copy);
//	glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
//
//	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);
//
//	glBindVertexArray(0);
//}

// エピポーラ線への変位量をVBOに格納する
void PS_Vertex::setShiftVector(std::vector<cv::Mat> &_shiftMatVectors)
{
	//std::cout << "modify vbo" << std::endl;
	shift_num = _shiftMatVectors.size();
	if (ready != true)
		return;

	std::cout << _shiftMatVectors[0].size().area() << ", " << vertex_num << std::endl;
	if (_shiftMatVectors[0].size().area() != vertex_num)
		return;

	// 頂点配列オブジェクトをバインド
	glBindVertexArray(vao);

	// 頂点バッファオブジェクトの生成
	vbo_shift = new GLuint[shift_num];
	// 移動量のVBOを生成
	for (int i = 0; i < shift_num; ++i){
		glGenBuffers(1, &vbo_shift[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_shift[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cv::Vec2f) * vertex_num, _shiftMatVectors[i].data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// この頂点配列オブジェクトの結合を解除する
	glBindVertexArray(0);
}

// 画素対応を数ピクセルエピポーラ線方向にシフトさせる
void PS_Vertex::setShaderLocationShiftVector(bool isEnabled, GLuint num)
{
	if (ready != true)
		return;

	if (num >= shift_num)
		return;

	// 頂点配列オブジェクトをバインド
	glBindVertexArray(vao);

	// 画素対応を移動させる場合
	if (isEnabled){
		// 頂点バッファオブジェクトをバインド
		glBindBuffer(GL_ARRAY_BUFFER, vbo_shift[num]);
		// バーテックスシェーダのロケーション番号3に設定
		glVertexAttribPointer(LOCATION_CAMERASHIFT, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location番号, サイズ, 型, 正規化(法線), ストライド, バッファオフセット
		// 頂点バッファオブジェクトをバインドを解除する
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(LOCATION_CAMERASHIFT);
	}
	else{
		glDisableVertexAttribArray(LOCATION_CAMERASHIFT);
	}

	// この頂点配列オブジェクトの結合を解除する
	glBindVertexArray(0);
}

void PS_Vertex::setDefaultVertexPointer()
{
	// 頂点配列オブジェクトを結合
	glBindVertexArray(vao);

	// 幾何変換用の頂点バッファオブジェクトを結合
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// バーテックスシェーダの0番目のattribute変数を取り出す(location = 0)
	glVertexAttribPointer(LOCATION_PROJECTOR, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void*)0); // location番号, サイズ, 型, 正規化, ストライド, バッファオフセット
	// バーテックスシェーダの1番目のattribute変数を取り出す(location = 1)
	glVertexAttribPointer(LOCATION_CAMERA, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), BUFFER_OFFSET(sizeof(cv::Vec2f)));

	// オフセット用の頂点バッファオブジェクトを結合
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset);
	// バーテックスシェーダの2番目のattribute変数を取り出す(location = 2)
	glVertexAttribPointer(LOCATION_CAMERAOFFSET, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location番号, サイズ, 型, 正規化, ストライド, バッファオフセット

	// 頂点配列オブジェクトを解除
	glBindVertexArray(0);
}

void PS_Vertex::feedbackCalclateForGetOffsetResult()
{
	//-----バッファのコピー-------

	// オフセット用の頂点バッファオブジェクトをコピーする
	glBindVertexArray(vao);
	glBindBuffer(GL_COPY_READ_BUFFER, vbo_offset_result);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, vertex_num * sizeof(cv::Vec2f));
	glBindVertexArray(0);

	//-----バッファの計算-------

	// 頂点配列オブジェクトを結合
	glBindVertexArray(vao);

	glEnableVertexAttribArray(LOCATION_PROJECTOR);
	glEnableVertexAttribArray(LOCATION_CAMERASHIFT);
	glEnableVertexAttribArray(LOCATION_OFFSETRESULT);

	// ラスタライザを止めて計算を実行する
	glEnable(GL_RASTERIZER_DISCARD);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);
	// 求まったオフセット値を格納するための頂点バッファオブジェクトを出力ターゲットとして指定する
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo_offset_result);
	// Transform Feedback 開始
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, vertex_num);

	// Transform Feedback 終了
	glEndTransformFeedback();

	//// テクスチャバッファオブジェクトのテクスチャを解除する
	//glBindTexture(GL_TEXTURE_BUFFER, 0);
	glDisable(GL_RASTERIZER_DISCARD);


	glEnableVertexAttribArray(LOCATION_PROJECTOR);
	glDisableVertexAttribArray(LOCATION_CAMERASHIFT);
	glEnableVertexAttribArray(LOCATION_OFFSETRESULT);

	// 頂点配列オブジェクトを解除
	glBindVertexArray(0);

}

void PS_Vertex::feedbackSetVertexPointerForGetOffsetResult(unsigned int num)
{
	//-----バーテックスポインターの設定-------

	// 頂点配列オブジェクトを結合
	glBindVertexArray(vao);

	// オフセットのコピーの頂点バッファオブジェクトを結合
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	// バーテックスシェーダの1番目のattribute変数を取り出す(location = 1)
	glVertexAttribPointer(LOCATION_OFFSETRESULT, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location番号, サイズ, 型, 正規化, ストライド, バッファオフセット

	// 頂点配列オブジェクトを解除
	glBindVertexArray(0);

}

void PS_Vertex::feedbackSetVertexPointerForSetNewOffset()
{
	//-----バーテックスポインターの設定-------

	// 頂点配列オブジェクトを結合
	glBindVertexArray(vao);

	// 求めたオフセット値の頂点バッファオブジェクトを結合
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	// バーテックスシェーダの0番目のattribute変数を取り出す(location = 0)
	glVertexAttribPointer(LOCATION_CAMERAOFFSET, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location番号, サイズ, 型, 正規化, ストライド, バッファオフセット

	// 求めたオフセット値の頂点バッファオブジェクトを結合
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_result);
	// バーテックスシェーダの0番目のattribute変数を取り出す(location = 0)
	glVertexAttribPointer(LOCATION_OFFSETRESULT, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location番号, サイズ, 型, 正規化, ストライド, バッファオフセット

	// 頂点配列オブジェクトを解除
	glBindVertexArray(0);

}

void PS_Vertex::feedbackCalclateForSetNewOffset()
{

	//-----バッファのコピー-------

	// オフセット用の頂点バッファオブジェクトをコピーする
	glBindVertexArray(vao);
	glBindBuffer(GL_COPY_READ_BUFFER, vbo_offset);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, vertex_num * sizeof(cv::Vec2f));
	glBindVertexArray(0);

	//-----バッファの計算-------

	// 頂点配列オブジェクトを結合
	glBindVertexArray(vao);

	glEnableVertexAttribArray(LOCATION_CAMERAOFFSET);
	glEnableVertexAttribArray(LOCATION_OFFSETRESULT);

	// ラスタライザを止めて計算を実行する
	glEnable(GL_RASTERIZER_DISCARD);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);
	// オフセット値の頂点バッファオブジェクトを出力ターゲットとして指定する
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo_offset);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, vbo_status);
	// Transform Feedback 開始
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, vertex_num);

	// Transform Feedback 終了
	glEndTransformFeedback();

	//// テクスチャバッファオブジェクトのテクスチャを解除する
	glDisable(GL_RASTERIZER_DISCARD);

	glDisableVertexAttribArray(LOCATION_CAMERAOFFSET);
	glDisableVertexAttribArray(LOCATION_OFFSETRESULT);

	// 頂点配列オブジェクトを解除
	glBindVertexArray(0);
}


void PS_Vertex::feedbackCalclateForBlur(GLuint vbo)
{

	//-----バーテックスポインターの設定-------

	// 頂点配列オブジェクトを結合
	glBindVertexArray(vao);

	// 求めたオフセット値の頂点バッファオブジェクトを結合
	glBindBuffer(GL_ARRAY_BUFFER, VBO_mask_camsize->buf());
	// バーテックスシェーダの0番目のattribute変数を取り出す(location = 0)
	glVertexAttribPointer(0, 1, GL_INT, GL_FALSE, sizeof(GLint), (void*)0); // location番号, サイズ, 型, 正規化, ストライド, バッファオフセット

	// 頂点配列オブジェクトを解除
	glBindVertexArray(0);

	//-----バッファの計算-------

	// 頂点配列オブジェクトを結合
	glBindVertexArray(vao);

	// ラスタライザを止めて計算を実行する
	glEnable(GL_RASTERIZER_DISCARD);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);
	// オフセット値の頂点バッファオブジェクトを出力ターゲットとして指定する
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo);
	// Transform Feedback 開始
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, vertex_num);

	// Transform Feedback 終了
	glEndTransformFeedback();

	//// テクスチャバッファオブジェクトのテクスチャを解除する
	glDisable(GL_RASTERIZER_DISCARD);


	// 頂点配列オブジェクトを解除
	glBindVertexArray(0);
}

/*
*	対応マップを初期化する関数
*/
void PS_Vertex::resetOffsetVBO()
{
	std::vector<cv::Vec2f> offsetVec;
	for (int i = 0; i < vertex_num; ++i){
		offsetVec.emplace_back(cv::Vec2f(0.0f, 0.0f));
	}
	// オフセット用の頂点バッファオブジェクトをコピーする
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_num * sizeof(cv::Vec2f), &offsetVec[0]);
	glBindVertexArray(0);
}