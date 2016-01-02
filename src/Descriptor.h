#ifndef DESCRIPTOR_H_
#define DESCRIPTOR_H_
#include "common.h"

class CDescriptor {
public:
	//virtual CDescriptor(){}
	virtual ~CDescriptor() { }
	virtual void ComputeFeature(Mat _img, float*& _feats) = 0;
	virtual int FeatureLength()= 0;
	virtual string DescriptorName() = 0;
private:
        //virtual void SetParameters(CConfigFile _config) = 0;
	virtual void CheckParameters() = 0;
	virtual void ComputeFeatureLength() = 0;
	virtual void Clear() = 0;
protected:
	int m_lenFtr;
};

#endif //DESCRIPTOR_H_
