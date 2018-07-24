#include "graycode.h"

// ディレクトリの作成
void GrayCode::gcCreateDirs()
{
	// グレイコード用
	_mkdir(path);
	_mkdir(gc_base_dir.c_str());
	_mkdir(std::string(gc_base_dir + "/CaptureImages").c_str());			// グレイコード撮影画像
	_mkdir(std::string(gc_base_dir + "/ProjectionImages").c_str());		// グレイコード生画像
	_mkdir(std::string(gc_base_dir + "/ThresholdImages").c_str());			// グレイコード撮影画像の二値化した画像

}

void GrayCode::init()
{
	gcCreateDirs();
	makeGraycodeImage();
}

// (GrayCode)パターンコード画像作成
void GrayCode::makeGraycodeImage()
{
	initGraycode();
	cv::Mat posi_img(projHeight, projWidth, CV_8UC3, cv::Scalar(0, 0, 0));
	cv::Mat nega_img(projHeight, projWidth, CV_8UC3, cv::Scalar(0, 0, 0));
	int bit = all_bit - 1;

	// ポジパターンコード画像作成
	for (unsigned int z = 0; z < all_bit; z++) {
		for (unsigned int y = 0; y < projHeight; y++) {
			for (unsigned int x = 0; x < projWidth; x++) {
				if (((graycode_mat.ptr<int>(y)[x] >> (bit - z)) & 1) == 0) {  // 最上位ビットから順に抽出し，そのビットが0だった時
					posi_img.ptr<cv::Vec3b>(y)[x] = cv::Vec3b(0, 0, 0);
					nega_img.ptr<cv::Vec3b>(y)[x] = cv::Vec3b(255, 255, 255);
				}
				else if (((graycode_mat.ptr<int>(y)[x] >> (bit - z)) & 1) == 1) {
					posi_img.ptr<cv::Vec3b>(y)[x] = cv::Vec3b(255, 255, 255);
					nega_img.ptr<cv::Vec3b>(y)[x] = cv::Vec3b(0, 0, 0);
				}
			}
		}
		// 連番でファイル名を保存（文字列ストリーム）
		cv::imwrite(gcGetProjctionFilePath(z, POSI).str(), posi_img);
		cv::imwrite(gcGetProjctionFilePath(z, NEGA).str(), nega_img);
	}

}

// 波長に合わせたビット数の計算とグレイコードの作成
void GrayCode::initGraycode()
{
	std::vector<int> bin_code_h;
	std::vector<int> bin_code_w;
	std::vector<int> gray_code_h;
	std::vector<int> gray_code_w;

	/***** 2進コード作成 *****/
	// 行について
	for (unsigned int y = 0; y < waveNumY; y++)
		bin_code_h.emplace_back(y + 1);
	// 列について
	for (unsigned int x = 0; x < waveNumX; x++)
		bin_code_w.emplace_back(x + 1);

	/***** グレイコード作成 *****/
	// 行について
	for (unsigned int y = 0; y < waveNumY; y++)
		gray_code_h.emplace_back(bin_code_h[y] ^ (bin_code_h[y] >> 1));
	// 列について
	for (unsigned int x = 0; x < waveNumX; x++)
		gray_code_w.emplace_back(bin_code_w[x] ^ (bin_code_w[x] >> 1));
	// 行列を合わせる（行 + 列）
	for (unsigned int y = 0; y < waveNumY; y++) {
		for (unsigned int x = 0; x < waveNumX; x++) {
			graycode.ptr<int>(y)[x] = (gray_code_h[y] << w_bit) | gray_code_w[x];
			//投影サイズのグレイコード配列作成
			for (unsigned int i = 0; i < waveLength_y; i++)
			for (unsigned int j = 0; j < waveLength_x; j++)
				graycode_mat.ptr<int>(y*waveLength_y + i)[x*waveLength_x + j] = graycode.ptr<int>(y)[x];
		}
	}
}


// opencvを利用して映像投影(映像が1pixel左と下にずれるので，輝度補正の際はGLを使うこと)
void GrayCode::code_projection_opencv()
{
	// 定数
	typedef enum flag{
		POSI = true,
		NEGA = false,
		VERTICAL = true,
		HORIZONTAL = false,
	} flag;

	TPGROpenCV	*pgrOpenCV = new TPGROpenCV;
	//初期設定
	pgrOpenCV->init(FlyCapture2::PIXEL_FORMAT_BGR, FlyCapture2::HQ_LINEAR);
	pgrOpenCV->setShutterSpeed(pgrOpenCV->getShutter_measure());
	pgrOpenCV->start();
	int delay = (int)(2.5 * pgrOpenCV->getDelay());

	char *CODE_IMG = "GC";
	// 全画面表示用ウィンドウの作成  
	cv::namedWindow(CODE_IMG, 0);

	HWND windowHandle = ::FindWindowA(NULL, CODE_IMG);
	SetWindowLong(windowHandle, GWL_STYLE, WS_POPUP);
	SetWindowPos(windowHandle, HWND_TOP,
		1680, 0,
		projWidth, projHeight, SWP_SHOWWINDOW);
	cv::imshow(CODE_IMG, cv::Mat(projHeight, projWidth, CV_8UC3, cv::Scalar::all(255)));
	cv::waitKey(delay);

	/******* GrayCode読込 *********/

	cv::Mat *posi_img = new cv::Mat[gcGetBitNum_all()];  // ポジパターン用
	cv::Mat *nega_img = new cv::Mat[gcGetBitNum_all()];  // ネガパターン用

	// 連番でファイル名を読み込む（文字列ストリーム）
	std::cout << "投影用グレイコード画像読み込み中" << std::endl;
	for (unsigned int i = 0; i < gcGetBitNum_all(); i++) {
		// 読み込み
		posi_img[i] = cv::imread(gcGetProjctionFilePath(i, POSI).str(), 1);
		nega_img[i] = cv::imread(gcGetProjctionFilePath(i, NEGA).str(), 1);
		// 読み込む枚数が足りなかったらグレイコード画像を作り直す
		if (posi_img[i].empty() || nega_img[i].empty()){
			std::cout << "ERROR(1)：投影用のグレイコード画像が不足しています。" << std::endl;
			std::cout << "ERROR(2)：グレイコード画像を作成します。" << std::endl;
			makeGraycodeImage();
			code_projection_opencv();
			return;
		}
	}

	/***** グレイコード投影 & 撮影 *****/
	cv::Mat cap;
	// ポジパターン投影 & 撮影
	std::cout << "ポジパターン撮影中" << std::endl;
	for (unsigned int i = 0; i < gcGetBitNum_all(); i++) {
		// 投影
		cv::imshow(CODE_IMG, posi_img[i]);
		// カメラ撮影
		cv::waitKey(delay);
		pgrOpenCV->CameraCapture(cap);

		// ポジパターン撮影結果を保存
		// 横縞
		if (i < gcGetBitNum_h())
			cv::imwrite(gcGetCaptureFilePath(i, HORIZONTAL, POSI).str(), cap);
		// 縦縞
		else
			cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_h(), VERTICAL, POSI).str(), cap);
	}

	cv::waitKey(delay);
	// ネガパターン投影 & 撮影
	std::cout << "ネガパターン撮影中" << std::endl;
	for (unsigned int i = 0; i < gcGetBitNum_all(); i++) {
		// 投影
		cv::imshow(CODE_IMG, nega_img[i]);

		// カメラ撮影
		cv::waitKey(delay);
		pgrOpenCV->CameraCapture(cap);

		// ポジパターン撮影結果を保持
		// 横縞
		if (i < gcGetBitNum_h())
			cv::imwrite(gcGetCaptureFilePath(i, HORIZONTAL, NEGA).str(), cap);
		// 縦縞
		else
			cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_h(), VERTICAL, NEGA).str(), cap);
	}

	/***** 投影 & 撮影終了 *****/

	pgrOpenCV->stop();
	//pgrOpenCV->release();

	/***** 終了 *****/

	cv::destroyWindow(CODE_IMG);
	delete[] posi_img;
	delete[] nega_img;
	delete pgrOpenCV;
	pgrOpenCV = NULL;
	posi_img = NULL;
	nega_img = NULL;
}


// テクスチャを読み込む関数
bool GrayCode::readTexture()
{
	// graycode画像の読み込み
	for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, POSI).str()));
	}
	// graycode画像の読み込み
	for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, NEGA).str()));
	}

	// テクスチャ化
	for (int i = 0; i < code_texture.size(); ++i) {
		code_texture[i].load();
	}

	return true;
}

bool GrayCode::code_projection_opengl(GLFWwindow *window)
{
	// シェーダの設定
	if (!glsl->init("./shader/texture.vert", "./shader/texture.frag"))
		return false;

	glsl->setEnabled(true);
	// uniform変数に値を渡す
	glUniform1i(glGetUniformLocation(glsl->getHandle(), "texture"), 0);	//テクスチャユニット0

	// ウインドウサイズのポリゴンを設定
	Vertex *vertex = new Vertex(projWidth, projHeight);
	vertex->init();

	// 画像の読み込み
	if (!readTexture())
		return false;

	//// PGRカメラの設定
	//TPGROpenCV	*pgrOpenCV = new TPGROpenCV;
	//// カメラをスタートさせる
	//if (pgrOpenCV->init(FlyCapture2::PIXEL_FORMAT_BGR, FlyCapture2::HQ_LINEAR) == -1)
	//	return false;
	//// カメラが正常に動作していた場合
	//pgrOpenCV->setShutterSpeed(pgrOpenCV->getShutter_h());
	//pgrOpenCV->start();	// カメラスタート

	cv::Mat cap;
	unsigned int count = 0;
	while (count < code_texture.size())
	{
		// 使用する画像をバインド
		code_texture[count].bind(GL_TEXTURE0);
		// 描画
		vertex->render_mesh();
		// カラーバッファを入れ替え,イベントを取得
		glfwSwapBuffers(window);

		//// 待機
		//Sleep(pgrOpenCV->getDelay() * 2);
		//// 画像を保存
		//pgrOpenCV->CameraCapture(cap);
		// グレイコードの保存
		// 横縞
		//if (count < gcGetBitNum_h() * 2){
		//	if (count % 2 == 0)
		//		cv::imwrite(gcGetCaptureFilePath(count / 2, HORIZONTAL, POSI).str(), cap);
		//	else
		//		cv::imwrite(gcGetCaptureFilePath((count - 1) / 2, HORIZONTAL, NEGA).str(), cap);
		//}
		//// 縦縞
		//else {
		//	int c = count - (gcGetBitNum_h() * 2);
		//	if (count % 2 == 0)
		//		cv::imwrite(gcGetCaptureFilePath(c / 2, VERTICAL, POSI).str(), cap);
		//	else
		//		cv::imwrite(gcGetCaptureFilePath((c - 1) / 2, VERTICAL, NEGA).str(), cap);
		//}
		count++;
		//if (count == code_texture.size()){
		//	finish = true;
		//	code_texture.clear();
		//}
	}

	//pgrOpenCV->stop();
	glsl->setEnabled(false);

	delete vertex;
	vertex = NULL;

	//delete pgrOpenCV;
	//pgrOpenCV = NULL;

	return true;
}

// 撮影画像の2値化をするインタフェース
void GrayCode::make_thresh()
{
	cv::Mat posi_img;
	cv::Mat nega_img;
	cv::Mat Geometric_thresh_img;  // 2値化された画像
	cv::Mat mask;

	// マスクを生成
	makeMask(mask);

	std::cout << "二値化開始" << std::endl;
	// 連番でファイル名を読み込む
	// 横縞
	for (unsigned int i = 0; i < h_bit; i++) {
		// ポジパターン読み込み
		posi_img = cv::imread(gcGetCaptureFilePath(i, HORIZONTAL, POSI).str(), 0);
		nega_img = cv::imread(gcGetCaptureFilePath(i, HORIZONTAL, NEGA).str(), 0);

		// 2値化
		cv::Mat masked_img;
		thresh(posi_img, nega_img, Geometric_thresh_img, 0);
		// マスクを適用して2値化
		Geometric_thresh_img.copyTo(masked_img, mask);
		cv::imwrite(gcGetThresholdFilePath(i).str(), masked_img);

		std::cout << i << ", ";
	}
	// 縦縞
	for (unsigned int i = 0; i < w_bit; i++) {
		posi_img = cv::imread(gcGetCaptureFilePath(i, VERTICAL, POSI).str(), 0);
		nega_img = cv::imread(gcGetCaptureFilePath(i, VERTICAL, NEGA).str(), 0);

		// 2値化
		cv::Mat masked_img;
		thresh(posi_img, nega_img, Geometric_thresh_img, 0);
		// マスクを適用して2値化
		Geometric_thresh_img.copyTo(masked_img, mask);
		cv::imwrite(gcGetThresholdFilePath(i + h_bit).str(), masked_img);

		std::cout << i + h_bit << ", ";
	}
	std::cout << std::endl;

	getMaskForGeometricCorrection();

	std::cout << "二値化終了" << std::endl;
}

void GrayCode::getMaskForGeometricCorrection()
{
	//// マスク画像のアドレスをセット
	square->setMask(gc_mask_address);
	// マスク画像に合わせて画像を作成
	square->adjustSquare(cv::Size(projWidth, projHeight), IMAGE_DIRECTORY, SAVE_DIRECTORY);
}

std::stringstream GrayCode::gcGetProjctionFilePath(unsigned int num, bool posi_nega)
{
	std::stringstream filename;
	if (num > all_bit) {
		std::cerr << num << " 枚目の画像はありません" << std::endl;
		return filename;
	}
	if (posi_nega == POSI)
		filename << gc_base_dir << "/ProjectionImages/posi" << std::setw(2) << std::setfill('0') << num << ".bmp";
	if (posi_nega == NEGA)
		filename << gc_base_dir << "/ProjectionImages/nega" << std::setw(2) << std::setfill('0') << num << ".bmp";

	return filename;
}
std::stringstream GrayCode::gcGetCaptureFilePath(unsigned int num, bool horizontal, bool pattern)
{
	std::stringstream filename;
	filename << gc_base_dir << "/CaptureImages/CameraImg" << horizontal << "_" << std::setw(2) << std::setfill('0') << num << "_" << pattern << ".bmp";
	return filename;
}
std::stringstream GrayCode::gcGetThresholdFilePath(unsigned int num)
{
	std::stringstream filename;
	filename << gc_base_dir << "/ThresholdImages/Geometric_thresh" << std::setw(2) << std::setfill('0') << num << ".bmp";
	return filename;
}

// マスクを作成するインタフェース
void GrayCode::makeMask(cv::Mat &mask)
{
	cv::Mat posi_img;
	cv::Mat nega_img;

	// マスク画像生成
	cv::Mat mask_vert, mask_hor;
	static int useImageNumber = 5;
	// y方向のグレイコード画像読み込み
	posi_img = cv::imread(gcGetCaptureFilePath(useImageNumber, HORIZONTAL, POSI).str(), 0);
	nega_img = cv::imread(gcGetCaptureFilePath(useImageNumber, HORIZONTAL, NEGA).str(), 0);

	// 仮のマスク画像Y生成
	makeMaskFromCam(posi_img, nega_img, mask_vert, threshold);

	// x方向のグレイコード画像読み込み
	posi_img = cv::imread(gcGetCaptureFilePath(useImageNumber, VERTICAL, POSI).str(), 0);
	nega_img = cv::imread(gcGetCaptureFilePath(useImageNumber, VERTICAL, NEGA).str(), 0);

	// 仮のマスク画像X生成
	makeMaskFromCam(posi_img, nega_img, mask_hor, threshold);

	// XとYのORを取る
	// マスク外はどちらも黒なので黒
	// マスク内は（理論的には）必ず一方が白でもう一方が黒なので、白になる
	// 実際はごま塩ノイズが残ってしまう
	cv::bitwise_or(mask_vert, mask_hor, mask);

	// 残ったごま塩ノイズを除去（白ゴマか黒ゴマかで適用順が逆になる）
	dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 5);
	erode(mask, mask, cv::Mat(), cv::Point(-1, -1), 5);

	//for (int i = 0; i < 2; i++)
	//	smallMaskRange(mask, mask);


	cv::imwrite(gc_mask_address, mask);


}

// グレイコードの画像を利用してマスクを生成する関数
// ポジとネガの差分を取ってthresholdValue以上の輝度のピクセルを白にする
void GrayCode::makeMaskFromCam(cv::Mat &posi, cv::Mat &nega, cv::Mat &result, int thresholdValue)
{
	result = cv::Mat(posi.size(), CV_8UC1);

	for (int j = 0; j < result.rows; j++){
		for (int i = 0; i<result.cols; i++){
			int posi_i = posi.at<uchar>(j, i);
			int nega_i = nega.at<uchar>(j, i);

			if (abs(posi_i - nega_i) > thresholdValue){
				result.at<uchar>(j, i) = 255;
			}
			else{
				result.at<uchar>(j, i) = 0;
			}
		}
	}
}

// 実際の2値化処理 
void GrayCode::thresh(cv::Mat &posi, cv::Mat &nega, cv::Mat &thresh_img, int thresh_value)
{
	thresh_img = cv::Mat(posi.rows, posi.cols, CV_8UC1);
	for (int y = 0; y < posi.rows; y++) {
		for (int x = 0; x < posi.cols; x++) {
			int posi_pixel = posi.at<uchar>(y, x);
			int nega_pixel = nega.at<uchar>(y, x);

			// thresh_valueより大きいかどうかで二値化
			if (posi_pixel - nega_pixel >= thresh_value)
				thresh_img.at<uchar>(y, x) = 255;
			else
				thresh_img.at<uchar>(y, x) = 0;
		}
	}
}


// 2値化コード復元
void GrayCode::getCorrespondence(bool isCodeProjection)
{
	// 撮影画像をデコード
	decodeGrayCode();

	g_code_map->clear();

	int camH = decode.rows;
	int camW = decode.cols;
	// 連想配列でグレイコードの値の場所に座標を格納
	for (int y = 0; y < camH; y++) {
		for (int x = 0; x < camW; x++) {
			int a = decode.ptr<int>(y)[x];
			(*g_code_map)[a] = cv::Point(x, y);
		}
	}

	// 0番目は使わない
	(*g_code_map)[0] = cv::Point(-1, -1);

	int graycodeH = graycode.rows;
	int graycodeW = graycode.cols;
	// デコードされた値に対応するプロジェクタ側の座標値をCamProに格納する
	for (int y = 0; y < graycodeH; y++) {
		for (int x = 0; x < graycodeW; x++) {
			int a = graycode.ptr<int>(y)[x];
			// map内に存在しないコード（カメラで撮影が上手くいかなかった部分）の場所にはエラー値-1を格納
			if ((*g_code_map).find(a) == (*g_code_map).end()) {
				coordmap.ptr<cv::Vec2i>(y)[x] = cv::Vec2i(-1, -1);
			}
			// 存在する場合は、計測されたグレイコード値に対応するプロジェクタ座標を格納
			else {
				coordmap.ptr<cv::Vec2i>(y)[x] = cv::Vec2i((*g_code_map)[a]);
			}
		}
	}

	// 頂点配列
	getBufferArrays();

	ready = true;
	std::cout << "finish gray code decoding" << std::endl;

}
void GrayCode::decodeGrayCode()
{

	std::cout << "start gray code decoding" << std::endl;

	int camH = decode.rows;
	int camW = decode.cols;
	// 2値化コード復元
	for (unsigned int i = 0; i < all_bit; i++) {
		cv::Mat a = cv::imread(gcGetThresholdFilePath(i).str(), 0);

		for (int y = 0; y < camH; y++) {
			for (int x = 0; x < camW; x++) {
				if (a.at<uchar>(y, x) == 255)
					decode.ptr<int>(y)[x] = (1 << (all_bit - i - 1)) | decode.ptr<int>(y)[x];
			}
		}
	}

}

// 画像変形・処理
// カメラ撮影領域からプロジェクタ投影領域を切り出し
void GrayCode::reshapeCam2Proj(cv::Mat &src, cv::Mat &dst)
{
	if (!ready) {
		std::cout << "対応マップが取得されていません" << std::endl;
		return;
	}
	int height = coordmap.rows;
	int width = coordmap.cols;
	cv::Mat tmp(coordmap.rows, coordmap.cols, src.type());
	if (src.channels() == 1){
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				cv::Point p = coordmap.ptr<cv::Vec2i>(y)[x];
				if (p.x != -1) {
					tmp.ptr<uchar>(y)[x] = src.ptr<uchar>(p.y)[p.x];
				}
			}
		}
	}
	if (src.channels() == 3) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				cv::Point p = coordmap.ptr<cv::Vec2i>(y)[x];
				if (p.x != -1) {
					tmp.ptr<cv::Vec3b>(y)[x] = src.ptr<cv::Vec3b>(p.y)[p.x];
				}
			}
		}
	}
	tmp.copyTo(dst);
}

// 頂点配列を取得する
void GrayCode::getBufferArrays()
{
	//cv::Mat vertCoordMap, texCoordMap;

	//// 求めた対応マップを，GLの座標系に変換
	//getGLCoordMap(vertCoordMap, texCoordMap);

	////// 頂点情報
	//cv::Mat indexMap = cv::Mat(projHeight, projWidth, CV_32S);
	//std::vector<Vertex::MyVertex> vertex;
	//std::vector<Vertex::MyIndex_TRIANGLES> index;
	//unsigned int count = 0;

	//// 対応がとれたすべての画素の vertex buffer を取得
	//for (unsigned int y = 0; y < projHeight; ++y) {
	//	int *index = indexMap.ptr<int>(y);
	//	for (unsigned int x = 0; x < projWidth; ++x) {

	//		cv::Vec3f pos = vertCoordMap.ptr<cv::Vec3f>(y)[x];
	//		cv::Vec2f tex = texCoordMap.ptr<cv::Vec2f>(y)[x];
	//		// 対応点が計測できている画素の頂点座標とテクスチャ座標をvectorに保持する
	//		if (pos[0] != -1.1f) {
	//			// 頂点情報をセットでvectorに保持
	//			vertex.emplace_back(Vertex::MyVertex(pos, tex));
	//			// 格納した画素に番号を割り振る(インデックスバッファで参照するため)
	//			index[x] = count;
	//			count++;
	//		}
	//	}
	//}

	//// 対応がとれたすべての画素にメッシュを張るための，index bufferを取得
	//for (unsigned int y = 0; y < projHeight - 1; ++y) {
	//	for (unsigned int x = 0; x < projWidth - 1; ++x) {

	//		// 着目画素，下, 右, 右下の画素を参照し，正常にとれていたら，三角メッシュの頂点にする
	//		cv::Vec3f pos = vertCoordMap.ptr<cv::Vec3f>(y)[x];
	//		cv::Vec3f right = vertCoordMap.ptr<cv::Vec3f>(y)[x + 1];
	//		cv::Vec3f down = vertCoordMap.ptr<cv::Vec3f>(y + 1)[x];
	//		cv::Vec3f diag = vertCoordMap.ptr<cv::Vec3f>(y + 1)[x + 1];
	//		if (pos[0] != -1.1f && right[0] != -1.1f && down[0] != -1.1f && diag[0] != -1.1f) {
	//			// メッシュを張る頂点画素の番号を，vectorに保持(２つ)
	//			int pos_index = indexMap.ptr<int>(y)[x];
	//			int right_index = indexMap.ptr<int>(y)[x + 1];
	//			int down_index = indexMap.ptr<int>(y + 1)[x];
	//			int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
	//			index.emplace_back(Vertex::MyIndex_TRIANGLES(pos_index, down_index, right_index));
	//			index.emplace_back(Vertex::MyIndex_TRIANGLES(diag_index, right_index, down_index));
	//		}
	//	}
	//}

	// 求めた頂点情報を用いて，頂点配列管理クラスVertexを更新
	//coord_trans_mesh[0]->setBuffers(vertex, index);

}

void GrayCode::getGLCoordMap(cv::Mat &dst_vertex, cv::Mat &dst_texture)
{
	dst_vertex = cv::Mat(projHeight, projWidth, CV_32FC3);
	dst_texture = cv::Mat(projHeight, projWidth, CV_32FC2);

	for (unsigned int y = 0; y < projHeight; ++y) {
		for (unsigned int x = 0; x < projWidth; ++x) {

			cv::Point2f point = coordmap.ptr<cv::Vec2f>(y)[x];
			// 座標データが正常値で、かつマスク領域内の場合，-1 ～ 1に収まるよう正規化
			if (point.x != -1.0){
				// 頂点座標を取得
				// テクスチャ座標を取得
				float vx = (float)x / (projWidth - 1.0f) * 2.0f - 1.0f;
				float vy = -(((float)y / (projHeight - 1.0f)) * 2.0f - 1.0f);
				float vz = 0.0;
				dst_vertex.ptr<cv::Vec3f>(y)[x] = cv::Vec3f(vx, vy, vz);
				// テクスチャ座標を取得
				float u = (float)point.x / (camWidth - 1.0f);
				float v = -((float)point.y / (camHeight - 1.0f)) + 1.0f;
				dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(u, v);
			}
			// 座標データがエラーの場合，範囲外の"-1.1"を格納
			else{
				dst_vertex.ptr<cv::Vec3f>(y)[x] = cv::Vec3f(-1.1f, -1.1f, -1.1f);
				dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
			}
		}
	}

}