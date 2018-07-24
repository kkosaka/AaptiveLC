#pragma once
#include "vertex.h"
#include "glUtl.h"

#define LOCATION_PROJECTOR			0
#define LOCATION_CAMERA				1
#define LOCATION_CAMERAOFFSET		2
#define LOCATION_CAMERASHIFT		3
#define LOCATION_OFFSETRESULT		4

class PS_Vertex :
	public Vertex
{
public:
	PS_Vertex(unsigned int window_w, unsigned int window_h);
	~PS_Vertex();

	// 頂点情報の頂点座標とテクスチャ座標をまとめるための構造体
	virtual struct MyVertex{
		MyVertex(cv::Vec2f proj = cv::Vec2f(0.0f, 0.0f), cv::Vec2f cam = cv::Vec2f(0.0f, 0.0f))
		: Proj(proj), Cam(cam){};
		cv::Vec2f Proj;
		cv::Vec2f Cam;
	};

	// (オーバーライド)
	void render_mesh();
	void render_point();
	
	// (オーバーライド)終了時に呼ばれる関数
	void close();

	// 外部で計算したindex-vertex bufferを，自身のメンバ変数のindex-vertex bufferにセットする関数
	void setBuffers(std::vector<MyVertex> _vertecies, std::vector<MyIndex_TRIANGLES> _indecies);
	void setCameraIndex(std::vector<float> _cameraRefer);
	void setShiftZeroIndex(int zeroIndex){ shift_zeroIndex = zeroIndex; };

	// 移動量を格納したベクター
	void setShiftVector(std::vector<cv::Mat> &shiftVectors);

	// layout = 2にセットするVBOを設定
	void setShaderLocationShiftVector(bool isEnable, GLuint num = 0);

	void setDefaultVertexPointer();

	void feedbackCalclateForGetOffsetResult();
	void feedbackSetVertexPointerForGetOffsetResult(unsigned int num);

	// 画素対応の移動量を更新する関数
	void feedbackSetVertexPointerForSetNewOffset();
	void feedbackCalclateForSetNewOffset();
	// 画素対応の移動量に対し，平滑処理を施す関数
	void feedbackSetVertexPointerForBlur();
	void feedbackCalclateForBlur(GLuint vbo);

	// 対応マップをデフォルトに戻す関数
	void resetOffsetVBO();

	GLuint getVAO(){ return vao; }
	GLuint* getVBO_SHIFT(){ return vbo_shift; };
	GLuint getVBO_OFFSET(){ return vbo_offset; };
	//GLuint getVBO_CAMERAINDEX(){ return vbo_camIndex; };
	GLuint getVBO_OFFSETRESULT(){ return vbo_offset_result; };
	GLuint getVBO_STATUS(){ return vbo_status; };
	GLuint getVBO_FEEDBACK(){ return feedback; };
	GLuint getVBO_MASK(){ return VBO_mask_camsize->buf(); }
	GLuint getVBO_MASK_SIZE(){ return VBO_mask_camsize->size(); }
	int getVertexNum(){ return vertex_num; };
	int getShiftNum(){ return shift_num; };

private:
	unsigned int windowWidth;
	unsigned int windowHeight;
	// 頂点配列オブジェクト(0：幾何補正、1:幾何補正なし)
	GLuint vao;

	// 頂点バッファオブジェクト
	GLuint vbo;			// 初期計測時の幾何対応
	GLuint vbo_offset;	// オフセット
	GLuint *vbo_shift;	// 移動用バッファオブジェクト

	// オフセットを求めた結果
	GLuint vbo_offset_result;

	// コピー用頂点バッファオブジェクト
	GLuint vbo_offset_copy;

	glUtl::glBuffer<GLfloat> *VBO_mask_camsize;

	// テクスチャバッファオブジェクト参照用のバッファオブジェクト
	//GLuint vbo_camIndex;
	GLuint vbo_status;

	// インデックスバッファオブジェクト
	GLuint indexbuffer;

	// feedback
	GLuint feedback;

	int vertex_num;
	int shift_num;
	int shift_zeroIndex;

	// 描画するインデックス数
	int index_num;

	// 描画準備ができているかどうか
	bool ready;

};

