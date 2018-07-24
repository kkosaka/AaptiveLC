#include "adaptiveGeomThread.h"

/**
* @brief   初期化関数
*
* @param   coord_map[in] : 対応先を保存したマット情報を保持するvecotr配列
*
* @param   points[in]	:	カメラ座標
*
* @param   lines[in]	:	プロジェクタ座標に対するカメラ座標のエピポーラ線を格納した情報
*/
bool adaptiveGeomCorresp::init(const cv::Mat &coord_map)
{
	// 座標データを消去
	camPoints.clear();
	projPoints.clear();
	if (coord_map.empty())
		return false;

	// 対応マップを保持
	coordMap = coord_map.clone();
	indexMap = cv::Mat(camHeight, camWidth, CV_32SC1);
	int count = 0;
	for (int y = 0; y < camHeight; y++) {
		for (int x = 0; x < camWidth; x++) {
			cv::Point2f point = coordMap.ptr<cv::Vec2f>(y)[x];
			// 対応エラー点は除去
			if (point.x == -1.0f || point.y == -1.0f)
				continue;

			camPoints.emplace_back(cv::Point2f(x, y));
			projPoints.emplace_back(point);
			indexMap.ptr<int>(y)[x] = count;
			count++;
		}
	}

	// カメラ画素側の画素対応を変更するので，カメラ画素上のエピポーラ線を求める
	// カメラ上のエピポーラ線を求める場合引数は,(プロジェクタ座標, 2, F, lines)
	// ちなみに，プロジェクタ上のエピポーラ線を求めたい場合, (カメラ座標, 1, F, lines)
	cv::computeCorrespondEpilines(projPoints, 2, FundametalMat, epiLines);

	//// debug(エピポーラ線を描画)
	//cv::Mat camImage = cv::imread("./Calibration/projection.jpg");
	//cv::Mat projImage = cv::imread("./Calibration/reshape.jpg");
	//std::vector<cv::Vec3f> lines;
	//std::vector<cv::Point2f> draw_points;
	//draw_points.emplace_back(camPoints[indexMap.ptr<int>(900)[1000]]);
	//draw_points.emplace_back(camPoints[indexMap.ptr<int>(600)[960]]);
	//draw_points.emplace_back(camPoints[indexMap.ptr<int>(300)[300]]);
	//lines.emplace_back(epiLines[indexMap.ptr<int>(900)[1000]]);
	//lines.emplace_back(epiLines[indexMap.ptr<int>(600)[960]]);
	//lines.emplace_back(epiLines[indexMap.ptr<int>(300)[300]]);
	//// エピポーラ線を描画
	//drawEpilines(camImage, lines, draw_points);

	// すべてのカメラ画素における移動量を求める
	calcNewGeometricCorrespond(shiftVectorMats, camPoints, epiLines);

	return true;
}

// エピポーラ線を描画
void adaptiveGeomCorresp::drawEpilines(cv::Mat& image, std::vector<cv::Vec3f> lines, std::vector<cv::Point2f> drawPoints)
{
	for (int i = 0; i < drawPoints.size(); ++i){
		if (lines[i][1]){
			// Y軸と交わる直線の場合
			double width = image.size().width;
			// left は x=0 における直線の通過座標
			cv::Point2d left = cv::Point2d(0.0, solveY(lines[i], 0.0));
			// right は x=width における直線の通過座標
			cv::Point2d right = cv::Point2d(width, solveY(lines[i], width));
			// 左端から右端に直線を引く
			cv::line(image, left, right, cv::Scalar(0, 0, 255), 2);
		}
		else{
			// Y軸に平行な直線の場合
			// x座標は一定なので，あらかじめ求める
			double x = -(lines[i])[2] / (lines[i])[0];
			// 画像の上端を通過する座標
			cv::Point2d top = cv::Point2d(x, 0.0);
			// 画像の下端を通過する座標
			cv::Point2d bottom = cv::Point2d(x, image.size().height);
			// 上端から下端に直線を引く
			cv::line(image, top, bottom, cv::Scalar(0, 0, 255), 2);
		}
		cv::circle(image, drawPoints[i], 5, cv::Scalar(255, 0, 0));
	}
	cv::resize(image, image, cv::Size(), 0.5, 0.5);
	cv::imshow("epiline", image);
	cv::waitKey(0);
}

/**
* @brief   各画素ごとにエピポーラ線方向に対応をずらすための参照テーブル(Mat)を作成するための関数
*		   ずれ量推定のときは，幾何変換を行うときに初期計測した対応マップ(+推定したずれ量)にこのずれ量を加算したものを用いる
*
* @param   _shiftVectorMats[in][out] : 対応先を保存したマット情報を保持するvecotr配列
*
* @param   points[in]	:	カメラ座標
*
* @param   lines[in]	:	プロジェクタ座標に対するカメラ座標のエピポーラ線を格納した情報
*/
void adaptiveGeomCorresp::calcNewGeometricCorrespond(std::vector<cv::Mat> &_shiftVectorMats, const std::vector<cv::Point2f> &points, const std::vector<cv::Vec3f> &lines)
{
	// 移動量を -d[pixel]  から d[pixel] まで 1[pixel] ずつ増やしていく
	for (float d = -(maxPixelSize); d <= maxPixelSize; d += 1.0f) {
		// 移動量を格納
		cv::Mat shiftVecMat(camHeight, camWidth, CV_32FC2, cv::Scalar::all(0.0f));

		// 全画素において、エピポーラ線上の移動量を算出する
		auto it = lines.begin();
		auto it_points = points.begin();
		for (; it != lines.end(); ++it, ++it_points)
		{
			cv::Vec2f move_vector;
			// y軸に平行でない場合
			if ((*it)[1]){
				// 傾きを求める
				float a = -(*it)[0] / (*it)[1];
				// 傾きが1よりも大きい場合
				if (abs(a) >= 1.0f) {
					// y軸方向への値の増加を優先
					float x_vector = d / a;
					move_vector = cv::Vec2f(x_vector, (float)d);
				}
				// 傾きが1よりも小さい場合
				else {
					// x軸方向への値の増加を優先
					int y_vector = d * a;
					move_vector = cv::Vec2f((float)d, (float)y_vector);
				}
			}
			else{
				// Y軸に平行な直線の場合
				// x座標は一定なので，あらかじめ求める
				move_vector = cv::Vec2f((float)d, 0.0f);
			}

			shiftVecMat.ptr<cv::Vec2f>((int)(*it_points).y)[(int)(*it_points).x] = move_vector;
		}

		// Matに保存
		_shiftVectorMats.emplace_back(shiftVecMat);
	}

}
//
//void adaptiveGeomCorresp::getValiance(GLuint &k_texture_objs, GLuint &mask_objs, int current_num)
//{
//	//fps->begin();
//	std::vector<GLuint> v_tex_objs;
//	// シェーダの有効
//	shader_getv->setEnabled(true);
//	// 変数の設定
//	glUniform1i(shl_corrent_index, current_num);
//	// デバッグ用
//	GLuint txo_valiance = texobj32F->getTextureObject();
//	// 分散を計算する
//	calcReliability(k_texture_objs, mask_objs, txo_valiance);
//	texobj32F->releaseTextureObject(txo_valiance);
//	cv::Mat	sd = saveTexture(txo_valiance, "adaptiveLC/Result/result" + to_string(current_num), projWidth, projHeight);
//	images.emplace_back(sd);
//	// シェーダの無効
//	shader_getv->setEnabled(false);
//	//std::cout << "ズレの推定処理" << fps->getElapsed() << "[ms]" << std::endl;
//}
//
//// 分散を計算する
//void adaptiveGeomCorresp::calcReliability(GLuint &srcTexture, GLuint &maskTexture, GLuint &texture_valiance)
//{
//	glActiveTexture(GL_TEXTURE0);	//ユニット0
//	glBindTexture(GL_TEXTURE_2D, srcTexture);
//	glActiveTexture(GL_TEXTURE1);	//ユニット1
//	glBindTexture(GL_TEXTURE_2D, txo_min_valiance_before);
//	glActiveTexture(GL_TEXTURE2);	//ユニット2
//	glBindTexture(GL_TEXTURE_2D, txo_sigma_before);
//	glActiveTexture(GL_TEXTURE3);	//ユニット3
//	glBindTexture(GL_TEXTURE_2D, txo_index_before);
//	glActiveTexture(GL_TEXTURE4);	//ユニット4
//	glBindTexture(GL_TEXTURE_2D, maskTexture);
//
//	// オフスクリーンレンダリング開始
//	agc_offscreen_render->setEnabled(true);
//	agc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txo_min_valiance_after);
//	agc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, txo_sigma_after);
//	agc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, txo_index_after);
//	agc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, texture_valiance);
//	agc_offscreen_render->drawBuffers();
//	vertex->render_mesh();
//	// オフスクリーンレンダリング終了
//	agc_offscreen_render->setEnabled(false);
//
//	// テクスチャの入れ替え
//	GLuint tmp;
//	tmp = txo_min_valiance_before;
//	txo_min_valiance_before = txo_min_valiance_after;
//	txo_min_valiance_after = tmp;
//
//	tmp = txo_sigma_before;
//	txo_sigma_before = txo_sigma_after;
//	txo_sigma_after = tmp;
//
//	tmp = txo_index_before;
//	txo_index_before = txo_index_after;
//	txo_index_after = tmp;
//}
//
//bool adaptiveGeomCorresp::Copy(GLuint pbo, void* dest, GLuint fbo, GLuint src_tex, GLint x, GLint y, GLsizei width, GLsizei height)
//{
//	GL_PIXEL_PACK_BUFFER_ARB;
//	bool ret = false;
//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src_tex, 0);
//	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
//	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
//	glReadPixels(x, y, width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
//	void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
//	if (ptr != NULL)
//	{
//		memcpy(dest, ptr, width * height * 4);
//		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
//		ret = true;
//	}
//	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	return ret;
//
//	// PBOの生成
//	//int channels = 1;
//	//PBO pbo;
//	//pbo.getPBO(camWidth, camHeight, channels, GL_FLOAT);
//	//fps->begin();
//	//gt_offscreen_render_cam->setEnabled(true);
//	//gt_offscreen_render_cam->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txo_tex_cam);
//	//glReadBuffer(GL_COLOR_ATTACHMENT0);
//	//pbo.bind(GL_PIXEL_PACK_BUFFER, true);
//	//glReadPixels(0, 0, camWidth, camHeight, GL_RED, GL_FLOAT, 0);
//	//// Mat へコピー
//	//cv::Mat dst(camHeight, camWidth, CV_32F);
//	//void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
//	//if (ptr != NULL)
//	//{
//	//	memcpy(dst.data, ptr, camWidth * camHeight * channels * sizeof(GL_FLOAT));
//	//	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
//	//}
//	//glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
//	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}



//void adaptiveGeomCorresp::correctGeometricCorresp(const cv::Mat &moveVectorMat, cv::Mat maskMat)
//{
//	cv::Mat dst = cv::Mat(camHeight, camWidth, CV_32FC2);
//	moveVectorsResult.clear();
//
//	// 値を補完する
//	for (int y = 0; y < camHeight; ++y) {
//		int *index_p = indexMap.ptr<int>(y);
//		cv::Vec2f *coord_p = coordMap.ptr<cv::Vec2f>(y);
//		for (int x = 0; x < camWidth; ++x) {
//
//			// 対応エラー点は除去
//			if (coord_p[x][0] == -1.0f || coord_p[x][1] == -1.0f)
//				continue;
//
//			// 事前に用意されたヴェクターから参照
//			unsigned int num = moveVectorMat.at<cv::Vec4f>(y, x)[0];
//			// 移動量[pixel]を格納
//			cv::Vec3f movePixel;
//			movePixel[0] = moveVectors[num][index_p[x]][0];
//			movePixel[1] = moveVectors[num][index_p[x]][1];
//			movePixel[2] = index_p[x];
//			moveVectorsResult.emplace_back(movePixel);
//
//			// debug用
//			dst.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(movePixel[0], movePixel[1]);
//		}
//	}
//	std::cout << "aa" << std::endl;
//}


/**
* @brief   レンダリング結果の保存(RGB)
*
* @param   texId[in]		保存するテクスチャのID
*
* @param   filename[in]		保存する画像のファイル名
*
* @param   w[in]		保存する画像の横幅[pixel]
*
* @param   h[in]		保存する画像の縦幅[pixel]
*/
//cv::Mat adaptiveGeomCorresp::saveTexture(GLuint texId, const string &filename, int w, int h, bool isFlip)
//{
//	cv::Mat img(h, w, CV_32FC4);
//
//	glBindTexture(GL_TEXTURE_2D, texId);
//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, img.data);
//	glBindTexture(GL_TEXTURE_2D, 0);
//
//	//ofstream ofs("./debugout/" + filename + ".txt");
//	//ofs << img;
//	cv::cvtColor(img, img, CV_RGBA2BGR);
//	if (isFlip)
//		cv::flip(img, img, 0);// 画像の反転(x軸周り)
//
//	cv::Mat uc_mat;
//	img.convertTo(uc_mat, CV_8UC3, 255.0);
//	cv::imwrite("./debugout/" + filename + ".bmp", uc_mat);
//
//	return img;
//}
