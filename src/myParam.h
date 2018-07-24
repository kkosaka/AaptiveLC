#ifndef MYPARAM_H
#define MYPARAM_H

#pragma once

// パスの設定
#define MASK_ADDRESS "./Data/mask.bmp"
#define RECTMASK_ADDRESS "./Data/RectMask.bmp"
#define RECTVALUE_ADDRESS "./Data/Rect_value.txt"
#define RECTMASK_DEBUG_ADDRESS "./Data/RectMask(確認用).jpg"
#define IMAGE_DIRECTORY "./Images"
#define SAVE_DIRECTORY "./Images/resize"

#define COORDMAP_ADDRESS "./Data/pointdata.xml"	// 対応マップのファイルパス
#define COLOR_CALIBRATION_ADDRESS "./Data/CMM計算用.jpg"	// カラーキャリブレーション結果ファイルパス
#define PROCAM_CALIBRATION_RESULT_FILENAME "./Data/calibration.xml"	// キャリブレーションファイルパス

// システム設定
#define FULLSCREEN 1	// フルスクリーンon/off
#define PROJECT_MONITOR_NUMBER (2)	//プロジェクタ画面番号
#define SINGLE_BUFFERING 0	// シングルバッファリングon/off

// 各種パラメータ設定
#define DlayTime 70		// ProCam遅延量

#define GrayCodeThreshodlValue 25	// グレイコードの閾値

#define FogetFactor 0.4		// RLSの忘却係数

#define TargetScale 0.4	// 目標画像の明るさ

#define ErrorDitectThreshold 7	// エラー画素検出の閾値(目標値-観測値) 

#define maxPixelSize 4	// エピ線方向への最大変位量[pixel]( ±maxPixelSize[pixel]ずらす )

#define ZEROINDEX (maxPixelSize)	// 変位量が0の時のインデックス番号

#define searchSize (maxPixelSize * 2 + 1)	// インクリメント数

// フラグ管理
#define FLAG_LC_LOOP	00001
#define FLAG_WRITE_FILE 00010
#define FLAG__IS_GEOM	00100
#define FLAG_IS_STOP	01000

#endif