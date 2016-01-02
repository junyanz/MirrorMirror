#ifndef BLEND_MODULE_H_
#define BLEND_MODULE_H_
#include "common.h"

class CBlendModule {
public: 
	static Mat LaplacianBlend(const Mat& _src, const Mat& _tgt, 
		const Mat& _mask, Rect _roi, float _r, Mat& _half, Mat& _full);
	static Mat DirectCopy( const Mat& _img1, const Mat& _img2, const Mat& _mask, float _r);

}; 

#endif //BLEND_MODULE_H_
