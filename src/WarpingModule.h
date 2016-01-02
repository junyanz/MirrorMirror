#ifndef WARPING_MODULE_H_
#define WARPING_MODULE_H_
#include "common.h"

class CWarpingModule {
public: 
	CWarpingModule(); 
	//CWarpingModule(Mat _meanPnts);
	~CWarpingModule();
	
    void WarpToFrontalFace3D(const Mat& _img, const PointSetd& _pnts, bool _isWarp = true);
	//void EstimateMatrix(const Mat& _pnts, Mat& _ar, Mat& _t);
	Mat FrontalFace(); 
	PointSetd WarpedLandmark(); 
	Mat WarpFace(Mat _img);
	void Matrix(Mat& _ar, Mat& _H, Point2d& _t) { _ar = m_ar.clone(); _H = m_H.clone();_t = m_t;}
	Mat Warp2d() { return m_warp2d; }

private: 
	void Clear();
	void DrawBox( Mat& _img, Point2f* _pvec, Scalar _s, int _thickness); 


private: 
	Mat m_frontalFrame; 
	Mat m_meanPnts; 
	Mat m_pnts3d; 
	Mat m_Z; 
	//Mat m_mask; 
	Mat m_ar; 
	Mat m_H; 
	//Mat m_H_inv; 
	Point2d m_t; 
	Mat m_warp2d; 
	//Mat m_cropFrame; 
	PointSetd m_warpLandmarks; 
	//PointSeti m_maskCountour; 
};
#endif //WARPING_MODULE_H_
