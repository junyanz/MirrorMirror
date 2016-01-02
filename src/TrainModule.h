#ifndef TRAIN_MODULE_H_
#define TRAIN_MODULE_H_
#include "common.h"

class CImageDatabase; 
class CWarpingModule; 
class CFeatureModule; 
class CCLMTrackingModule;
class CSVMSolver; 
class CSaveModule; 
class CParameter; 

class CTrainModule {
public: 
	CTrainModule(const CParameter* _param); 
	~CTrainModule(); 
	void Train(); 

private: 
	void Clear(); 
	void NormalizeScore(vectord& _score); 
	//Mat LoadMeanPnts( string _path );

private: 
	// parameter 
	const CParameter* m_param; 
	// dir
	string m_imgFold; 
	string m_modelFold; 
	string m_scoreFold; 
	string m_imgList; 

	// image
	CImageDatabase* m_imgDB; 
	int m_nImgs; 

	// module
	CWarpingModule* m_warpModule; 
	CFeatureModule* m_featModule; 
    CCLMTrackingModule* m_detectModule;
//	CSVMSolver* m_svm;
	CSaveModule* m_saveModule; 
};



#endif //TRAIN_MODULE_H_
