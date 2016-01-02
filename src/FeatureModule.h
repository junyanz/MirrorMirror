#ifndef FEATURE_MODULE_H_
#define FEATURE_MODULE_H_
#include "common.h"

class CFeatureModule {
public: 
    CFeatureModule();
	~CFeatureModule();
	//float* GetFeature(const Mat& _img, PointSetd _landmarks, bool _isVis = false);
	float* GetFeature(const Mat& _img, bool _isVis = false);
	int FeatureLength() {return m_ndims; } 

private: 
	void Clear();
	Rect LocalizeParts(PointSetd _landmraks, int _idx[], int _n, Size _sz); 
	

private: 
	//bool m_useHOG; 
	//bool m_useLandmarks; 
	int m_ndims; 
}; 
#endif //FEATURE_MODULE_H_
