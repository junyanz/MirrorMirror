#ifndef DISSOLVE_MODULE_H_
#define DISSOLVE_MODULE_H_
#include "common.h"
#include <GL/glut.h>
#include <GL/glu.h>
class CWarpingModule;

//typedef struct {
//	int width;
//	int height;
//	char* title;

//	float field_of_view_angle;
//	float z_near;
//	float z_far;
//} glutWindow;



class CFadeModule {
public: 
    CFadeModule();
	~CFadeModule(); 
	void SetTexture(const Mat& _texture) { m_texture = _texture; }
    Mat Fade3D( const Mat& _tgt, PointSetd _srcPnts, PointSetd _tgtPnts, float _r, bool _flag);
	Mat Warp3d() { return m_warp3d; }
	Mat HalfBlend() { return m_halfBlend;}
	Mat FullBlend() { return m_fullBlend; }
	void Reset(); 
	double Error() {return m_error; }
	

private: 
	// get contours
	void LoadContours();
	// 3d
	Mat Render3D(const Mat& _Z, const Mat& _texture, const Mat& _ar, Point2d _t);
	PointSeti Contours(const Mat& _Z, const Mat& _ar, Point2d _t, const Mat& _H);
	PointSetd ProjPnts(const Mat& _ar, Point2d _t, const Mat& _H);
    void Init();
	//void display();
    Mat DrawFace(Size _size, const Mat& _Z, const Mat& _texture);
	//void ColorFit(const Mat& _src, const Mat& _dst, const Mat& _mask);
	void InitList(const Mat& _Z, const Mat& _texture, float _size); 


	void Clear();
	// draw
	Mat DrawPoints(const Mat& _img, const PointSetd& _p, const PointSetd& _q);
//	Mat DrawArrows(const Mat& _img, PointSetd _startPnt, PointSetd _endPnt);
	Mat DrawMask(const Mat& _img, const PointSeti& _mask);
//	void DrawArrow(Mat& _img, Point2d _p, Point2d _q);
	void DrawEllipse(const Mat& _img, string _name, RotatedRect _e);
	// color match
	Mat EstimateHistMatch(const Mat& _src, const Mat& _tgt, const Mat& _mask);
	Mat ColorMatch(const Mat& _src, const ImageSet& _luts);
	PointSeti CreatFaceMask(const Mat& _img, const PointSetd& _pnts, Mat& _mask, Rect& _roi);
	double AlignmentError(const PointSetd& _pnt1, const PointSetd& _pnts2);
	//double MatchCost(const Mat& _img1, const Mat& _img2, const Mat& _mask, PointSetd& _pnt1, PointSetd& _pnt2);
	//Mat MixImage(const Mat& _img1, const Mat& _img2);
private: 

	int m_gridSz; 
	float m_alpha; 
	float m_transRatio; 
	Mat m_H; 
	vector<Mat> m_Hs;
	int m_nFrames;
//	int m_nFramesH;
	ImageSet m_luts;
	typedef vector<vectord> CDF; 
	vector<CDF> m_srcCDF;
	vector<CDF> m_tgtCDF; 
	PointSetd m_meanPnts; 
	bool m_isDebug;
	double m_error;
	bool m_isFirst; 
	//double m_matchCost;

	// 3D
	CWarpingModule* m_warpModule; 
	Mat m_texture; 
	Mat m_Z; 
	//Mat m_mask; 
	PointSetd m_countours; 
	int m_count; 

	// display list
	GLuint m_list;
	GLuint m_maskList; 
	bool m_isInitList;

	// for debug
	Mat m_warp3d; 
	Mat m_halfBlend; 
	Mat m_fullBlend;
	Mat m_pnts3d; 
}; 

#endif //DISSOLVE_MODULE_H_
