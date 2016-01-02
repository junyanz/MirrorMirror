#include "FeatureModule.h"
#include "HOGDescriptor.h"
//#include "ImageTools.h"
#include "ImageTools.h"
#include "Timer.h"

void CFeatureModule::Clear() {
	//m_useHOG = false; 
	//m_useLandmarks = false; 
	m_ndims = 0; 
}

CFeatureModule::CFeatureModule() {
	Clear(); 
}

CFeatureModule::~CFeatureModule() {
	Clear(); 
}

//float* CFeatureModule::GetFeature(const Mat& _img, PointSetd _landmarks, bool _isVis) {
//
//	Mat disp;
//	int  border = 16; 
//	Mat img_e = ImageTools::CImageTools::ExtendImage(_img, border);
//	if (_isVis)
//		disp = img_e.clone(); 
//	//SHOW_IMG(img_e);
//	const int nLandmarks = 66; 
//	FOR (i, (int)_landmarks.size())
//		_landmarks[i] = Point2d(_landmarks[i].x+border, _landmarks[i].y+border);
//	int idx_len[3] = {6, 6, 18}; 
//	//int border[3] = {16, 16, 12}; 
//	int left_eye_idx[] = {36, 37, 38, 39, 40, 41};
//	int right_eye_idx[] = {42, 43, 44, 45, 46, 47};
//	int mouth_idx[] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65};
//	vector<int*> idx_vec; 
//	vector<Size> size_vec; 
//	vector<Size> rect_vec; 
//	Size eye_size(64, 48); 
//	Size mouth_size(80, 40);
//	Size eye_rect(48, 32); 
//	Size mouth_rect(80, 40);
//	idx_vec.push_back(left_eye_idx); 
//	idx_vec.push_back(right_eye_idx); 
//	idx_vec.push_back(mouth_idx); 
//	size_vec.push_back(eye_size); 
//	size_vec.push_back(eye_size); 
//	size_vec.push_back(mouth_size); 
//	rect_vec.push_back(eye_rect);
//	rect_vec.push_back(eye_rect);
//	rect_vec.push_back(mouth_rect);
//	vector<float*> hogs; 
//	vector<int> ndims; 
//
//	FOR (i, 3) {
//		Rect rect = LocalizeParts(_landmarks, idx_vec[i], idx_len[i], rect_vec[i]);
//		if (!ImageTools::CImageTools::IsValidROI(rect, img_e.size()))
//			return NULL;
//		if (_isVis)
//			rectangle(disp, rect, Scalar(0, 255, 0), 2); 
//
//		Mat patch; 
//		resize(img_e(rect), patch, size_vec[i]); 
//		CHOGDescriptor* hog = new CHOGDescriptor(); 
//		hog->SetDims(patch.size()); 
//		float* f = NULL; 
//		int n = hog->FeatureLength(); 
//		hog->ComputeFeature(patch, f); 
//		ndims.push_back(n); 
//		hogs.push_back(f); 
//		DELETE_OBJECT(hog); 
//	}
//
//	// compute face
//	Mat faceIm; 
//	resize(_img, faceIm, Size(64, 64));
//	//SHOW_IMG(faceIm);
//	float* faceF = NULL;
//	CHOGDescriptor* hog = new CHOGDescriptor(); 
//	hog->SetDims(faceIm.size()); 
//	int n = hog->FeatureLength(); 
//	hog->ComputeFeature(faceIm, faceF); 
//	ndims.push_back(n); 
//	hogs.push_back(faceF); 
//	DELETE_OBJECT(hog); 
//
//	// add landmarks
//	m_ndims = vecSum(ndims);
//	int hogDims = m_ndims;
//
//	if (m_useLandmarks)
//		m_ndims += (int)_landmarks.size()*2;
//	float* feat = new float[m_ndims]; 
//	int idx = 0; 
//	FOR (i, (int)ndims.size()) {
//		FOR (j, ndims[i])
//			feat[j+idx] = hogs[i][j]; 
//		idx += ndims[i]; 
//	}
//
//	if (m_useLandmarks) {
//		FOR_u (i, _landmarks.size()) {
//			feat[hogDims+2*i] = (float)_landmarks[i].x;
//			feat[hogDims+2*i+1] = (float)_landmarks[i].y;
//		}
//	}
//
//	if (_isVis) {
//		Mat disp_crop; 
//		disp(Rect(border, border, _img.cols, _img.rows)).copyTo(disp_crop);
//		imshow("landmark", disp_crop);
//		waitKey();
//	}
//	rect_vec.clear(); 
//	size_vec.clear(); 
//	//cout << "feat ndim << endl; 
//	return feat; 
//}

float* CFeatureModule::GetFeature( const Mat& _img, bool _isVis /*= false*/ ) {
		//CTimer timer("get feature");
	Mat disp; 
	if (_isVis)
		disp = _img.clone(); 
	//Rect face_rect();
	//float width = disp.cols; 
	//float height = disp.rows; 
	Rect face_rect(25,20,120,170);
	Rect mouth_rect(28, 143, 114, 44);
	Rect left_eye_rect(28, 60, 44, 45);
	Rect right_eye_rect(98, 60, 44, 45);
	Rect wrinkle_rect(50, 45, 75, 30);

	Size mouth_size(64, 32);
	Size eye_size(64, 48); 
	Size face_size(64, 80);
	Size wrinkle_size(64, 32);

	vector<Rect> rects; 
	vector<Size> sizes; 
	rects.push_back(face_rect);
	rects.push_back(mouth_rect);
	rects.push_back(left_eye_rect);
	rects.push_back(right_eye_rect);
	rects.push_back(wrinkle_rect);
	sizes.push_back(face_size);
	sizes.push_back(mouth_size);
	sizes.push_back(eye_size);
	sizes.push_back(eye_size);
	sizes.push_back(wrinkle_size);
	//if (_isVis) {
	//	FOR_u (i, rects.size()) 
	//		rectangle(disp, rects[i], Scalar(0, 255, 0), 2);
	//}

	vector<float*> hogs(rects.size()); 
	vectori ndims(rects.size()); 
	#pragma omp parallel for 
	FOR (i, (int)rects.size())  {
		//Rect rect = LocalizeParts(_landmarks, idx_vec[i], idx_len[i], rect_vec[i]);
		//if (!ImageTools::CImageTools::IsValidROI(rect, img_e.size()))
			//return NULL;
		if (_isVis)
			rectangle(disp, rects[i], Scalar(0, 255, 0), 2);
		Mat patch; 
		resize(_img(rects[i]), patch, sizes[i]); 
		CHOGDescriptor* hog = new CHOGDescriptor(); 
		hog->SetDims(patch.size()); 
		float* f = NULL; 
		int n = hog->FeatureLength(); 
		hog->ComputeFeature(patch, f); 
		ndims[i] = n; 
		hogs[i] = f; 
		DELETE_OBJECT(hog); 
	}


	m_ndims = vecSum(ndims);
	float* f = new float[m_ndims]; 
	int idx = 0; 
	FOR (i, (int)ndims.size()) {
		FOR (j, ndims[i])
			f[j+idx] = hogs[i][j]; 
		idx += ndims[i]; 
	}

	return f; 
}

Rect CFeatureModule::LocalizeParts( PointSetd _pnts, int _idx[], int _n, Size _sz) {
	int min_x = 10000; 
	int min_y = 10000; 
	int max_x = 0; 
	int max_y = 0; 

	FOR (j, _n) {
		int i = _idx[j]; 
		min_x = MIN(min_x, (int)_pnts[i].x); 
		min_y = MIN(min_y, (int)_pnts[i].y); 
		max_x = MAX(max_x, (int)_pnts[i].x);
		max_y = MAX(max_y, (int)_pnts[i].y); 
	}

	int center_x = roundInt((min_x+max_x)/2);
	int center_y = roundInt((min_y+max_y)/2);
	  
	// add border
	/*min_x = min_x - _border; 
	min_y = min_y - _border; 
	max_x = max_x + _border; 
	max_y = max_y + _border;*/
	//cout << min_x << " " << min_y << " " << max_x << " " << max_y << endl; 
	return Rect(center_x-_sz.width, center_y - _sz.height, _sz.width*2, _sz.height*2); 
}


