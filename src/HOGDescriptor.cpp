#include "HOGDescriptor.h"

#include "Timer.h"
// small value, used to avoid division by zero
#define EPS 0.0001f
#define MIN_THRES 0.20f
#define TEXTURE_CONSTANT 0.2357f
// unit vectors used to compute gradient orientation
float uu[9] = { 1.0000f, 0.9397f, 0.7660f, 0.5000f, 0.1736f, -0.1736f, -0.5000f,
		-0.7660f, -0.9397f };
float vv[9] = { 0.0000f, 0.3420f, 0.6428f, 0.8660f, 0.9848f, 0.9848f, 0.8660f, 0.6428f,
		0.3420f };

static inline int min(int x, int y) {
	return (x <= y ? x : y);
}
static inline int max(int x, int y) {
	return (x <= y ? y : x);
}

CHOGDescriptor::CHOGDescriptor() {
	Clear();
}

void CHOGDescriptor::Clear() {
	m_lenFtr = 0;
	m_cellSize = 8;
	m_hogDimension = 31; 
	m_imageHeight = 0; 
	m_imageWidth = 0; 
	m_hogHeight = 0; 
	m_hogWidth = 0; 
}

CHOGDescriptor::~CHOGDescriptor() {
	Clear(); 
}

void CHOGDescriptor::ComputeFeatureLength() {
	// hack
	//m_hogWidth = (m_imageWidth + m_cellSize / 2) / m_cellSize; 
	//m_hogHeight = (m_imageHeight + m_cellSize / 2) / m_cellSize; 
	//m_lenFtr = m_hogHeight * m_hogWidth * m_hogDimension; 
	m_dims[0] = m_imageHeight; 
	m_dims[1] = m_imageWidth; 
	m_blocks[0] = (int)roundInt((double) m_dims[0] / (double) m_cellSize);
	m_blocks[1] = (int)roundInt((double) m_dims[1] / (double) m_cellSize);
	m_out[0] = max(m_blocks[0] - 2, 0);
	m_out[1] = max(m_blocks[1] - 2, 0);
	m_out[2] = 31;
	m_hogHeight = m_out[0]; 
	m_hogWidth = m_out[1]; 
	m_lenFtr = m_out[0] * m_out[1] * m_out[2];
}

void CHOGDescriptor::ComputeFeature(Mat _img, float*& _feats) {
	Mat tmpIm; 
	Mat floatIm; 
	Size imgSz = _img.size(); 
	if (imgSz.height == m_imageHeight && imgSz.width == m_imageWidth) 
		tmpIm = _img; 
	else 
		resize(_img, tmpIm, Size(m_imageWidth, m_imageHeight)); 
	
	tmpIm.convertTo(floatIm, CV_32FC3);
	_feats = CV_GetHOG(floatIm); 
}

int CHOGDescriptor::FeatureLength() {
	ComputeFeatureLength();
	return m_lenFtr;
}

float* CHOGDescriptor::GetHOG(Mat _img) {
	// memory for caching orientation histograms & their norms
	float* hist = new float[m_blocks[0] * m_blocks[1] * 18];
	std::fill_n(hist, m_blocks[0] * m_blocks[1] * 18, 0.0f); 
	float* norm = new float[m_blocks[0] * m_blocks[1]];
	std::fill_n(norm, m_blocks[0] * m_blocks[1], 0.0f); 
	// memory for HOG features
	float* feat = new float[m_lenFtr];
	std::fill_n(feat, m_lenFtr, 0.0f); 

	int visible[2];
	visible[0] = m_blocks[0] * m_cellSize;
	visible[1] = m_blocks[1] * m_cellSize;

	for (int x = 1; x < visible[1] - 1; x++) {
		for (int y = 1; y < visible[0] - 1; y++) {
			// compute derivatives
			float v3[3], dx3[3], dy3[3];
			float v, dx, dy;
			FOR (c, 3) {
				int yy = min(y, m_dims[0] - 2);
				int xx = min(x, m_dims[1] - 2);
				dy3[c] = _img.at<Vec3f>(yy + 1, xx)[c] - _img.at<Vec3f>(yy - 1, xx)[c];
				dx3[c] = _img.at<Vec3f>(yy, xx + 1)[c] - _img.at<Vec3f>(yy, xx - 1)[c];
				v3[c] = dx3[c] * dx3[c] + dy3[c] * dy3[c];
			}
			
			v = v3[0];
			dx = dx3[0];
			dy = dy3[0];
			// pick channel with strongest gradient
			if (v3[2] > v) {
				v = v3[2];
				dx = dx3[2];
				dy = dy3[2];
			}
			if (v3[1] > v) {
				v = v3[1];
				dx = dx3[1];
				dy = dy3[1];
			}

			// snap to one of 18 orientations
			float best_dot = 0;
			int best_o = 0;
			for (int o = 0; o < 9; o++) {
				float dot = uu[o] * dx + vv[o] * dy;
				if (dot > best_dot) {
					best_dot = dot;
					best_o = o;
				} else if (-dot > best_dot) {
					best_dot = -dot;
					best_o = o + 9;
				}
			}

			// add to 4 histograms around pixel using linear interpolation
			float xp = ((float) x + 0.5f) / (float) m_cellSize - 0.5f;
			float yp = ((float) y + 0.5f) / (float) m_cellSize - 0.5f;
			int ixp = (int) floor(xp);
			int iyp = (int) floor(yp);
			float vx0 = xp - ixp;
			float vy0 = yp - iyp;
			float vx1 = 1.0f - vx0;
			float vy1 = 1.0f - vy0;
			v = sqrt(v);

			if (ixp >= 0 && iyp >= 0)
				*(hist + ixp * m_blocks[0] + iyp + best_o * m_blocks[0] * m_blocks[1]) += vx1 * vy1 * v;

			if (ixp + 1 < m_blocks[1] && iyp >= 0)
				*(hist + (ixp + 1) * m_blocks[0] + iyp + best_o * m_blocks[0] * m_blocks[1]) += vx0 * vy1 * v;

			if (ixp >= 0 && iyp + 1 < m_blocks[0])
				*(hist + ixp * m_blocks[0] + (iyp + 1) + best_o * m_blocks[0] * m_blocks[1]) += vx1 * vy0 * v;

			if (ixp + 1 < m_blocks[1] && iyp + 1 < m_blocks[0])
				*(hist + (ixp + 1) * m_blocks[0] + (iyp + 1) + best_o * m_blocks[0] * m_blocks[1]) += vx0 * vy0 * v;
		}
	}

	// compute energy in each block by summing over orientations
	for (int o = 0; o < 9; o++) {
		float *src1 = hist + o * m_blocks[0] * m_blocks[1];
		float *src2 = hist + (o + 9) * m_blocks[0] * m_blocks[1];
		float *dst = norm;
		float *end = norm + m_blocks[1] * m_blocks[0];
		while (dst < end) {
			*(dst++) += (*src1 + *src2) * (*src1 + *src2);
			src1++;
			src2++;
		}
	}

	// compute features
	for (int x = 0; x < m_out[1]; x++) {
		for (int y = 0; y < m_out[0]; y++) {
			float *dst = feat + x * m_out[0] + y;
			float *src, *p, n1, n2, n3, n4;

			p = norm + (x + 1) * m_blocks[0] + y + 1;
			n1 = 1.0f / sqrt(*p + *(p + 1) + *(p + m_blocks[0]) + *(p + m_blocks[0] + 1) + EPS);
			p = norm + (x + 1) * m_blocks[0] + y;
			n2 = 1.0f / sqrt(*p + *(p + 1) + *(p + m_blocks[0]) + *(p + m_blocks[0] + 1) + EPS);
			p = norm + x * m_blocks[0] + y + 1;
			n3 = 1.0f / sqrt(*p + *(p + 1) + *(p + m_blocks[0]) + *(p + m_blocks[0] + 1) + EPS);
			p = norm + x * m_blocks[0] + y;
			n4 = 1.0f / sqrt(*p + *(p + 1) + *(p + m_blocks[0]) + *(p + m_blocks[0] + 1) + EPS);

			float t1 = 0.0f;
			float t2 = 0.0f;
			float t3 = 0.0f;
			float t4 = 0.0f;

			//float bh_test=0;
			// contrast-sensitive features
			src = hist + (x+1)*m_blocks[0] + (y+1);
			for (int o = 0; o < 18; o++) {
				float h1 = min(*src * n1, MIN_THRES);
				float h2 = min(*src * n2, MIN_THRES);
				float h3 = min(*src * n3, MIN_THRES);
				float h4 = min(*src * n4, MIN_THRES);
				*dst = 0.5f * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;
				dst += m_out[0]*m_out[1];
				src += m_blocks[0]*m_blocks[1];
			}

			// contrast-insensitive features
			src = hist + (x + 1) * m_blocks[0] + (y + 1);
			for (int o = 0; o < 9; o++) {
				float sum = *src + *(src + 9 * m_blocks[0] * m_blocks[1]);
				float h1 = min(sum * n1, MIN_THRES);
				float h2 = min(sum * n2, MIN_THRES);
				float h3 = min(sum * n3, MIN_THRES);
				float h4 = min(sum * n4, MIN_THRES);
				*dst = 0.5f * (h1 + h2 + h3 + h4);
				dst += m_out[0] * m_out[1];
				src += m_blocks[0] * m_blocks[1];
			}

			// texture features
			*dst = TEXTURE_CONSTANT * t1;
			dst += m_out[0] * m_out[1];
			*dst = TEXTURE_CONSTANT * t2;
			dst += m_out[0] * m_out[1];
			*dst = TEXTURE_CONSTANT * t3;
			dst += m_out[0] * m_out[1];
			*dst = TEXTURE_CONSTANT * t4;

			/*dst += m_out[0]*m_out[1];
			*dst = 0;*/
		}
	}


	DELETE_ARRAY(hist); 
	DELETE_ARRAY(norm); 
	return feat;
}

void CHOGDescriptor::SetDims( Size _imgSz, int _cellSize) {
	m_cellSize = _cellSize; 
	m_imageHeight = _imgSz.height; 
	m_imageWidth = _imgSz.width; 
	ComputeFeatureLength(); 
}

float* CHOGDescriptor::GetHOGGray( Mat _img ) {
	// memory for caching orientation histograms & their norms
	float* hist = new float[m_blocks[0] * m_blocks[1] * 18];
	std::fill_n(hist, m_blocks[0] * m_blocks[1] * 18, 0.0f); 
	float* norm = new float[m_blocks[0] * m_blocks[1]];
	std::fill_n(norm, m_blocks[0] * m_blocks[1], 0.0f); 
	// memory for HOG features
	float* feat = new float[m_lenFtr];
	std::fill_n(feat, m_lenFtr, 0.0f); 

	int visible[2];
	visible[0] = m_blocks[0] * m_cellSize;
	visible[1] = m_blocks[1] * m_cellSize;

	for (int x = 1; x < visible[1] - 1; x++) {
		for (int y = 1; y < visible[0] - 1; y++) {
			// compute derivatives
			/*float v3[3], dx3[3], dy3[3];*/
			float v, dx, dy;
			int yy = min(y, m_dims[0] - 2);
			int xx = min(x, m_dims[1] - 2);
			dy = _img.at<float>(yy + 1, xx) - _img.at<float>(yy - 1, xx);
			dx = _img.at<float>(yy, xx + 1) - _img.at<float>(yy, xx - 1);
			v = dx * dx + dy * dy;
			
			// snap to one of 18 orientations
			float best_dot = 0;
			int best_o = 0;
			for (int o = 0; o < 9; o++) {
				float dot = uu[o] * dx + vv[o] * dy;
				if (dot > best_dot) {
					best_dot = dot;
					best_o = o;
				} else if (-dot > best_dot) {
					best_dot = -dot;
					best_o = o + 9;
				}
			}

			// add to 4 histograms around pixel using linear interpolation
			float xp = ((float) x + 0.5f) / (float) m_cellSize - 0.5f;
			float yp = ((float) y + 0.5f) / (float) m_cellSize - 0.5f;
			int ixp = (int) floor(xp);
			int iyp = (int) floor(yp);
			float vx0 = xp - ixp;
			float vy0 = yp - iyp;
			float vx1 = 1.0f - vx0;
			float vy1 = 1.0f - vy0;
			v = sqrt(v);

			if (ixp >= 0 && iyp >= 0)
				*(hist + ixp * m_blocks[0] + iyp + best_o * m_blocks[0] * m_blocks[1]) += vx1 * vy1 * v;

			if (ixp + 1 < m_blocks[1] && iyp >= 0)
				*(hist + (ixp + 1) * m_blocks[0] + iyp + best_o * m_blocks[0] * m_blocks[1]) += vx0 * vy1 * v;

			if (ixp >= 0 && iyp + 1 < m_blocks[0])
				*(hist + ixp * m_blocks[0] + (iyp + 1) + best_o * m_blocks[0] * m_blocks[1]) += vx1 * vy0 * v;

			if (ixp + 1 < m_blocks[1] && iyp + 1 < m_blocks[0])
				*(hist + (ixp + 1) * m_blocks[0] + (iyp + 1) + best_o * m_blocks[0] * m_blocks[1]) += vx0 * vy0 * v;
		}
	}

	// compute energy in each block by summing over orientations
	for (int o = 0; o < 9; o++) {
		float *src1 = hist + o * m_blocks[0] * m_blocks[1];
		float *src2 = hist + (o + 9) * m_blocks[0] * m_blocks[1];
		float *dst = norm;
		float *end = norm + m_blocks[1] * m_blocks[0];
		while (dst < end) {
			*(dst++) += (*src1 + *src2) * (*src1 + *src2);
			src1++;
			src2++;
		}
	}

	// compute features
	for (int x = 0; x < m_out[1]; x++) {
		for (int y = 0; y < m_out[0]; y++) {
			float *dst = feat + x * m_out[0] + y;
			float *src, *p, n1, n2, n3, n4;

			p = norm + (x + 1) * m_blocks[0] + y + 1;
			n1 = 1.0f / sqrt(*p + *(p + 1) + *(p + m_blocks[0]) + *(p + m_blocks[0] + 1) + EPS);
			p = norm + (x + 1) * m_blocks[0] + y;
			n2 = 1.0f / sqrt(*p + *(p + 1) + *(p + m_blocks[0]) + *(p + m_blocks[0] + 1) + EPS);
			p = norm + x * m_blocks[0] + y + 1;
			n3 = 1.0f / sqrt(*p + *(p + 1) + *(p + m_blocks[0]) + *(p + m_blocks[0] + 1) + EPS);
			p = norm + x * m_blocks[0] + y;
			n4 = 1.0f / sqrt(*p + *(p + 1) + *(p + m_blocks[0]) + *(p + m_blocks[0] + 1) + EPS);

			float t1 = 0.0f;
			float t2 = 0.0f;
			float t3 = 0.0f;
			float t4 = 0.0f;

			//float bh_test=0;
			// contrast-sensitive features
			src = hist + (x+1)*m_blocks[0] + (y+1);
			for (int o = 0; o < 18; o++) {
				//bh_test=bh_test+(*src * n1)+(*src * n2)+(*src * n3)+(*src * n4);
				float h1 = min(*src * n1, MIN_THRES);
				float h2 = min(*src * n2, MIN_THRES);
				float h3 = min(*src * n3, MIN_THRES);
				float h4 = min(*src * n4, MIN_THRES);
				*dst = 0.5f * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;
				dst += m_out[0]*m_out[1];
				src += m_blocks[0]*m_blocks[1];
			}

			// contrast-insensitive features
			src = hist + (x + 1) * m_blocks[0] + (y + 1);
			for (int o = 0; o < 9; o++) {
				float sum = *src + *(src + 9 * m_blocks[0] * m_blocks[1]);
				float h1 = min(sum * n1, MIN_THRES);
				float h2 = min(sum * n2, MIN_THRES);
				float h3 = min(sum * n3, MIN_THRES);
				float h4 = min(sum * n4, MIN_THRES);
				*dst = 0.5f * (h1 + h2 + h3 + h4);
				dst += m_out[0] * m_out[1];
				src += m_blocks[0] * m_blocks[1];
			}

			// texture features
			*dst = TEXTURE_CONSTANT * t1;
			dst += m_out[0] * m_out[1];
			*dst = TEXTURE_CONSTANT * t2;
			dst += m_out[0] * m_out[1];
			*dst = TEXTURE_CONSTANT * t3;
			dst += m_out[0] * m_out[1];
			*dst = TEXTURE_CONSTANT * t4;

			/*dst += m_out[0]*m_out[1];
			*dst = 0;*/
		}
	}


	DELETE_ARRAY(hist); 
	DELETE_ARRAY(norm); 
	return feat;
}


Mat rotateImage(const Mat& src) {
	Size imgSz = src.size(); 
	Mat dst = Mat(Size(imgSz.height, imgSz.width), CV_32FC1); 
	FOR (h, imgSz.height)
		FOR (w, imgSz.width)
			dst.at<float>(w, h) = src.at<float>(h, w); 
	return dst; 
}


float* CHOGDescriptor::CV_GetHOG( Mat _img ) {
	if (_img.channels() == 1)
		return GetHOGGray(_img); 
	else
		return GetHOG(_img); 
}

void CHOGDescriptor::TranslateHOG( const float* _o, float*& _out, Point2i _offset ) {
	//DEBUG_INFO("feature length %d", m_lenFtr); 
	_out = new float[m_lenFtr]; 
	std::fill_n(_out, m_lenFtr, 0.0f); 
	int dx = _offset.x / m_cellSize; 
	int dy = _offset.y / m_cellSize; 
	//SHOW_ARRAY(_o, 50); 
	//cout << m_lenFtr << endl; 
	FOR (i, m_out[2]) {
		int base = i * m_out[0] * m_out[1]; 
		FOR (x, m_out[1]) {
			FOR (y, m_out[0]) {
				int idx = base + x * m_out[0] + y;
				//cout << idx << " "; 
				int map_idx = base + (x-dx) * m_out[0] + (y-dy);
				if (map_idx >= 0 && map_idx < m_lenFtr)
					_out[map_idx] = _o[idx]; 
			}
		}
	}

	//cout << endl; 
	//SHOW_ARRAY(_out, 50); 
}

void CHOGDescriptor::NormalizeHOG( float* _f ) {
	float mean_f = 0.0f; 
	int count = 0;

	FOR (i, m_lenFtr) {
		if (abs(_f[i]) > 1e-10) {
			mean_f += _f[i]; 
			count++; 
		}
	}

	mean_f /= count; 
	#pragma omp parallel for
	FOR (i, m_lenFtr)
		if (abs(_f[i]) > 1e-10)
			_f[i] -= mean_f; 

}

void CHOGDescriptor::MinusHOG( float* _f ) {
	//int nCells = m_out[0] * m_out[1]; 
	#pragma omp parallel for
	FOR (i, m_lenFtr)
		if (abs(_f[i]) > 1e-10f)
			_f[i] = -0.05f; 
}

void CHOGDescriptor::CropHOG( Rect _roi, int*& _cropIdx, int& _n ) {

}

void CHOGDescriptor::CropHOG( const Mat& _mask, int*& _cropIdx, int& _n ) {

}

float CHOGDescriptor::HOGProduction( float* _f, float* _g, Point2i _offset, int* _idx, int _numIdx, bool _isVis) {
	float s = 0.0f; 
	int dx = roundInt(_offset.x/m_cellSize); 
	int dy = roundInt(_offset.y/m_cellSize);
	int mout = m_out[0]; 
	if (dx == 0 && dy == 0)
		FOR (i, _numIdx) 
			s += _f[_idx[i]] * _g[_idx[i]]; 
	else {
		FOR (i, _numIdx) {
			int map_idx = _idx[i] - dx * mout-dy;
			if (map_idx >= 0 && map_idx < m_lenFtr)
				s += _f[_idx[i]] * _g[map_idx]; 
		}
	}


	return s; 
}
