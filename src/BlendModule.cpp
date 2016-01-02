#include "BlendModule.h"
#include "LaplacianBlend.h"
#include "Timer.h"

Mat CBlendModule::LaplacianBlend( const Mat& _src, const Mat& _tgt, const Mat& _mask, 
								 Rect _roi, float _r, Mat& _half, Mat& _full) {
	//CTimer* timer = new CTimer("blend");
	Mat src_f, tgt_f, mask_f; 
	Mat src_s = _src(_roi); 
	Mat tgt_s = _tgt(_roi);
	Mat mask_s = _mask(_roi);
	/*SHOW_IMG(src_s);
	SHOW_IMG(tgt_s);
	SHOW_IMG(mask_s);*/
	
	src_s.convertTo(src_f, CV_32F);
	tgt_s.convertTo(tgt_f, CV_32F);
	mask_s.convertTo(mask_f, CV_32F);
	mask_f = mask_f /255.0f; 
    CLaplacianBlend lb(src_f, tgt_f, mask_f);
	Mat paste_f = lb.LaplacianBlend(); 
	Mat out_f = paste_f * (1-_r) + tgt_f * _r;
	Mat half_f = paste_f * 0.5f + tgt_f * 0.5f; 
	Mat paste; 
	Mat out_c, half_c; 
	out_f.convertTo(out_c, CV_8U);
	half_f.convertTo(half_c, CV_8U);
	paste_f.convertTo(paste, CV_8U);
	//SHOW_IMG(out_c);

	Mat out = _tgt.clone();
	_half = _tgt.clone();
	_full = _tgt.clone();
	out_c.copyTo(out(_roi));
	half_c.copyTo(_half(_roi));
	paste.copyTo(_full(_roi));
	//SHOW_IMG(out);
	//DELETE_OBJECT(timer);

	return out; 
}

Mat CBlendModule::DirectCopy( const Mat& _img1, const Mat& _img2, const Mat& _mask, float _r) {
	Mat blendIm_f, blendIm, img1_f, img2_f; 
	_img1.convertTo(img1_f, CV_32F);
	_img2.convertTo(img2_f, CV_32F);
	blendIm_f = img1_f * (1-_r) + img2_f * _r; 
	blendIm_f.convertTo(blendIm, CV_8U);
	Mat out = _img2.clone();
	blendIm.copyTo(out, _mask);
	return out; 
}
