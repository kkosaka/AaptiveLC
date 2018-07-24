#pragma once

#include <Windows.h>
#include <MMSystem.h>

#define USE_HIGH_PERFORMANCE_COUNTER	//高分解能パフォーマンスカウンタを使う場合はdefine


#pragma comment(lib, "winmm.lib")



namespace myUtl
{
	//FPS(フレームレート)測定クラス
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
			//fps計測
			double mfps = 1.0 / ((getTimeMiliSecond() - start_time) / 1000.0);

			fpss[fps_index] = mfps;
			fps_index = (fps_index + 1) % fpss_count;
		}

		//FPSを取得する
		double get()
		{
			double s = 0;
			for (int i = 0; i < fpss_count; i++)
			{
				s += fpss[i];
			}

			return s / fpss_count;
		}

		//FPSを表示する
		//interval: 表示する間隔[ms]
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

		//最後にbegin()を呼び出してからの経過時間[ms]を取得する
		double getElapsed()
		{
			return getTimeMiliSecond() - start_time;
		}

		//ミリ秒単位の時間を取得する
		//timeGetTime() または QueryPerformanceCounter() で取得した値
		double getTime()
		{
			return getTimeMiliSecond();
		}

		//最後にbegin()を呼び出してからの経過時間が指定したフレームレートに必要な経過時間
		//を超えるまで待機する
		//(要するにFPSが速くなりすぎないように調整する)
		void adjustFrameRate(double fps)
		{
			double frameTime = 1.0 / fps * 1000.0;

			while (getElapsed() < frameTime);
		}
	};
}