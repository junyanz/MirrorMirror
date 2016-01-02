#ifndef RESIZABLE_WINDOW_H_
#include "common.h"

class CResizableWindow {
public: 
	CResizableWindow();
	CResizableWindow(string _name); 
	~CResizableWindow();
	void ShowImage(const Mat& _img);
	void namedWindow(string _name);

private: 
	void Clear(); 

private: 
	double m_ratio; 
	string m_name; 
};


#endif //RESIZABLE_WINDOW_H_