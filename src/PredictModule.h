#ifndef PREDICT_MODULE_H_
#define PREDICT_MODULE_H_
#include "common.h"
class CSVMSolver; 
class CFeatureModule; 
class CImageDatabase; 
class CWarpingModule; 
class CFadeModule; 
class CParameter; 

class CPredictModule {
public: 
	CPredictModule(const CParameter* _param); 
	~CPredictModule();
    void SetImageAndPnts(Mat& _frame, Mat& _cropFrame, PointSetd _pnts);
	vectord PredictScore(); 
	void VisualizeScore(Mat& _img, const vectord& _preds, bool _isDraw, float _ratio = 1.0f); 
	void VisualizeGTScore(Mat& _img);
	Mat CrossFade(bool _isReset, float& _r, bool _isPause); 
	bool IsValid() { return m_isValid;}
	bool IsAutoSave(vectord preds);
	bool IsPeakFrame();
    Mat ReferenceFrame();
	Mat OriReferenceFrame();

	void Reset(bool _flag = false);
	double MatchCost() { return m_cost;}
	// get 
	Mat HalfBlend();
	Mat FullBlend();
	Mat Warp3D(); 

private: 
	void Clear();
//	void LoadExamplers(string _imgFold, string _dataFold);
    void LoadReference(string _imgFile);
	void LoadModels(string _modelFold);

private: 
	// parameters
	const CParameter* m_param; 
	CFeatureModule* m_featModule; 
	CImageDatabase* m_imgDB; 
	CWarpingModule* m_warpModule; 
	CFadeModule* m_dissolveModule; 
	// model and data
	vector<CSVMSolver*> m_models; 
    vectord m_modelScores;
	vectorString m_attrs; 
    PointSetd m_landmark;
	// frame info
    Mat m_example;
    Mat m_refFace;
	Mat m_cropFrame; 
    Mat m_frame;

    PointSetd m_pnts;
	float* m_feat; 
	// string 
	string m_dataDir; 

	bool m_isValid;

	int m_nFrames; 
	double m_cost;
	vectord m_scoreCache;
	double m_bestScore;
	bool m_isReset; 
};

#endif //PREDICT_MODULE_H_
