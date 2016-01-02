#ifndef WARP_UTILS_H_
#define WARP_UTILS_H_
#include "common.h"

class CWarpUtils {

public:
	static Mat EstimateH( PointSetd _pnt1, PointSetd _pnt2, bool _flag); 
	static PointSetd Mat2Pnts( Mat _matPnts );
	static PointSetd Mat2Pnts2(Mat _matPnts);
	static Mat Pnts2Mat(const PointSetd& _pnts);
	static Rect GetRect(Mat _pnts, int _border = 0);
	static Rect GetRect(PointSetd _pnts, int _border = 0);
    static PointSetd MedianPnts(const vector<PointSetd>& _allPnts );
	//static Mat LoadMeanPnts( string _path );
	static Mat LoadMeanPnts3D(string _path);
	static double PntDist(Point2d _p1, Point2d _p2);
	static PointSetd ProjPnts(const PointSetd _p, const Mat& _H);
	static Mat CropFrame(const Mat& _img, PointSetd _pnts, int _border);
	static Mat CropFrame(const Mat& _img, PointSetd _pnts, vectori _border);
	static Mat CropFace(const Mat& _img, PointSetd _pnts, vectord _ratios=vectord());
	static Rect FaceROI(PointSetd _pnts, vectord _ratios=vectord());
	static void EstimateHWeighted(const PointSetd a, const PointSetd b, const vectord _w, CvMat* M, int full_affine);
	static Mat EstimateHWeighted( PointSetd _pnt1, PointSetd _pnt2, bool _flag);
	static Rect RecfityROI(Rect _roi, Size _imgSz);
	static Mat LoadZ(string _zFile);
};
#endif //WARP_UTILS_H_
