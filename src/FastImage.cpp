#include "FastImage.h"

void FAST::CFastImage::Mask2Index(const Mat& _mask, int _pVec[], int& _nPixel) {
	int dataSz = _mask.rows * _mask.cols; 
	//cout << dataSz << endl; 
	size_t elemSize = _mask.elemSize();
	//cout << dataSz << " " << elemSize << endl; 
	_nPixel = 0; 

	FOR (i, dataSz) {
		if ((int)*((uchar*)(_mask.data+i*elemSize)) > 0) {

			//if (_nPixel == 0) {
				//imshow("mask", _mask); 
				//wait
			//}
			//cout << (int)*((uchar*)(_mask.data+i*elemSize))  << " "; 
			_pVec[_nPixel] = i; 
			//cout << _pVec[_nPixel] << " "; 
			_nPixel++; 
		} 
	}
}


float FAST::CFastImage::FastSSDCentral(Mat _img1, Mat _img2, Mat _weight, float _norm, const int _pVec[], int _nPixel) {
	unsigned long ssd = 0; 
	int err = 0; 
	size_t elemSize = _img1.elemSize(); 

	if (_img1.channels() == 1) {
		FOR (i, _nPixel) {
			err = (int)(*((uchar*)(_img1.data+_pVec[i]*elemSize)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize))); 
			ssd += (unsigned long)(err * err  * (int)(*((uchar*)(_weight.data+i*elemSize)))); 
		}
	} else {
		FOR (i, _nPixel) {
			err = (int)(*((uchar*)(_img1.data+_pVec[i]*elemSize)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize))); 
			ssd += (unsigned long)(err * err) * (int)(*((uchar*)(_weight.data+i*elemSize))); 
			err = (int)(*((uchar*)(_img1.data+_pVec[i]*elemSize+1)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize+1))); 
			ssd += (unsigned long)(err * err) * (int)(*((uchar*)(_weight.data+i*elemSize))); 
			err = (int)(*((uchar*)(_img1.data+_pVec[i]*elemSize+2)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize+2))); 
			ssd += (unsigned long)(err * err) * (int)(*((uchar*)(_weight.data+i*elemSize))); 
		}
	}
	 
	return (float)ssd / _norm; 
}


float FAST::CFastImage::FastSSD(Mat _img1, Mat _img2, const int _pVec[], int _nPixel) {
	float ssd = 0; 
	int err = 0; 
	size_t elemSize = _img1.elemSize(); 
	if (_img1.channels() == 1) {
		for (int i = 0; i < _nPixel; i++) {
			err = (int)(*((uchar*)(_img1.data+_pVec[i]*elemSize)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize))); 
			ssd += (float)(err * err); 
		}
	} else {
		for (int i = 0; i < _nPixel; i++) {
			err = (int)(*((uchar*)(_img1.data+_pVec[i]*elemSize)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize))); 
			ssd += (float)(err * err); 
			err = (int)(*((uchar*)(_img1.data+_pVec[i]*elemSize+1)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize+1))); 
			ssd += (float)(err * err); 
			err = (int)(*((uchar*)(_img1.data+_pVec[i]*elemSize+2)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize+2))); 
			ssd += (float)(err * err); 
		}
	}

	//SHOW(ssd); 
	return ssd; 
}

float FAST::CFastImage::FastSSD(Mat _img, uchar _p[], const int _pVec[], int _nPixel) {
	float ssd = 0; 
	int err = 0; 
	size_t elemSize = _img.elemSize(); 
	int p[3]; 
	FOR (i, _img.channels()) p[i] = (int)_p[i]; 
	//SHOW(elemSize); 
	if (_img.channels() == 1) {
		FOR (i, _nPixel) {
			err = (int)(*((uchar*)(_img.data+_pVec[i]*elemSize)))
				- p[0]; 
			ssd += (float)(err * err); 
		}
	} else {
		FOR (i, _nPixel) {
			err = (int)(*((uchar*)(_img.data+_pVec[i]*elemSize)))
				- p[0]; 
			ssd += (float)(err * err); 
			err = (int)(*((uchar*)(_img.data+_pVec[i]*elemSize+1)))
				- p[1]; 
			ssd += (float)(err * err); 
			err = (int)(*((uchar*)(_img.data+_pVec[i]*elemSize+2)))
				- p[2]; 
			ssd += (float)(err * err); 
		}
	}

	return ssd; 
}

float FAST::CFastImage::FastSSD( Mat _img1, Mat _img2, const int _pVec1[], const int _pVec2[], int _nPixels ) {
	float ssd = 0; 
	int err = 0; 
	size_t elemSize = _img1.elemSize(); 
	if (_img1.channels() == 1) {
		for (int i = 0; i < _nPixels; i++) {
			err = (int)(*((uchar*)(_img1.data+_pVec1[i]*elemSize)))
				- (int)(*((uchar*)(_img2.data+_pVec2[i]*elemSize))); 
			ssd += (float)(err * err); 
		}
	} else {
		for (int i = 0; i < _nPixels; i++) {
			err = (int)(*((uchar*)(_img1.data+_pVec1[i]*elemSize)))
				- (int)(*((uchar*)(_img2.data+_pVec2[i]*elemSize))); 
			ssd += (float)(err * err); 
			err = (int)(*((uchar*)(_img1.data+_pVec1[i]*elemSize+1)))
				- (int)(*((uchar*)(_img2.data+_pVec2[i]*elemSize+1))); 
			ssd += (float)(err * err); 
			err = (int)(*((uchar*)(_img1.data+_pVec1[i]*elemSize+2)))
				- (int)(*((uchar*)(_img2.data+_pVec2[i]*elemSize+2))); 
			ssd += (float)(err * err); 
		}
	}

	//SHOW(ssd); 
	return ssd; 
}

float FAST::CFastImage::FastConvolution( Mat _img, const int _pVec[], int _nPixels) {
	unsigned long conv = 0; 
	int thres = 128; 
	size_t elemSize = _img.elemSize(); 

	FOR (i, _nPixels) 
		conv += min(thres, (int)(*((uchar*)(_img.data+_pVec[i]*elemSize)))); 

	return (float)conv; 
}

float FAST::CFastImage::FastConvolution( Mat _img1, Mat _img2, const int _pVec[], int _nPixels ) {
	float conv = 0; 
	//int tmp = 0; 
	size_t elemSize = _img1.elemSize(); 
	for (int i = 0; i < _nPixels; i++) {
			conv += (float)(*((uchar*)(_img1.data+_pVec[i]*elemSize)))
				* (float)(*((uchar*)(_img2.data+_pVec[i]*elemSize))); 
	}

	return conv; 
}

void FAST::CFastImage::NormalizePatch( Mat& _img ) {
	int dataSz = _img.rows * _img.cols; 
	size_t elemSize = _img.elemSize(); 
	uchar max_uchar = 0; 
	FOR (i, dataSz)
		if (*((uchar*)(_img.data+i*elemSize)) > max_uchar)
			max_uchar = *((uchar*)(_img.data+i*elemSize)); 
	if (max_uchar == 0)
		return; 
	int max_int =  (int)max_uchar; 
	#pragma omp parallel for 
	FOR (i, dataSz)
		*((uchar*)(_img.data+i*elemSize)) 
		= (uchar)(((int)(*((uchar*)(_img.data+i*elemSize))) * 255) / max_int); 
}

void FAST::CFastImage::MagnifyPatch( Mat& _img, int _factor ) {
	int dataSz = _img.rows * _img.cols; 
	size_t elemSize = _img.elemSize(); 

	#pragma omp parallel for 
	FOR (i, dataSz)
		*((uchar*)(_img.data+i*elemSize)) 
		= (uchar)min((int)(*((uchar*)(_img.data+i*elemSize))) * _factor, 255); 
}

void FAST::CFastImage::ShowImages( MatSet _imgVec, string _name, int _hGrid, int _wGrid, int _scale) {
	if (_imgVec.empty()) 
		return; 
	Size imgSz = _imgVec[0].size(); 
	int border = 8; 
	int small_h = roundInt(imgSz.height/_scale); 
	int small_w = roundInt(imgSz.width/_scale); 
	int height = _hGrid * small_h + (_hGrid-1) * border; 
	int width =  _wGrid * small_w + (_wGrid-1) * border; 
	int nChannels = _imgVec[0].channels(); 

	Mat showImg  = nChannels == 1 ? Mat(Size(width, height), _imgVec[0].type(), 255)
		:  Mat(Size(width, height), _imgVec[0].type(), Scalar(255, 255, 255)); 

	#pragma omp parallel for 
	FOR (h, _hGrid) 
		FOR (w, _wGrid) {		
			Mat img = _imgVec[h*_wGrid+w]; 
			Mat tmp; 

			if (_scale == 1)
				tmp = img; 
			else
				cv::resize(img, tmp, Size(small_w, small_h)); 

			tmp.copyTo(showImg(Rect(w*(small_w+border), h*(small_h+border), 
				small_w, small_h))); 
	}

	imshow("SHOW IMAGES" + _name, showImg); 
	//waitKey(); 
}

void FAST::CFastImage::FastErrorImage( Mat _img1, Mat _img2, Mat& _out, const int _pVec[], int _nPixel ) {
	size_t elemSize = _img1.elemSize(); 
	size_t floatSize = _out.elemSize(); 

	if (_img1.channels() == 1) {
		FOR (i, _nPixel) {
			*((float*)(_out.data+_pVec[i]*floatSize)) += abs((float)((int)(*((uchar*)(_img1.data+_pVec[i]*elemSize)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize))))); 
		}
	} else {
		FOR (i, _nPixel) {
			int ssd = 0; 
			ssd += abs((int)(*((uchar*)(_img1.data+_pVec[i]*elemSize)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize)))); 		
			ssd += abs((int)(*((uchar*)(_img1.data+_pVec[i]*elemSize+1)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize+1)))); 
			ssd += abs((int)(*((uchar*)(_img1.data+_pVec[i]*elemSize+2)))
				- (int)(*((uchar*)(_img2.data+_pVec[i]*elemSize+2)))); 
			*((float*)(_out.data+_pVec[i]*floatSize)) += (float)ssd/3.0f; 
		}
	}
}

void FAST::CFastImage::ThresholdZeroError( vector<float>& _errVec ) {
	float min_error = 1e15f; 
	int min_idx = -1;
	vector<int> zeroIndex; 
	zeroIndex.clear(); 
	FOR (i, (int)_errVec.size()) {
		if (_errVec[i] < 1e-20f)
			zeroIndex.push_back(i); 
		else if (_errVec[i] < min_error) {
			min_error = _errVec[i]; 
			min_idx = i; 
		}
	}

	FOR (i, (int)zeroIndex.size())
		_errVec[zeroIndex[i]] = min_error / 2; 
}

void FAST::CFastImage::UpdateMask(Size _imgSz, const int _pVec[], int _nPixel, int _offset_x, 
	int _offset_y, int _outPVec[], int& _outPixel) {
		int width = _imgSz.width; 
		int height = _imgSz.height; 
		int dataSz = width * height; 
		_outPixel = 0; 
		FOR (i, _nPixel) {
			int tmp = _pVec[i] + _offset_x + _offset_y * width;
			if (tmp < dataSz && tmp >= 0) {
				_outPVec[_outPixel] = tmp; 
				_outPixel++; 
			}
		}
}

cv::Mat FAST::CFastImage::FastAddImages( const MatSet& _imgs ) {
	int nImgs = (int)_imgs.size(); 
	int middle1 = roundInt(nImgs/2); 
	Mat outImg, outImg2; 
	
	#pragma omp parallel sections 
	{
		#pragma omp section 
		{
			outImg = Mat::zeros(_imgs[0].size(), _imgs[0].type());
			FOR (i, middle1)
				cv::add(outImg, _imgs[i], outImg); 
		}
		#pragma omp section 
		{
			outImg2 = Mat::zeros(_imgs[0].size(), _imgs[0].type());
			for (int i = middle1; i < nImgs; i++)
				cv::add(outImg2, _imgs[i], outImg2); 
		}
	}

	add(outImg, outImg2, outImg); 
	return outImg; 
}

float FAST::CFastImage::FastSum( Mat& _img, int _pVec[], int _n ) {
	size_t elemSize = _img.elemSize(); 
	float sum = 0.0f; 
	FOR (i, _n)
		sum += (float)(*(uchar*)(_img.data+_pVec[i]*elemSize)); 
	return sum; 
}
