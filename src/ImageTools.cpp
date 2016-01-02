#include "ImageTools.h"
#include "FastImage.h"


Mat ImageTools::CImageTools::MakeRows(const MatSet& _imgs) {
	if (_imgs.empty())
		return Mat(); 
	Mat img = _imgs[0]; 

	FOR (i, (int)_imgs.size()-1)
		hconcat(img, _imgs[i+1], img); 
	return img; 
}

Mat ImageTools::CImageTools::MakeCols(const MatSet& _imgs) {
	if (_imgs.empty())
		return Mat(); 
	Mat img = _imgs[0]; 
	FOR (i, (int)_imgs.size()-1)
		vconcat(img, _imgs[i+1], img); 
	return img; 
}

Rect ImageTools::CImageTools::GetROI( const vector<Point2i>& _points ) {
	int x1 = 100000;  
	int y1 = 100000; 
	int x2 = 0; 
	int y2 = 0; 

	FOR (i, (int)_points.size()) {
		x1 = min(x1, _points[i].x); 
		y1 = min(y1, _points[i].y);
		x2 = max(x2, _points[i].x);
		y2 = max(y2, _points[i].y);
	}

	return Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1); 
}

cv::Mat ImageTools::CImageTools::RandomImage( const Mat& _img ) {
	Mat out = Mat::zeros(_img.size(), _img.type()); 
	randu(out, Scalar(0), Scalar(256)); 
	return out; 
}

bool ImageTools::CImageTools::IsValidROI( Rect _roi, Size _imgSz ) {
	if (_roi.x < 0 || _roi.y < 0 || _roi.x +_roi.width-1 >= _imgSz.width 
		|| _roi.y + _roi.height - 1 >= _imgSz.height || _roi.width <= 0 || _roi.height <= 0) //check
		return false; 
	else
		return true; 
}

cv::Mat ImageTools::CImageTools::IncreaseContrast ( const Mat& _img ) {
	Mat out = Mat::zeros(_img.size(), _img.type()); 
	int nChannel = _img.channels();
	if (nChannel == 1)
		return _img.clone(); 
	//int width = _img.cols; 
	//int height = _img.rows;
	uchar max_value = *std::max_element(_img.begin<uchar>(),_img.end<uchar>());
	uchar min_value = *std::min_element(_img.begin<uchar>(),_img.end<uchar>()); 
	min_value = (uchar)((int)min_value); 
	int norm = max(1, (int)max_value - (int)min_value); 
	out = (_img - min_value)/norm; 

	return out; 

}

Mat ImageTools::CImageTools::MakeGridLayout( const MatSet& _imgs, Size _gridSz ) {
    assert((int)_imgs.size() == _gridSz.width*_gridSz.height);
	MatSet rowImgs; 
	FOR (h, _gridSz.height) {
		MatSet imgs; 
		FOR (w, _gridSz.width)
			imgs.push_back(_imgs[h*_gridSz.width+w]);
		Mat rowImg = MakeRows(imgs); 
		rowImgs.push_back(rowImg); 
		imgs.clear(); 
	}
	Mat gridImg = MakeCols(rowImgs); 
	rowImgs.clear(); 
	return gridImg; 
}

Mat ImageTools::CImageTools::ExtendImage( const Mat& _img, int _border ) {
	if (_border == 0)
		return _img.clone(); 
	//SHOW_IMG(_img);
	int h = _img.rows; 
	int w = _img.cols; 
	Mat img_e = Mat::zeros(Size(w+2*_border, h+2*_border), CV_8UC3);
	_img.copyTo(img_e(Rect(_border, _border, w, h)));
	//SHOW_IMG(img_e);
	Mat border_im_x=  _img(Rect(0, 0, w, _border)).clone();
	flip(border_im_x, border_im_x, 0); 
	border_im_x.copyTo(img_e(Rect(_border, 0, w, _border)));
	//SHOW_IMG(img_e);

	border_im_x = _img(Rect(0, h-_border, w, _border)).clone();
	flip(border_im_x, border_im_x, 0);
	border_im_x.copyTo(img_e(Rect(_border, h+_border, w, _border)));
	//SHOW_IMG(img_e);

	Mat border_im_y =img_e(Rect(_border, 0, _border, h+_border*2)).clone();
	flip(border_im_y, border_im_y, 1); 
	border_im_y.copyTo(img_e(Rect(0, 0, _border, h+_border*2)));
	//SHOW_IMG(img_e);

	border_im_y = img_e(Rect(w, 0, _border, h+_border*2)).clone();
	flip(border_im_y, border_im_y, 1);
	border_im_y.copyTo(img_e(Rect(w+_border, 0, _border, h+_border*2)));
	//SHOW_IMG(img_e);

	return img_e;
}


void cvDrawDottedLine(Mat& img,CvPoint pt1, CvPoint pt2,CvScalar color, int thickness, int lenghOfDots, int lineType, int leftToRight)
{
	LineIterator it(img, pt1, pt2, 8);            // get a line iterator
	for(int i = 0; i < it.count; i++,it++) {
		if ( i%lenghOfDots!=0 ) {
			(*it)[0] = (uchar)color.val[0];
			(*it)[1] = (uchar)color.val[1];
			(*it)[2] = (uchar)color.val[2];
		}     
	}

} 


void ImageTools::CImageTools::DrawDottedRect(Mat& _img, Rect rect, CvScalar color, int thickness, int lenghOfDots, int lineType ) {
	Point2d pt1 = Point2d(rect.x,rect.y);
	Point2d pt2 = Point2d(rect.x+rect.width,rect.y);
	Point2d pt3 = Point2d(rect.x+rect.width,rect.y+rect.height);
	Point2d pt4 = Point2d(rect.x,rect.y+rect.height);
	//IplImage tmp = _img;
	cvDrawDottedLine(_img,pt1,pt2,color,thickness,lenghOfDots,lineType, 0);
	cvDrawDottedLine(_img,pt2,pt3,color,thickness,lenghOfDots,lineType, 0);
	cvDrawDottedLine(_img,pt3,pt4,color,thickness,lenghOfDots,lineType, 1);
	cvDrawDottedLine(_img,pt4,pt1,color,thickness,lenghOfDots,lineType, 1); 
}

void ImageTools::CImageTools::DrawHightlight( Mat& _img, Rect _rect ) {
	Mat img_f, out_f;
	_img.convertTo(img_f, CV_32F);
	Mat ones_f = Mat(img_f.size(), img_f.type(), Scalar(255.0f, 255.0f, 255.0f));// * 255.0;
	float alpha = 0.25f;
	out_f = img_f * alpha + ones_f * (1-alpha);
	img_f(_rect).copyTo(out_f(_rect));
	out_f.convertTo(_img, CV_8U);
	//SHOW_IMG(_img);
}
