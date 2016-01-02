#ifndef MIRROR_MAIN_
#define MIRROR_MAIN_
#include "common.h"
#include "framerate.hpp"
#include "Timer.h"

class CRecordModule; 
class CWarpingModule; 
class CCLMTrackingModule;
class CTrainModule; 
class CPredictModule; 
class CParameter; 
class CSaveModule; 

class CMirrorMain {
public: 
    CMirrorMain(const CParameter* _param);
	~CMirrorMain();
	void Run(); 

private: 
	void Clear();
	void RunWebCameraMode(); 
//	void RunVideoMode();
	void TrainModel(); 
	float Predict(Mat _img, PointSeti _pnts); 
	char KeyboardInput(); 
	void ProcessFrame(Mat _frame); 
	void DrawFPS(Mat& _disp);
	void SetUpWindows();
	void ImShowRatio(string _name, const Mat& _im);
	double ReadRatio(); 
	void SaveRatio(double _r);
    void ShowRefFrame();

private: 
	// parameter
    const CParameter* m_param;
	// module
	CRecordModule* m_recordModule; 
	CWarpingModule* m_warpModule; 
	CTrainModule* m_trainModule; 
	CPredictModule* m_predictModule;
    CCLMTrackingModule* m_tracker;
	CSaveModule* m_saveModule;
	utils::CFrameRate* m_frameRate; 

	// save landmarks
    vector<PointSetd> m_recentPnts;
	// save image
	Mat m_frame; 
	Mat m_disp;
	Mat m_peakFrame;
	Mat m_peakDisp;
	Mat m_peak3D; 
	Mat m_peakHalf; 
	Mat m_peakFull;
	Mat m_peakRef; 
	vectord m_peakPreds;
	double m_peakCost;
	double m_peakT;
	PointSetd m_peakPnts;
	vectord m_preds;
	// timer 
	CTimer* m_timer; 
	double m_cost;
	PointSetd m_pnts;
	// flag
	int m_nFrames; 
	int m_screenWidth; 

	bool m_isPause; 
	bool m_isPrint; 
	bool m_isReset; 
	// window name
	string m_mainName; 
	string m_refName;
	string m_oriName;
	double m_ratio; 
};

#endif  //CMIRROR_MAIN_
