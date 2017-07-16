#include "filter2.h"
#include "AcConst.h"
#include "Typedef.h"

#if FFT_NUM == 128
    #define N       128
    #define N_div_2 64
    #define Log2N   7
#elif FFT_NUM == 64
    #define N       64
    #define N_div_2 32
    #define Log2N   6
#elif FFT_NUM == 32
    #define N       32
    #define N_div_2 16
    #define Log2N   5
#endif

void fft(short * realin, short * imagin, short *realout, short *imagout)
{
#if FFT_NUM == 128
	const static short int lab[N]= {0, 64, 32, 96, 16, 80, 48, 112, 8, 72, 40, 104, 24, 88, 56, 120, 4, 68, 36, 100, 20, 84, 52, 116, 12, 76, 44, 108, 28, 92, 60, 124, 2, 66, 34, 98, 18, 82, 50, 114, 10, 74, 42, 106, 26, 90, 58, 122, 6, 70, 38, 102, 22, 86, 54, 118, 14, 78, 46, 110, 30, 94, 62, 126, 1, 65, 33, 97, 17, 81, 49, 113, 9, 73, 41, 105, 25, 89, 57, 121, 5, 69, 37, 101, 21, 85, 53, 117, 13, 77, 45, 109, 29, 93, 61, 125, 3, 67, 35, 99, 19, 83, 51, 115, 11, 75, 43, 107, 27, 91, 59, 123, 7, 71, 39, 103, 23, 87, 55, 119, 15, 79, 47, 111, 31, 95, 63, 127};
	const static short int twid_real[N_div_2] = {0x7fff, 0x7fd7, 0x7f61, 0x7e9c, 0x7d89, 0x7c29, 0x7a7c, 0x7883, 0x7640, 0x73b5, 0x70e1, 0x6dc9, 0x6a6c, 0x66ce, 0x62f1, 0x5ed6, 0x5a81, 0x55f4, 0x5133, 0x4c3f, 0x471c, 0x41cd, 0x3c56, 0x36b9, 0x30fb, 0x2b1e, 0x2527, 0x1f19, 0x18f8, 0x12c7, 0x0c8b, 0x0647, 0x0000, 0xf9b9, 0xf375, 0xed39, 0xe708, 0xe0e7, 0xdad9, 0xd4e2, 0xcf05, 0xc947, 0xc3aa, 0xbe33, 0xb8e4, 0xb3c1, 0xaecd, 0xaa0c, 0xa57f, 0xa12a, 0x9d0f, 0x9932, 0x9594, 0x9237, 0x8f1f, 0x8c4b, 0x89c0, 0x877d, 0x8584, 0x83d7, 0x8277, 0x8164, 0x809f, 0x8029};
	const static short int twid_imag[N_div_2] = {0x0000, 0xf9b9, 0xf375, 0xed39, 0xe708, 0xe0e7, 0xdad9, 0xd4e2, 0xcf05, 0xc947, 0xc3aa, 0xbe33, 0xb8e4, 0xb3c1, 0xaecd, 0xaa0c, 0xa57f, 0xa12a, 0x9d0f, 0x9932, 0x9594, 0x9237, 0x8f1f, 0x8c4b, 0x89c0, 0x877d, 0x8584, 0x83d7, 0x8277, 0x8164, 0x809f, 0x8029, 0x8002, 0x8029, 0x809f, 0x8164, 0x8277, 0x83d7, 0x8584, 0x877d, 0x89c0, 0x8c4b, 0x8f1f, 0x9237, 0x9594, 0x9932, 0x9d0f, 0xa12a, 0xa57f, 0xaa0c, 0xaecd, 0xb3c1, 0xb8e4, 0xbe33, 0xc3aa, 0xc947, 0xcf05, 0xd4e2, 0xdad9, 0xe0e7, 0xe708, 0xed39, 0xf375, 0xf9b9};
#elif FFT_NUM == 64
  	//64样点的比特反转倒序表
	const static short int lab[N]= {0x00, 0x20, 0x10, 0x30, 0x08, 0x28, 0x18, 0x38, 0x04, 0x24, 0x14, 0x34, 0x0C, 0x2C, 0x1C, 0x3C,
						   	  0x02, 0x22, 0x12, 0x32, 0x0A, 0x2A, 0x1A, 0x3A, 0x06, 0x26, 0x16, 0x36, 0x0E, 0x2E, 0x1E, 0x3E,
						   	  0x01, 0x21, 0x11, 0x31, 0x09, 0x29, 0x19, 0x39, 0x05, 0x25, 0x15, 0x35, 0x0D, 0x2D, 0x1D, 0x3D,
						 	  0x03, 0x23, 0x13, 0x33, 0x0B, 0x2B, 0x1B, 0x3B, 0x07, 0x27, 0x17, 0x37, 0x0F, 0x2F, 0x1F, 0x3F};
	const static short int twid_real[N_div_2]={0x7fff, 0x7f61, 0x7d89, 0x7a7c, 0x7640, 0x70e1, 0x6a6c, 0x62f1, 0x5a81, 0x5133, 0x471c, 0x3c56, 0x30fb, 0x2527, 0x18f8, 0x0c8b, 0x0000, 0xf375, 0xe708, 0xdad9, 0xcf05, 0xc3aa, 0xb8e4, 0xaecd, 0xa57f, 0x9d0f, 0x9594, 0x8f1f, 0x89c0, 0x8584, 0x8277, 0x809f};
	const static short int twid_imag[N_div_2]={0x0000, 0xf375, 0xe708, 0xdad9, 0xcf05, 0xc3aa, 0xb8e4, 0xaecd, 0xa57f, 0x9d0f, 0x9594, 0x8f1f, 0x89c0, 0x8584, 0x8277, 0x809f, 0x8002, 0x809f, 0x8277, 0x8584, 0x89c0, 0x8f1f, 0x9594, 0x9d0f, 0xa57f, 0xaecd, 0xb8e4, 0xc3aa, 0xcf05, 0xdad9, 0xe708, 0xf375};
#elif FFT_NUM == 32
  	//32样点的比特反转倒序表
	const static short int lab[N]= {0x00, 0x10, 0x08, 0x18, 0x04, 0x14, 0x0C, 0x1C, 
                              0x02, 0x12, 0x0A, 0x1A, 0x06, 0x16, 0x0E, 0x1E, 
						   	  0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0D, 0x1D,                    
						 	  0x03, 0x13, 0x0B, 0x1B, 0x07, 0x17, 0x0F, 0x1F};
	const static short int twid_real[N_div_2]={0x7fff, 0x7d89, 0x7640, 0x6a6c, 0x5a81, 0x471c, 0x30fb, 0x18f8, 0x0000, 0xe708, 0xcf05, 0xb8e4, 0xa57f, 0x9594, 0x89c0, 0x8277};
	const static short int twid_imag[N_div_2]={0x0000, 0xe708, 0xcf05, 0xb8e4, 0xa57f, 0x9594, 0x89c0, 0x8277, 0x8002, 0x8277, 0x89c0, 0x9594, 0xa57f, 0xb8e4, 0xcf05, 0xe708};
#endif

	int i,j,k;
	int iGroups,iBflysPerGroup,iNodeSpace;
	int iGroupStart,iX0Ptr,iX1Ptr,iTwidExp;
	int lT0,lT1;

  	for (i=0;i<N;i++)               /*比特反转倒序*/
  	{
    	k=lab[i];
    	realout[i] = realin[k];
    	imagout[i]=0;
  	}
  
 	iGroups = N_div_2;//总共16组
 	iBflysPerGroup = 1;//每组1个蝶形
 	iNodeSpace = 1;//每组一个接点
 
 	for	(i=0; i<Log2N; i++)               /*stage_loop compute all stages in FFT N=5*/
 	{
   		iGroupStart=0;
  		for (j=0;j<iGroups;j++)     /*group_loop compute all groups in stage*/
     	{
       		iX0Ptr=iGroupStart;
       		for(k=0;k<iBflysPerGroup;k++)
         	{
           		iTwidExp=iGroups*k;
           		iX1Ptr=iX0Ptr+iNodeSpace;
           		lT0=((long)twid_real[iTwidExp]*realout[iX1Ptr]-(long)twid_imag[iTwidExp]*imagout[iX1Ptr])>>15;  /*      iT0 = (c)x1-(-s)y1      */
           		lT1=((long)twid_real[iTwidExp]*imagout[iX1Ptr]+(long)twid_imag[iTwidExp]*realout[iX1Ptr])>>15;  /*      iT1 = (c)y1+(-s)x1      */
           		realout[iX1Ptr]=(realout[iX0Ptr]-lT0)>>1;
           		imagout[iX1Ptr]=(imagout[iX0Ptr]-lT1)>>1;
           		realout[iX0Ptr]=(realout[iX0Ptr]+lT0)>>1;
           		imagout[iX0Ptr]=(imagout[iX0Ptr]+lT1)>>1;                       
           		iX0Ptr++;
         	}
       		iGroupStart+=iBflysPerGroup<<1;          
     	}
   		iGroups=iGroups>>1;
   		iBflysPerGroup=iBflysPerGroup<<1;
   		iNodeSpace=iBflysPerGroup;
 	}
 
  	for (i=0;i<N;i++)               /*比例调整最后一次不用进行，补回。*/
  	{
    	realout[i] = realout[i]<<1;
    	imagout[i] = imagout[i]<<1;
  	}
}

short sRealin[FFT_NUM], sRealout[FFT_NUM], sImagout[FFT_NUM];
void cfft_fr16 (const complex_fract16 _input[], 	//Input array
                 complex_fract16 _temp[], 				//Temporary buffer
                 complex_fract16 _output[], 			//Output array
                 const complex_fract16 _twiddle_table[], 	//Twiddle table
                 int _twiddle_stride, int _fft_size, 			//Twiddle stride
                 int _block_exponent, int _scale_method)
{
	int i;
	for (i=0; i<FFT_NUM; i++)
	{
		sRealin[i] = _input[i].re;
	}
	
	fft(sRealin, NULL, sRealout, sImagout);
	
	for (i=0; i<FFT_NUM; i++)
	{
		_output[i].re = sRealout[i];
		_output[i].im = sImagout[i];
	}
}

//描述:IIR滤波算法,去除交流信号,剩下CT检测的高次谐波
void iir(short* in, short* out, int n)  //, iir_coeffs  *coeffs
{
	/*int b0 = 0.29289 * 16384;
	int b1 = -2*0.29289 * 16384;
	int b2 = 0.29289 * 16384;
	int a1 = 0;
	int a2 = 0.17157 * 16384;*/
/*
	//800
	int b0 = 0.638945 * 16384;
	int b1 = -2 * 0.638945 * 16384;
	int b2 = 0.638945 * 16384;
	int a1 = -1.14298 * 16384;
	int a2 = 0.4128 * 16384; 
*/
	//1k
	const int b0 = (int)(0.569035 * 16384);
	const int b1 = (int)(-2 * 0.569035 * 16384);
	const int b2 = (int)(0.569035 * 16384);
	const int a1 = (int)(-0.942808 * 16384);
	const int a2 = (int)(0.33333 * 16384); 

/*	//500
	int b0 = 0.75707637 * 16384;
	int b1 = -2 * 0.75707637 * 16384;
	int b2 = 0.75707637 * 16384;
	int a1 = -1.454244 * 16384;
	int a2 = 0.5740619 * 16384; 
*/
	int x, y, x1=0, x2=0, y1=0, y2=0;
	int i;
	for (i=0; i<n; i++)
	{
		x = *in++;
		y = (b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2) >> 14; 
		x2 = x1;
		x1 = x;
		y2 = y1;
		y1 = y;
		*out++ = y;
	}
}
