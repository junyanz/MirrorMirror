#include "ResizableWindow.h"

CResizableWindow::CResizableWindow( string _name ) {
	Clear();
	m_name = _name;
	cv::namedWindow(m_name);
}

CResizableWindow::CResizableWindow() {
	Clear();
}

CResizableWindow::~CResizableWindow() {
	Clear();
	destroyWindow(m_name);
}

void CResizableWindow::Clear() {
	m_ratio = 1.0; 
	m_name.empty();
}

void CResizableWindow::ShowImage( const Mat& _img ) {
	char key = waitKey(1);
	if (key == '=')
		m_ratio *= 1.05; 
	if (key == '-')
		m_ratio /= 1.05;

	Mat disp;
	resize(_img, disp, Size(), m_ratio, m_ratio);
	imshow(m_name, disp);
}

void CResizableWindow::namedWindow( string _name ) {
	m_name = _name; 
	cv::namedWindow(m_name);
}
