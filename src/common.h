#ifndef COMMON_H_
#define COMMON_H_

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

//include std
#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cassert>
#include <omp.h>
#include <cstdio>
#include <iomanip>
#include <time.h>
#include <cstdlib>
#include <stdio.h>
#include <math.h>
#include <set>

//logging
#include "log.hpp"
//include OpenCV 
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"

using namespace std;
using namespace cv;

#define DELETE_OBJECT(x)	if ((x)) { delete (x); (x) = NULL; }
#define DELETE_ARRAY(x)		if ((x)) { delete [] (x); (x) = NULL; }
#define DELETE_OBJECT_ARRAY(n,x)		if ((x)) { for (int i = 0; i < n; i++) { DELETE_OBJECT(((x)[i])); } delete [] (x); (x) = NULL; }
#define FOR(i,n) for( int i=0; i<n; i++ )
#define FOR_u(i, n) for (unsigned i = 0; i < n; i++)
#define PRINT(a) std::cout << #a << ": " << (a) << std::endl
#define SQUARE(x) ((x)*(x))

#define SHOW_IMG(img){\
	imshow(#img, img); \
	waitKey(); \
}

#define PI				   3.141592653589793238512808959406186204433
#define NUM_POINTS 9

// typedefs
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef vector<int> vectori;
typedef vector<float> vectorf;
typedef vector<double> vectord;
typedef vector<bool> vectorb; 
typedef vector<uchar> vectoru;
typedef vector<ushort> vectorus;
typedef vector<string> vectorString;
typedef vector<Point2d> PointSetd;
typedef vector<Point2i> PointSeti;
typedef vector<Mat> MatSet; 
typedef vector<Mat> ImageSet; 
typedef const Mat CMat;
typedef const string CStr;

template<class T> int roundInt(T val) {
    return floor(val + 0.5);
}


template<class T> T vecSum(const vector<T> &v) {
    T sum = (T) 0.0;
    for (size_t i = 0; i < v.size(); i++)
        sum += v[i];
    return sum;
}

template<class T> vector<T> vecDiv(const vector<T> &v, T val) {
    vector<T> r(v);
    for (size_t i = 0; i < v.size(); i++)
        r[i] /= val;
    return r;
}

template<class T> T vecMax(const vector<T> &v) {
    return *max_element(v.begin(), v.end());
}

template<class T> T vecMin(const vector<T> &v) {
    return *min_element(v.begin(), v.end());
}


template<class T> class SortableElement {
public:
	T _val;
	int _ind;
	SortableElement() {
	}
	;
	SortableElement(T val, int ind) {
		_val = val;
		_ind = ind;
	}
	bool operator<(const SortableElement &b) const {
		return (_val < b._val);
	}
	;
};

template<class T> void sortOrder(vector<T> &v, vectori &order) {
	int n = v.size();
	vector<SortableElement<T> > v2;
	v2.resize(n);
	order.clear();
	order.resize(n);
	for (int i = 0; i < n; i++) {
		v2[i]._ind = i;
		v2[i]._val = v[i];
	}
	std::sort(v2.begin(), v2.end());
	for (int i = 0; i < n; i++) {
		order[i] = v2[i]._ind;
		v[i] = v2[i]._val;
	}
}






#endif //COMMON_H_
