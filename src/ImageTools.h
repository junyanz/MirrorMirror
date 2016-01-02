#ifndef IMAGE_TOOLS_H_
#define IMAGE_TOOLS_H_
#include "common.h"

namespace ImageTools {
	class CImageTools {
	public: 
		// for debug
		static Mat RandomImage(const Mat& _img); 
		// roi
		static Rect GetROI(const vector<Point2i>& _points); 
		static bool IsValidROI(Rect _roi, Size _imgSz);

		static Mat IncreaseContrast(const Mat& _img); 
		// group images
		static Mat MakeRows(const MatSet& _imgs); 
		static Mat MakeCols(const MatSet& _imgs); 
		static Mat MakeGridLayout(const MatSet& _imgs, Size _gridSz);
		// extend image 
		static Mat ExtendImage(const Mat& _img, int _border);
		// draw
		static void DrawDottedRect(Mat& _img, Rect _rect, CvScalar _color, int _thickness, int _lenghOfDots, int _lineType);
		static void DrawHightlight(Mat& _img, Rect _rect);
	};
}
#endif //IMAGE_TOOLS_H_
