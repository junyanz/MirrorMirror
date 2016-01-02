#ifndef SAVE_MODULE_H_
#define SAVE_MODULE_H_
#include "common.h"

class CSaveModule {
public: 
	CSaveModule(string _foldPath); 
	~CSaveModule();

	void AutoSave(const Mat& _disp, const Mat& _frame, vectord _preds);
	void SavePeak(vectord _preds);
	void SaveFrame(const Mat& _disp, const Mat& _ref, const Mat& _frame, const Mat& _warp3d, const Mat& _half, const Mat& _full, const vectord& _scores, 
		double _cost, double _t, const PointSetd& _landmarks,  string _print=string());
	//void SaveMetaData();
	void ShowAll();
	void Reset(); 

private: 
	void Clear();
	void CreateFolder();

private: 
	string m_fold;
	string m_saveFold; 
	int m_nFrames; 
	int m_nArchives; 
	vectord m_scores;  
	bool m_isCreated;
	ImageSet m_frames; 
	ImageSet m_disps;
};
#endif //SAVE_MODULE_H_