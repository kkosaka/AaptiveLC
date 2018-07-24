#ifndef MYPARAM_H
#define MYPARAM_H

#pragma once

// �p�X�̐ݒ�
#define MASK_ADDRESS "./Data/mask.bmp"
#define RECTMASK_ADDRESS "./Data/RectMask.bmp"
#define RECTVALUE_ADDRESS "./Data/Rect_value.txt"
#define RECTMASK_DEBUG_ADDRESS "./Data/RectMask(�m�F�p).jpg"
#define IMAGE_DIRECTORY "./Images"
#define SAVE_DIRECTORY "./Images/resize"

#define COORDMAP_ADDRESS "./Data/pointdata.xml"	// �Ή��}�b�v�̃t�@�C���p�X
#define COLOR_CALIBRATION_ADDRESS "./Data/CMM�v�Z�p.jpg"	// �J���[�L�����u���[�V�������ʃt�@�C���p�X
#define PROCAM_CALIBRATION_RESULT_FILENAME "./Data/calibration.xml"	// �L�����u���[�V�����t�@�C���p�X

// �V�X�e���ݒ�
#define FULLSCREEN 1	// �t���X�N���[��on/off
#define PROJECT_MONITOR_NUMBER (2)	//�v���W�F�N�^��ʔԍ�
#define SINGLE_BUFFERING 0	// �V���O���o�b�t�@�����Oon/off

// �e��p�����[�^�ݒ�
#define DlayTime 70		// ProCam�x����

#define GrayCodeThreshodlValue 25	// �O���C�R�[�h��臒l

#define FogetFactor 0.4		// RLS�̖Y�p�W��

#define TargetScale 0.4	// �ڕW�摜�̖��邳

#define ErrorDitectThreshold 7	// �G���[��f���o��臒l(�ڕW�l-�ϑ��l) 

#define maxPixelSize 4	// �G�s�������ւ̍ő�ψʗ�[pixel]( �}maxPixelSize[pixel]���炷 )

#define ZEROINDEX (maxPixelSize)	// �ψʗʂ�0�̎��̃C���f�b�N�X�ԍ�

#define searchSize (maxPixelSize * 2 + 1)	// �C���N�������g��

// �t���O�Ǘ�
#define FLAG_LC_LOOP	00001
#define FLAG_WRITE_FILE 00010
#define FLAG__IS_GEOM	00100
#define FLAG_IS_STOP	01000

#endif