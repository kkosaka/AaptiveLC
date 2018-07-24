#pragma once

#include <Windows.h>
#include <MMSystem.h>

#define USE_HIGH_PERFORMANCE_COUNTER	//������\�p�t�H�[�}���X�J�E���^���g���ꍇ��define


#pragma comment(lib, "winmm.lib")



namespace myUtl
{
	//FPS(�t���[�����[�g)����N���X
	class FPSMajor
	{
	private:
		double *fpss;
		int fps_index;
		int fpss_count;
		double start_time;
		double before_showed_time;

		LARGE_INTEGER timer_freq;


		double getTimeMiliSecond()
		{
#ifdef USE_HIGH_PERFORMANCE_COUNTER
			LARGE_INTEGER timer_counter;
			QueryPerformanceCounter(&timer_counter);

			return (double)timer_counter.QuadPart / (double)timer_freq.QuadPart * 1000.0;
#endif

			return (double)timeGetTime();
		}


	public:
		FPSMajor(int n)
		{
			fpss = new double[n];
			fpss_count = n;
			fps_index = 0;
			start_time = 0.0;
			before_showed_time = 0.0;

			QueryPerformanceFrequency(&timer_freq);

			for (int i = 0; i < fpss_count; i++)
				fpss[i] = 0.0;
		}

		~FPSMajor()
		{
			delete fpss;
		}

		void begin()
		{
			start_time = getTimeMiliSecond();
		}

		void end()
		{
			//fps�v��
			double mfps = 1.0 / ((getTimeMiliSecond() - start_time) / 1000.0);

			fpss[fps_index] = mfps;
			fps_index = (fps_index + 1) % fpss_count;
		}

		//FPS���擾����
		double get()
		{
			double s = 0;
			for (int i = 0; i < fpss_count; i++)
			{
				s += fpss[i];
			}

			return s / fpss_count;
		}

		//FPS��\������
		//interval: �\������Ԋu[ms]
		double show(double interval)
		{
			if (getTime() - before_showed_time >= interval)
			{
				before_showed_time = getTime();
				//printf("FPS : %.3lf\n", get());
				return get();
			}
			return 0;
		}

		//�Ō��begin()���Ăяo���Ă���̌o�ߎ���[ms]���擾����
		double getElapsed()
		{
			return getTimeMiliSecond() - start_time;
		}

		//�~���b�P�ʂ̎��Ԃ��擾����
		//timeGetTime() �܂��� QueryPerformanceCounter() �Ŏ擾�����l
		double getTime()
		{
			return getTimeMiliSecond();
		}

		//�Ō��begin()���Ăяo���Ă���̌o�ߎ��Ԃ��w�肵���t���[�����[�g�ɕK�v�Ȍo�ߎ���
		//�𒴂���܂őҋ@����
		//(�v�����FPS�������Ȃ肷���Ȃ��悤�ɒ�������)
		void adjustFrameRate(double fps)
		{
			double frameTime = 1.0 / fps * 1000.0;

			while (getElapsed() < frameTime);
		}
	};
}