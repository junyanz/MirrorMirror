#ifndef HOGDESCRIPTOR_H_
#define HOGDESCRIPTOR_H_
#include "Descriptor.h"

class CHOGDescriptor: public CDescriptor
{
public:
	CHOGDescriptor();
	virtual ~CHOGDescriptor();
	void ComputeFeature(Mat _img, float*& _feats);
	int CellSize() {return m_cellSize; }
	Size CellGrid() {return Size(m_out[1], m_out[0]); }
	Mat VisualizeHOG(float* _hog); 
	void SetDims(Size _imgSz, int _cellSize = 8); 
	int NumCells() const {return m_lenFtr / 31; };
	int FeatureLength();
	string DescriptorName() {return "HOG"; }
    //void SetParameters(CConfigFile _config) {}
    void TranslateHOG(const float* _o, float*& _out, Point2i _offset);
	void NormalizeHOG(float* _f); 
	void MinusHOG(float* _f); 
	void CropHOG(Rect _roi, int*& _cropIdx, int& _n); 
	void CropHOG(const Mat& _mask, int*& _cropIdx, int& _n); 
	float HOGProduction(float* _f, float* _g, Point2i _offset, int* _idx, int _numIdx, bool _isVis); 

private:
	void Clear();
	float* GetHOG(Mat _img);
	float* GetHOGGray(Mat _img); 
	void CheckParameters() {}
	void ComputeFeatureLength();
	float* CV_GetHOG(Mat _img); 
	// VLFeat HOG implementation
	float* VL_GetHOG(Mat _img); 
	float* VL_Mat2Float(Mat _img); 
	Mat VL_Float2Mat(float* _f, int _height, int _width); 
	Mat VL_HOGImage(float* _f); 

private:
	int m_cellSize;
	int m_hogDimension; 
	int m_dims[2];   //m_dims[0]: height, m_dims[1]: width
	int m_out[3];
	int m_blocks[2];
	int m_imageWidth; 
	int m_imageHeight; 
	int m_hogWidth; 
	int m_hogHeight; 
	//VlHog* m_vlhog; 
};

#endif //HOGDESCRIPTOR_H_
