#include "phaseshift.h"
#include "TextureObjectStocker.h"
#include <memory>

using namespace glUtl;

// ディレクトリの作成
void PhaseShift::psCreateDirs()
{
	_mkdir(ps_path);
	gcCreateDirs();	// フォルダが文字化けする原因だと思う
	_mkdir(ps_base_dir.c_str());
	_mkdir(ps_image_dir.c_str());
	_mkdir(std::string(ps_image_dir + "/ProjectionImages").c_str());	// 正弦波投影画像
	_mkdir(std::string(ps_image_dir + "/CaptureImages").c_str());	// 正弦波撮影画像
}

void PhaseShift::init()
{
	// ディレクトリの作成
	psCreateDirs();
	makeGraycodeImage();
	makeSineWaveImage();

	init_flag = true;
}

// 正弦波画像作成
void PhaseShift::makeSineWaveImage()
{
	cv::Mat x_img(projHeight, projWidth, CV_8UC1);
	cv::Mat y_img(projHeight, projWidth, CV_8UC1);
	for (int i = 0; i < Proj_num; i++) {
		for (unsigned int x = 0; x < projWidth; x++){
			for (unsigned int y = 0; y < projHeight; y++){
				x_img.at<uchar>(y, x) = (uchar)(Amp * sin(2.0*PI*((double)i / Proj_num + (double)(x + xplus) / WaveLength_X)) + Bias);
				y_img.at<uchar>(y, x) = (uchar)(Amp * sin(2.0*PI*((double)i / Proj_num + (double)(y + yplus) / WaveLength_Y)) + Bias);
			}
		}
		// 連番でファイル名を保存（文字列ストリーム）
		cv::imwrite(psGetProjctionFilePath(i, VERTICAL).str(), x_img);
		// 連番でファイル名を保存（文字列ストリーム）
		cv::imwrite(psGetProjctionFilePath(i, HORIZONTAL).str(), y_img);
	}
	x_img.release();
	y_img.release();
}


// opencvを利用して映像投影
void PhaseShift::code_projection_opencv()
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

	/******* sin波読込 *********/

	cv::Mat *phase_x_img = new cv::Mat[Proj_num];
	cv::Mat *phase_y_img = new cv::Mat[Proj_num];
	// sine 画像の読み込み
	for (unsigned int i = 0; i < psGetProjectionNum(); ++i) {
		phase_x_img[i] = cv::imread(psGetProjctionFilePath(i, VERTICAL).str(), 1);
		phase_y_img[i] = cv::imread(psGetProjctionFilePath(i, HORIZONTAL).str(), 1);
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

	/***** 正弦波 投影 & 撮影 *****/

	std::cout << "正弦波パターン撮影中" << std::endl;
	//　縦縞
	for (int i = 0; i < Proj_num; i++) {
		// 投影
		cv::imshow(CODE_IMG, phase_x_img[i]);
		// カメラ撮影
		cv::waitKey(delay);
		pgrOpenCV->CameraCapture(cap);
		// パターン撮影結果を保存
		cv::imwrite(psGetCaptureFilePath(i, VERTICAL).str(), cap);
	}

	//　横縞
	for (int i = 0; i < Proj_num; i++) {
		// 投影
		cv::imshow(CODE_IMG, phase_y_img[i]);
		// カメラ撮影
		cv::waitKey(delay);
		pgrOpenCV->CameraCapture(cap);
		// パターン撮影結果を保存
		cv::imwrite(psGetCaptureFilePath(i, HORIZONTAL).str(), cap);
	}
	std::cout << "正弦波パターン撮影終了" << std::endl;

	/***** 投影 & 撮影終了 *****/

	pgrOpenCV->stop();
	//pgrOpenCV->release();

	/***** 終了 *****/

	cv::destroyWindow(CODE_IMG);
	delete[] posi_img;
	delete[] nega_img;
	delete[] phase_x_img;
	delete[] phase_y_img;
	delete pgrOpenCV;
	pgrOpenCV = NULL;
	posi_img = NULL;
	nega_img = NULL;
	phase_x_img = NULL;
	phase_y_img = NULL;
}

// テクスチャを読み込む関数
bool PhaseShift::readTexture()
{

	//// graycode画像の読み込み
	//for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
	//	code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, POSI).str()));
	//}
	//// graycode画像の読み込み
	//for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
	//	code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, NEGA).str()));
	//}
	//// sine 画像の読み込み
	//for (unsigned int i = 0; i < psGetProjectionNum(); ++i)
	//	code_texture.emplace_back(Texture(GL_TEXTURE_2D, psGetProjctionFilePath(i, VERTICAL).str()));
	//// sine 画像の読み込み
	//for (unsigned int i = 0; i < psGetProjectionNum(); ++i)
	//	code_texture.emplace_back(Texture(GL_TEXTURE_2D, psGetProjctionFilePath(i, HORIZONTAL).str()));

	//// テクスチャ化
	//for (unsigned int i = 0; i < code_texture.size(); ++i) {
	//	code_texture[i].load();
	//}

	// GPUへの転送遅延待つ
	Sleep(3 * 44);

	return true;
}
bool PhaseShift::code_projection_opengl(GLFWwindow *window)
{
	if (!init_flag){
		std::cout << "init が呼ばれていません" << std::endl;
		return false;
	}

	// シェーダの設定
	if (!GrayCode::glsl->init("./shader/texture.vert", "./shader/texture.frag")){
		std::cout << "shaderファイルが読み込めません" << std::endl;
		return false;
	}
	// シェーダの有効
	glsl->setEnabled(true);

	// uniform変数に値を渡す
	glUniform1i(getShlWithErrorDetect(glsl->getHandle(), "projection_texture"), 0);	//テクスチャユニット0

	// ウインドウサイズのポリゴンを設定
	std::unique_ptr<Vertex> vert(new Vertex(projWidth, projHeight));
	//Vertex *vert = new Vertex(projWidth, projHeight);
	vert->init();

	// テクスチャを保持する変数
	std::vector<Texture> code_texture;
	// graycode画像の読み込み
	for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, POSI).str()));
	}
	// graycode画像の読み込み
	for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, NEGA).str()));
	}
	// sine 画像の読み込み
	for (unsigned int i = 0; i < psGetProjectionNum(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, psGetProjctionFilePath(i, VERTICAL).str()));
	}
	// sine 画像の読み込み
	for (unsigned int i = 0; i < psGetProjectionNum(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, psGetProjctionFilePath(i, HORIZONTAL).str()));
	}

	// テクスチャ化
	for (unsigned int i = 0; i < code_texture.size(); ++i) {
		code_texture[i].load();
	}

	//// 画像の読み込み
	//if (!readTexture())
	//	return false;

	unsigned int proj_num = 2;
	unsigned int proj_count = 0;
	unsigned int count = 0;
	pgrCamera->resize(100);
	// 投影
	glfwSwapInterval(1);
	while (count < code_texture.size())
	{
		for (int i = 0; i < proj_num; ++i){
			// 使用する画像をバインド
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, code_texture[count].m_textureObj);
			// 描画
			vert->render_mesh();
			// カラーバッファを入れ替え,イベントを取得

			glfwSwapBuffers(window);
			proj_count++;
		}
		count++;
	}
	glfwSwapInterval(0);
	// 遅延分待機
	Sleep(90);

	// バッファ上の画像をすべて読み込み
	std::vector<cv::Mat> buff;
	pgrCamera->getAllImages(buff);

	// ホワイトの背景
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);

	//// (debug)バッファの画像をすべて保存
	//for (int i = 0; i < buff.size(); ++i) {
	//	if (!buff[i].empty()){
	//		char buf[256];
	//		sprintf_s(buf, "./Calibration/pgr/cap%d.jpg", i);
	//		cv::imwrite(buf, buff[i]);
	//	}
	//}

	// 末端から見ていって色が大きく変化したらそこが投影映像を撮り始めている場所
	cv::Vec3b check, before_color;
	unsigned int firstCaptureIndex = 0;
	for (int i = 0; i < buff.size(); ++i) {
		int lastIndex = buff.size() - 1;
		cv::Point center = cv::Point(camWidth / 2, camHeight / 2);
		cv::Mat image = buff[lastIndex - i].clone();
		if (i == 0) {
			before_color = image.at<cv::Vec3b>(center.y, center.x);
			continue;
		}
		check = image.at<cv::Vec3b>(center.y, center.x);
		cv::Point3i diff = cv::Point3i(abs(before_color[0] - check[0]), abs(before_color[1] - check[1]), abs(before_color[2] - check[2]));
		std::cout << diff << std::endl;
		if (diff.x > 30 || diff.y > 30 || diff.z > 30){
			firstCaptureIndex = lastIndex - i;
			break;
		}
		else {
			before_color = check;
		}
	}

	//std::cout << firstCaptureIndex << std::endl;

	// 画像バッファからコード投影部分のみ抽出する
	for (unsigned int i = 0; i < code_texture.size(); ++i) {
		// 画像の参照場所
		unsigned int index = (firstCaptureIndex - 1) - (i * proj_num);
		//// debug
		//char buf[256];
		//sprintf_s(buf, "./Calibration/pgr/cap%d.jpg", i);
		//cv::imwrite(buf, buff[index]);

		// 画像を保存
		// posi
		if (i < gcGetBitNum_all()) {
			if (i < gcGetBitNum_h())
				cv::imwrite(gcGetCaptureFilePath(i, HORIZONTAL, POSI).str(), buff[index]);
			else
				cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_h(), VERTICAL, POSI).str(), buff[index]);
		}
		// nega
		else if (i < gcGetBitNum_all() * 2){
			if (i - gcGetBitNum_all() < gcGetBitNum_h())
				cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_all(), HORIZONTAL, NEGA).str(), buff[index]);
			else
				cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_all() - gcGetBitNum_h(), VERTICAL, NEGA).str(), buff[index]);
		}
		// sine
		else if (i < gcGetBitNum_all() * 2 + psGetProjectionNum()) {
			// 縦縞
			cv::imwrite(psGetCaptureFilePath(i - gcGetBitNum_all() * 2, VERTICAL).str(), buff[index]);
		}
		// sine
		else {
			// 横縞
			cv::imwrite(psGetCaptureFilePath(i - gcGetBitNum_all() * 2 - psGetProjectionNum(), HORIZONTAL).str(), buff[index]);
		}

	}

	// 投影テクスチャをすべて削除
	code_texture.clear();
	code_texture.shrink_to_fit();

	// シェーダの無効
	GrayCode::glsl->setEnabled(false);

	// PGRカメラスレッドのバッファサイズを変更
	pgrCamera->resize(30);

	return true;
}

std::stringstream PhaseShift::psGetProjctionFilePath(unsigned int num, bool vertical)
{
	std::stringstream filename;
	//if (num >= Proj_num) {
	//	std::cerr << num << " 枚目の正弦波画像はありません" << std::endl;
	//	return filename;
	//}
	// 縦縞
	if (vertical)
		filename << ps_image_dir << "/ProjectionImages/x_pattern" << std::setw(2) << std::setfill('0') << num << ".bmp";
	// 横縞
	else
		filename << ps_image_dir << "/ProjectionImages/y_pattern" << std::setw(2) << std::setfill('0') << num << ".bmp";

	return filename;
}

std::stringstream PhaseShift::psGetCaptureFilePath(unsigned int num, bool vertical)
{
	std::stringstream filename;
	// 縦縞
	if (vertical)
		filename << ps_image_dir << "/CaptureImages/x_pattern" << std::setw(2) << std::setfill('0') << num << ".bmp";
	// 横縞
	else
		filename << ps_image_dir << "/CaptureImages/y_pattern" << std::setw(2) << std::setfill('0') << num << ".bmp";
	return filename;

}

/***********************************
** プロジェクタとカメラの対応付け **
************************************/

void PhaseShift::getCorrespondence(bool isCodeProjection)
{
	std::cout << "幾何対応取得開始" << std::endl;
	mask = cv::imread(gc_mask_address, 0);// マスク画像の読み込み
	if (!init_flag)
		init();

	if (isCodeProjection) {
		//復元処理
		code_restore();
		// データを保存
		writePixCorrespData();
	}
	else {
		// データの読み出し
		readPixCorrespData();
		// debug用に離散化させたマップも一応作成する
		calcIntCoordMap();
	}

	// (debug用)サブピクセル精度の対応を整数値に丸めて，対応マップを取得
	calcNearestNeighbor();

	// openglのメッシュを用いた幾何変換用のVAOを作成する
	cv::Mat vertCoordMap, texCoordMap;
	// カメラ座標系→プロジェクタ座標系の幾何変換用VAO
	//getGLCoordMap(vertCoordMap, texCoordMap, true);
	getBufferArrays(vertCoordMap, texCoordMap, coord_trans);
	ready = true;
}


// 復元処理
void PhaseShift::code_restore()
{
	std::cout << "位相復元開始 " << std::endl;

	// 位相復元
	restore_phase_value();

	// 連結処理
	phaseConnection();

	// 対応エラーを修正する
	errorCheck();

	std::cout << "位相復元終了 : ";
}

/* 位相復元 */
void PhaseShift::restore_phase_value()
{

	cv::Point2d lumi[Proj_num];
	cv::Mat *phase_x_img = new cv::Mat[Proj_num];
	cv::Mat *phase_y_img = new cv::Mat[Proj_num];

	cv::Mat xphase_img = cv::Mat(camHeight, camWidth, CV_32F);
	cv::Mat yphase_img = cv::Mat(camHeight, camWidth, CV_32F);

	// 画像の読み込み
	for (int i = 0; i < Proj_num; ++i)
	{
		phase_x_img[i] = cv::imread(psGetCaptureFilePath(i, VERTICAL).str(), 0);
		phase_y_img[i] = cv::imread(psGetCaptureFilePath(i, HORIZONTAL).str(), 0);
	}
	int height = (*phase_x_img).rows;
	int width = (*phase_x_img).cols;

	//復元処理
	//（Debug用）画像
	cv::Mat phasemask((*phase_x_img).rows, (*phase_x_img).cols, CV_8UC1, cv::Scalar::all(0));

	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){

			//マスク外は何もしない
			if (mask.ptr<uchar>(y)[x] == 0){
				continue;
			}

			cv::Point2d cosValue = cv::Point2d(0.0, 0.0);
			cv::Point2d sinValue = cv::Point2d(0.0, 0.0);

			//分子について
			for (int i = 0; i < Proj_num; i++){
				lumi[i] = cv::Point2d((double)phase_x_img[i].at<uchar>(y, x), (double)phase_y_img[i].at<uchar>(y, x));
				lumi[i] *= cos(2.0*PI*i / Proj_num);
				cosValue += lumi[i];
			}

			//分母について
			for (int i = 0; i < Proj_num; i++){
				lumi[i] = cv::Point2d((double)phase_x_img[i].at<uchar>(y, x), (double)phase_y_img[i].at<uchar>(y, x));
				lumi[i] *= sin(2.0*PI*i / Proj_num);
				sinValue += lumi[i];
			}

			//0割防止処理
			if (((0 <= abs(sinValue.x)) && (abs(sinValue.x) <= 1)) && cosValue.x > 0)
				sinValue.x = 1;
			if (((0 <= abs(sinValue.x)) && (abs(sinValue.x) <= 1)) && cosValue.x < 0)
				sinValue.x = -1;
			if (((0 <= abs(sinValue.y)) && (abs(sinValue.y) <= 1)) && cosValue.x > 0)
				sinValue.y = 1;
			if (((0 <= abs(sinValue.y)) && (abs(sinValue.y) <= 1)) && cosValue.x < 0)
				sinValue.y = -1;

			//位相復元
			cv::Point2d phase_value = cv::Point2d(atan2(cosValue.x, sinValue.x), atan2(cosValue.y, sinValue.y));

			//格納
			phaseValue.ptr<cv::Vec2d>(y)[x] = cv::Vec2d(phase_value.x, phase_value.y);
			//位相情報からマスクを作成する
			phasemask.at<uchar>(y, x) = 255;
			if (fabs(cosValue.x) < ps_thresh &&  fabs(cosValue.y) < ps_thresh){
				if (fabs(sinValue.x) < ps_thresh &&  fabs(sinValue.y) < ps_thresh)
					phasemask.at<uchar>(y, x) = 255;
			}

			// (Debug用) 正規化するために値を 0< value <2π にする
			xphase_img.at<float>(y, x) = (float)(phase_value.x + (PI));
			yphase_img.at<float>(y, x) = (float)(phase_value.y + (PI));


		}
	}

	/* マスク画像を二周り小さくする*/
	for (int i = 0; i < 2; i++)
		smallMaskRange(phasemask, phasemask);

	// (Debug用)鋸刃状の画像を作成&保存
	cv::Mat temp;
	xphase_img.convertTo(temp, CV_8U, 255 / (2.0*PI));
	cv::imwrite(std::string(ps_base_dir + "/phase_x.bmp").c_str(), temp);
	yphase_img.convertTo(temp, CV_8U, 255 / (2.0*PI));
	cv::imwrite(std::string(ps_base_dir + "/phase_y.bmp").c_str(), temp);

	// マスク画像を変更
	cv::imwrite(std::string(ps_base_dir + "/mask.bmp").c_str(), phasemask);
	mask = phasemask.clone();

	delete[] phase_x_img;
	delete[] phase_y_img;
	temp.release();

}


/* 連結処理 */
void PhaseShift::phaseConnection()
{
	// 撮影画像をデコード
	GrayCode::decodeGrayCode();
	std::map<int, cv::Point> *gray_code_map = new std::map<int, cv::Point>;
	cv::Mat graycode = GrayCode::gcGetGrayCode();

	//コードマップを作成( Key:グレイコード，value:波の座標データ(何周期目か) )
	for (unsigned int y = 0; y < waveNumY; y++) {
		for (unsigned int x = 0; x < waveNumX; x++) {
			int a = graycode.ptr<int>(y)[x];
			if (a != 0)
				(*gray_code_map)[a] = cv::Point(x, y);
		}
	}

	// 0番目は使わない
	(*gray_code_map)[0] = cv::Point(-1, -1);

	//連結処理(位相値を連続にする処理)を行う
	for (unsigned int y = 0; y < camHeight; y++){

		// ポインタを取得
		cv::Vec2d *phaseValue_p = phaseValue.ptr<cv::Vec2d>(y);
		cv::Vec2f *coordmap_p = coord_map.ptr<cv::Vec2f>(y);
		cv::Vec2i *coordmap_int_p = coord_map_int.ptr<cv::Vec2i>(y);

		for (unsigned int x = 0; x < camWidth; x++){

			//マスク外は何もしない
			if (mask.ptr<uchar>(y)[x] == 0){
				continue;
			}

			int a = GrayCode::gcGetDecodeMap().ptr<int>(y)[x];

			//コードが見つからない場合エラー処理
			if ((*gray_code_map).find(a) != (*gray_code_map).end())
			{
				// 波の座標(+1)を格納
				int nx = ((*gray_code_map)[a]).x + 1;
				int ny = ((*gray_code_map)[a]).y + 1;

				// 2nπ加算
				double phase_x = phaseValue_p[x][0] + (nx*2.0*PI);
				double phase_y = phaseValue_p[x][1] + (ny*2.0*PI);
				// 位相値を更新
				phaseValue_p[x] = cv::Vec2d(phase_x, phase_y);

				//対応するプロジェクタ座標(サブピクセル精度)を算出
				float point_x = (float)((phase_x * WaveLength_X) / (2.0 * PI) - (double)xplus);
				float point_y = (float)((phase_y * WaveLength_Y) / (2.0 * PI) - (double)yplus);

				// コードマップに保持
				coordmap_p[x] = cv::Vec2f(point_x, point_y);

				// 対応する座標を近傍の画素とする(Nearest Neighbor補完)
				coordmap_int_p[x] = cv::Vec2i((int)(point_x + 0.5f), (int)(point_y + 0.5f));

			}
			else
			{
				coordmap_p[x] = cv::Vec2f(-1.0f, -1.0f);
				coordmap_int_p[x] = cv::Vec2i(-1, -1);
			}
		}
	}
	delete gray_code_map;
	gray_code_map = NULL;
}

// 位相の連結エラーを行う関数
void PhaseShift::errorCheck()
{

	// 位相画像を作成
	cv::Mat xphase_img(camHeight, camWidth, CV_64F, cv::Scalar::all(0));
	cv::Mat yphase_img(camHeight, camWidth, CV_64F, cv::Scalar::all(0));
	int error;
	for (unsigned int y = 1; y < camHeight; y++){
		cv::Vec2i *int_cm_p = coord_map_int.ptr<cv::Vec2i>(y);
		cv::Vec2i *int_cm_p_up = coord_map_int.ptr<cv::Vec2i>(y - 1);

		cv::Vec2f *cm_p = coord_map.ptr<cv::Vec2f>(y);
		cv::Vec2f *cm_p_up = coord_map.ptr<cv::Vec2f>(y - 1);

		cv::Vec2d *phase_p = phaseValue.ptr<cv::Vec2d>(y);
		for (unsigned int x = 1; x < camWidth; x++){

			//マスク・エラー値の場合何もしない
			if (mask.ptr<uchar>(y)[x] == 0)
				continue;

			// 着目画素がエラー値だった場合何もしない
			if (int_cm_p[x] == cv::Vec2i(-1, -1))
				continue;

			/****************/
			/*  x について	*/
			/****************/
			// 着目画素の一つ前の画素がエラー値だった場合何もしない
			if (int_cm_p[x - 1] == cv::Vec2i(-1, -1))
				continue;

			int point = int_cm_p[x][0];
			int point_up = int_cm_p[x - 1][0];
			error = point - point_up;

			// 座標の推移がリニアではない場合(error < 0),着目画素の位相値に2π足して座標を算出し，
			// そのときの座標値が妥当なものかを判断する
			if (error < 0){
				double value = phase_p[x][0] + 2.0 * PI;
				double newPoint = (value * WaveLength_X) / (2.0 * PI) - (double)xplus;
				int p = (int)(newPoint + 0.5);
				// 再度1つ前の座標値との誤差量を計算
				error = p - point_up;

				// 修正後の座標がリニアに推移していた場合
				if (-1 <= error){
					//-グレイコードの境界誤差
					if (error < 5){
						// 正しい座標値を入れる
						cm_p[x][0] = (float)newPoint;
						int_cm_p[x][0] = p;
						phase_p[x][0] = value;
					}
					//-位相の誤差
					else{
						//c->p.pointdata[y][x].x = c->p.pointdata[y][x-1].x;
					}
				}
			}


			/****************/
			/*  y について	*/
			/****************/
			// 着目画素の一つ上の画素がエラー値だった場合何もしない
			if (int_cm_p_up[x] == cv::Vec2i(-1, -1))
				continue;

			point = int_cm_p[x][1];
			point_up = int_cm_p_up[x][1];
			error = point - point_up;
			//-座標の推移がリニアではない場合
			if (error < 0){
				double value = phase_p[x][1] + 2.0 * PI;
				double newPoint = (value * WaveLength_Y) / (2 * PI) - (double)yplus;
				int p = (int)(newPoint + 0.5);
				error = p - point_up;
				if (0 <= error){
					//-グレイコードの境界誤差
					if (error < 5){
						cm_p[x][1] = (float)newPoint;
						int_cm_p[x][1] = p;
						phase_p[x][1] = value;
					}
					//-位相の誤差
					else{
						//c->p.pointdata[y][x].y = c->p.pointdata[y-1][x].y;
					}
				}
			}

			// プロジェクタ座標の範囲を超えた場合のエラー処理
			if (cm_p[x][0] < 0.0f || cm_p[x][0]  > projWidth - 1.0f) {
				cm_p[x][0] = -1.0f;
				int_cm_p[x][0] = -1;

			}
			if (cm_p[x][1] < 0.0f || cm_p[x][1] > projHeight - 1.0f) {
				cm_p[x][1] = -1.0f;
				int_cm_p[x][1] = -1;
			}

			// (Debug用)
			xphase_img.at<double>(y, x) = phase_p[x][0];
			yphase_img.at<double>(y, x) = phase_p[x][1];

		}
	}

	// （確認用）画像で確認
	double nx = (int)(2.0*PI*(projWidth / WaveLength_X) + PI - 2.0*PI * 1 * (0 + xplus) / WaveLength_X);	//xの位相値の取り得る範囲(max - min)
	double ny = (int)(2.0*PI*(projHeight / WaveLength_Y) + PI - 2.0*PI * 1 * (0 + yplus) / WaveLength_Y);	//yの位相値の取り得る範囲(max - min)
	xphase_img.convertTo(xphase_img, CV_8U, 255 / nx);
	yphase_img.convertTo(yphase_img, CV_8U, 255 / ny);
	cv::imwrite(std::string(ps_base_dir + "/phase_restore_x.bmp").c_str(), xphase_img);
	cv::imwrite(std::string(ps_base_dir + "/phase_restore_y.bmp").c_str(), yphase_img);

	//（確認用）
	xphase_img.release();
	yphase_img.release();
}

/*
マスク領域を1pixel小さくする関数
投影領域の縁は信頼性が低いため，縁は幾何対応付けさせない．
*/
void PhaseShift::smallMaskRange(cv::Mat &src, cv::Mat &dst)
{
	cv::Mat newMask(camHeight, camWidth, CV_8UC1);

	for (unsigned int y = 0; y < camHeight; y++) {
		for (unsigned int x = 0; x < camWidth; x++) {
			if ((x > 0 && src.at<uchar>(y, x - 1) == 0 && src.at<uchar>(y, x) == 255)
				|| (y > 0 && src.at<uchar>(y - 1, x) == 0 && src.at<uchar>(y, x) == 255))
				newMask.at<uchar>(y, x) = 0;
			else if ((x < camWidth - 1 && src.at<uchar>(y, x) == 255 && src.at<uchar>(y, x + 1) == 0)
				|| (y < camHeight - 1 && src.at<uchar>(y, x) == 255 && src.at<uchar>(y + 1, x) == 0))
				newMask.at<uchar>(y, x) = 0;
			else if (src.at<uchar>(y, x) == 0)
				newMask.at<uchar>(y, x) = 0;
			else if (src.at<uchar>(y, x) == 255)
				newMask.at<uchar>(y, x) = 255;
		}
	}

	dst = newMask.clone();
}

void PhaseShift::writePixCorrespData()
{
	std::vector<float> sub_pointdataVector_X;
	std::vector<float> sub_pointdataVector_Y;
	for (unsigned int y = 0; y < camHeight; y++) {
		cv::Vec2f *float_pointer = coord_map.ptr<cv::Vec2f>(y);
		for (unsigned int x = 0; x < camWidth; x++) {
			sub_pointdataVector_X.emplace_back(float_pointer[x][0]);
			sub_pointdataVector_Y.emplace_back(float_pointer[x][1]);
		}
	}
	std::string fileName = COORDMAP_ADDRESS;
	cv::FileStorage fs(fileName, CV_STORAGE_WRITE);
	cv::write(fs, "subPointdata_x", sub_pointdataVector_X);
	cv::write(fs, "subPointdata_y", sub_pointdataVector_Y);

	sub_pointdataVector_X.clear();
	sub_pointdataVector_Y.clear();
}

void PhaseShift::readPixCorrespData()
{
	std::vector<float> sub_pointdataVector_X;
	std::vector<float> sub_pointdataVector_Y;

	std::string fileName = COORDMAP_ADDRESS;
	cv::FileStorage fs(fileName, cv::FileStorage::READ);
	fs["subPointdata_x"] >> sub_pointdataVector_X;
	fs["subPointdata_y"] >> sub_pointdataVector_Y;

	for (unsigned int y = 0; y < camHeight; y++) {
		cv::Vec2f *float_pointer = coord_map.ptr<cv::Vec2f>(y);
		for (unsigned int x = 0; x < camWidth; x++) {
			int key = y * camWidth + x;
			float_pointer[x] = cv::Vec2f(sub_pointdataVector_X[key], sub_pointdataVector_Y[key]);
		}
	}
	sub_pointdataVector_X.clear();
	sub_pointdataVector_Y.clear();

}

void PhaseShift::readPixCorrespData(const std::string &filename, cv::Mat &dst_map)
{
	cv::Mat map = cv::Mat(camHeight, camWidth, CV_32FC2);

	std::vector<float> sub_pointdataVector_X;
	std::vector<float> sub_pointdataVector_Y;

	std::string fileName = "./Data/" + filename;
	cv::FileStorage fs(fileName, cv::FileStorage::READ);
	fs["subPointdata_x"] >> sub_pointdataVector_X;
	fs["subPointdata_y"] >> sub_pointdataVector_Y;

	for (unsigned int y = 0; y < camHeight; y++) {
		cv::Vec2f *float_pointer = map.ptr<cv::Vec2f>(y);
		for (unsigned int x = 0; x < camWidth; x++) {
			int key = y * camWidth + x;
			float_pointer[x] = cv::Vec2f(sub_pointdataVector_X[key], sub_pointdataVector_Y[key]);
		}
	}
	sub_pointdataVector_X.clear();
	sub_pointdataVector_Y.clear();
	sub_pointdataVector_X.shrink_to_fit();
	sub_pointdataVector_Y.shrink_to_fit();

	dst_map = map.clone();
	map.release();

}
// サブピクセル精度の対応点を整数値に丸める関数
void PhaseShift::calcIntCoordMap()
{

	for (unsigned int y = 0; y < camHeight; y++) {
		cv::Vec2f *coord_p = coord_map.ptr<cv::Vec2f>(y);
		cv::Vec2i *coord_int_p = coord_map_int.ptr<cv::Vec2i>(y);
		for (unsigned int x = 0; x < camWidth; x++) {
			if (coord_p[x][0] != -1.0f)
				// 対応する座標を近傍の画素とする(Nearest Neighbor補完)
				coord_int_p[x] = cv::Vec2i((int)(coord_p[x][0] + 0.5f), (int)(coord_p[x][1] + 0.5f));
		}
	}
}

//　二アレストネイバー補完の場合
void PhaseShift::calcNearestNeighbor()
{

	for (unsigned int y = 0; y < camHeight; y++) {
		for (unsigned int x = 0; x < camWidth; x++) {
			cv::Point prj_point = coord_map_int.ptr<cv::Vec2i>(y)[x];
			if (prj_point.x != -1 && prj_point.y != -1)
				coord_map_proj.ptr<cv::Vec2i>(prj_point.y)[prj_point.x] = cv::Point(x, y);
		}
	}
}

//　幾何変換
void PhaseShift::reshapeCam2Proj(cv::Mat &src, cv::Mat &dst)
{

	if (!ready) {
		std::cout << "対応マップが取得されていません" << std::endl;
		return;
	}
	cv::Mat tmp(projHeight, projWidth, src.type());
	//dst = cv::Mat(src.size(), src.type(), cv::Scalar(0, 0, 0)); 

	if (src.channels() == 1){
		for (unsigned int y = 0; y < projHeight; y++) {
			for (unsigned int x = 0; x < projWidth; x++) {
				cv::Point p = coord_map_proj.ptr<cv::Vec2i>(y)[x];
				if (p.x != -1) {
					tmp.ptr<uchar>(y)[x] = src.ptr<uchar>(p.y)[p.x];
				}
			}
		}
	}
	if (src.channels() == 3) {
		for (unsigned int y = 0; y < projHeight; y++) {
			for (unsigned int x = 0; x < projWidth; x++) {
				cv::Point p = coord_map_proj.ptr<cv::Vec2i>(y)[x];
				if (p.x != -1) {
					tmp.ptr<cv::Vec3b>(y)[x] = src.ptr<cv::Vec3b>(p.y)[p.x];
				}
			}
		}
	}
	tmp.copyTo(dst);
	std::cout << "幾何補正 : ";
}

// 幾何変換用の頂点配列を取得する
void PhaseShift::getBufferArrays(cv::Mat &in_vertex, cv::Mat &in_texture, PS_Vertex *mesh)
{

	// 頂点情報
	cv::Mat indexMap = cv::Mat(camHeight, camWidth, CV_32S, cv::Scalar::all(-1));
	std::vector<PS_Vertex::MyVertex> vertex;
	std::vector<float> referVec;	// 参照用

	// 対応がとれたすべての画素の vertex buffer を取得
	unsigned int count = 0;
	for (unsigned int y = 0; y < camHeight; ++y) {
		int *index_p = indexMap.ptr<int>(y);

		for (unsigned int x = 0; x < camWidth; ++x) {

			// 左下を原点としたときのガメラ画素の番号を取得
			int cameraIndex = (y * -1 + (camHeight - 1)) * camWidth + x;

			// 対応マップから値を取得
			cv::Vec2f projPoint = coord_map.ptr<cv::Vec2f>(y)[x];

			//// 対応点が計測できている画素の頂点座標とテクスチャ座標をvectorに保持する
			//if (projPoint[0] != -1.0f && projPoint[1] != -1.0f){
			//	// 頂点情報をセットでvectorに保持
			//	cv::Vec2f camPoint = cv::Vec2f(x, y);
			//	vertex.emplace_back(PS_Vertex::MyVertex(projPoint, camPoint));

			//	// カメラ画素参照用のバッファを作成する(openGLのテクスチャ座標を中心としたカメラ画素の番号を取得)
			//	referVec.emplace_back((float)cameraIndex);
			//	// 格納した画素に番号を割り振る(インデックスバッファで参照するため)
			//	index_p[x] = count;
			//	count++;
			//}

			cv::Vec2f camPoint = cv::Vec2f(x, y);
			vertex.emplace_back(PS_Vertex::MyVertex(projPoint, camPoint));
			// カメラ画素参照用のバッファを作成する(op
			referVec.emplace_back((float)cameraIndex);
			// 格納した画素に番号を割り振る(インデックスバッファで参照するため)
			index_p[x] = count;
			count++;

		}
	}
	// メッシュを張る場合
	std::vector<PS_Vertex::MyIndex_TRIANGLES> index;
	// 対応がとれたすべての画素にメッシュを張るための，index bufferを取得
	for (unsigned int y = 0; y < camHeight - 1; ++y) {
		for (unsigned int x = 0; x < camWidth - 1; ++x) {

			// 着目画素，下, 右, 右下の画素を参照し，正常にとれていたら，三角メッシュの頂点にする
			cv::Vec2f pos = coord_map.ptr<cv::Vec2f>(y)[x];
			cv::Vec2f right = coord_map.ptr<cv::Vec2f>(y)[x + 1];
			cv::Vec2f down = coord_map.ptr<cv::Vec2f>(y + 1)[x];
			cv::Vec2f diag = coord_map.ptr<cv::Vec2f>(y + 1)[x + 1];

			// 着目画素が取れていない場合
			if (pos[0] == -1.0f) {
				// かつ、右、下、右下の画素が取れている場合
				if (right[0] != -1.0f && down[0] != -1.0f && diag[0] != -1.0f) {
					// メッシュを張る頂点画素の番号を，vectorに保持(２つ)
					int right_index = indexMap.ptr<int>(y)[x + 1];
					int down_index = indexMap.ptr<int>(y + 1)[x];
					int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(diag_index, right_index, down_index));
				}
			}
			// 着目画素が取れている場合
			else {
				// 3点とれている場合
				if (right[0] != -1.0f && down[0] != -1.0f && diag[0] != -1.0f) {
					// メッシュを張る頂点画素の番号を，vectorに保持(２つ)
					int pos_index = indexMap.ptr<int>(y)[x];
					int right_index = indexMap.ptr<int>(y)[x + 1];
					int down_index = indexMap.ptr<int>(y + 1)[x];
					int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(pos_index, down_index, right_index));
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(diag_index, right_index, down_index));
				}
				// 右画素がとれていない場合
				else if (right[0] == -1.0f && down[0] != -1.0f && diag[0] != -1.0f) {
					// メッシュを張る頂点画素の番号を，vectorに保持(２つ)
					int pos_index = indexMap.ptr<int>(y)[x];
					int down_index = indexMap.ptr<int>(y + 1)[x];
					int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(pos_index, down_index, diag_index));

				}
				// 下画素がとれていない場合
				else if (right[0] != -1.0f && down[0] == -1.0f && diag[0] != -1.0f) {
					// メッシュを張る頂点画素の番号を，vectorに保持(２つ)
					int pos_index = indexMap.ptr<int>(y)[x];
					int right_index = indexMap.ptr<int>(y)[x + 1];
					int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(pos_index, diag_index, right_index));

				}
				// 対角画素だけが取れていない場合
				else if (right[0] != -1.0f && down[0] != -1.0f && diag[0] == -1.0f) {
					// メッシュを張る頂点画素の番号を，vectorに保持(２つ)
					int pos_index = indexMap.ptr<int>(y)[x];
					int right_index = indexMap.ptr<int>(y)[x + 1];
					int down_index = indexMap.ptr<int>(y + 1)[x];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(pos_index, down_index, right_index));

				}
			}

		}
	}

	// 求めた頂点情報を用いて，頂点配列管理クラスPS_Vertexを更新
	mesh->setBuffers(vertex, index);
	//mesh->setCameraIndex(referVec);
}

void PhaseShift::getGLCoordMap(cv::Mat &dst_vertex, cv::Mat &dst_texture, bool cam2proj)
{
	dst_vertex = cv::Mat(camHeight, camWidth, CV_32FC2);
	dst_texture = cv::Mat(camHeight, camWidth, CV_32FC2);

	//float texel_x = GrayCode::coord_trans_mesh->getTexel().x;
	//float texel_y = GrayCode::coord_trans_mesh->getTexel().y;
	float texel_x = 0.0f;
	float texel_y = 0.0f;

	if (cam2proj) {

		// カメラ座標系→プロジェクタ座標系への変換
		for (unsigned int y = 0; y < camHeight; ++y) {
			for (unsigned int x = 0; x < camWidth; ++x) {

				cv::Point2f point = coord_map.ptr<cv::Vec2f>(y)[x];
				// 座標データがエラーの場合，範囲外の"-1.1"を格納
				if (point.x == -1.0f || point.y == -1.0f){
					dst_vertex.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
					dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
				}
				// 座標データが正常値で、かつマスク領域内の場合，-1 ～ 1に収まるよう正規化
				else{
					// 頂点座標を取得
					float vx = (point.x / (projWidth - 1.0f)) * 2.0f - 1.0f;
					float vy = -((point.y / (projHeight - 1.0f)) * 2.0f - 1.0f);
					dst_vertex.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(vx - texel_x, vy - texel_y);
					// テクスチャ座標を取得
					float u = (float)(x) / (camWidth - 1.0f);
					float v = -((float)(y) / (camHeight - 1.0f)) + 1.0f;
					dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(u, v);
				}
			}
		}
	}
	else {

		// プロジェクタ座標系→カメラ座標系への変換
		for (unsigned int y = 0; y < camHeight; ++y) {
			for (unsigned int x = 0; x < camWidth; ++x) {

				cv::Point2f point = coord_map.ptr<cv::Vec2f>(y)[x];
				// 座標データが正常値で、かつマスク領域内の場合，-1 ～ 1に収まるよう正規化
				if (point.x != -1.0f && point.y != -1.0f){
					// 頂点座標を取得
					//float vx = (point.x / (projWidth - 1.0f)) * 2.0f - 1.0f;
					//float vy = -((point.y / (projHeight - 1.0f)) * 2.0f - 1.0f);
					float vx = ((float)x / (camWidth - 1.0f)) * 2.0f - 1.0f;
					float vy = -(((float)y / (camHeight - 1.0f)) * 2.0f - 1.0f);
					dst_vertex.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(vx, vy);
					// テクスチャ座標を取得
					float u = (float)(point.x) / (projWidth - 1.0f);
					float v = -((float)(point.y) / (projHeight - 1.0f)) + 1.0f;
					dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(u, v);
				}
				// 座標データがエラーの場合，範囲外の"-1.1"を格納
				else{
					dst_vertex.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
					dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
				}
			}
		}
	}
}