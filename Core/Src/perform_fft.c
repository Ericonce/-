#include "perform_fft.h"
#include "arm_math.h"
#include "arm_const_structs.h"


//float data[NPT];	//输入数据  
//float mag_output[NPT / 2];	//fft之后的数据

// 内部缓存
static float fft_output[NPT];
static float windowed[NPT];
static float hann[NPT];
static float mean_hann;

/**
  * @brief 对输入数据执行加窗、FFT、幅值计算，并归一化
  * @param input   实数输入信号（长度为 NPT）
  * @param mag_out 输出归一化后的幅值数组（长度为 NPT / 2）
  */
void perform_fft(float *input, float *mag_out)
{

    // 生成窗函数系数
    arm_hanning_f32(hann, NPT);
	
		// 计算 hann 窗的均值
		arm_mean_f32(hann, NPT, &mean_hann);

		// 归一化窗函数
		for (int i = 0; i < NPT; i++) {
				hann[i] /= mean_hann;
		}

    // 乘以窗函数，得到 windowed[]
    for (int i = 0; i < NPT; i++) {
        windowed[i] = input[i] * hann[i];
    }

    // 初始化 FFT 实例
    arm_rfft_fast_instance_f32 S;
    arm_rfft_fast_init_f32(&S, NPT);

    // 执行 FFT（实数转复数）
    arm_rfft_fast_f32(&S, windowed, fft_output, 0);

    // 计算复数幅值
    arm_cmplx_mag_f32(fft_output, mag_out, NPT / 2);

    // 幅值归一化
    for (int i = 0; i < NPT / 2; i++)
    {
        mag_out[i] /= (NPT / 2);
    }
}
