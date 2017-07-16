#ifndef FILTER_H
#define	FILTER_H

typedef short fract16;
typedef long fract32;

typedef struct complex_fract16 {
	fract16 re, im;
} complex_fract16;

typedef struct complex_fract32 {
	fract32 re, im;
} complex_fract32;

/* * * *        cfftN      * * * *
 *
 *    N point radix 2 complex input FFT
 * 
 */
void cfft_fr16 (const complex_fract16 _input[], 	//Input array
                 complex_fract16 _temp[], 				//Temporary buffer
                 complex_fract16 _output[], 			//Output array
                 const complex_fract16 _twiddle_table[], 	//Twiddle table
                 int _twiddle_stride, int _fft_size, 			//Twiddle stride
                 int _block_exponent, int _scale_method);	//Not used



//描述:IIR滤波算法,去除交流信号,剩下CT检测的高次谐波
void iir(short* in, short* out, int n);  //, iir_coeffs  *coeffs

#endif

