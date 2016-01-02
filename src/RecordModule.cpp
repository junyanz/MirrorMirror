#include "RecordModule.h"
#include "ImageTools.h"

void CRecordModule::Clear() {
	m_size = Size(-1, -1); 
	m_isSave = false; 
}


CRecordModule::CRecordModule( string _videoFile ) {
    DEBUG_INFO("Loading CRecordModule...");
	Clear(); 
    if (_videoFile.empty()) {
        m_cap.open(0);
        m_isPause = false;
        if (IsOpened())
            DEBUG_INFO("Open the webcam");
        else
            DEBUG_ERROR("Fail to open the webcam");
    } else {
        m_cap.open(_videoFile);
        m_isPause = false;
        if (IsOpened())
            DEBUG_INFO("Open the video (%s)", _videoFile.c_str());
        else
            DEBUG_ERROR("Fail to open the video (%s)", _videoFile.c_str());
    }
}

CRecordModule::~CRecordModule() { 
	SaveFrames(); 
	Clear(); 
}

bool CRecordModule::IsOpened() {
	if (m_cap.isOpened()) 
		DEBUG_INFO("Camera is open"); 
	else 
		DEBUG_INFO("Camera is disabled"); 
	return m_cap.isOpened();  
}

cv::Mat CRecordModule::GetFrame() {
	Mat input, crop, frame; 
	//int fixedSize = 200;
	//waitKey(15);
	//FOR (i, 10)
	if (!m_isPause) {
		m_cap >> input;
		//if (m_isSave)
		//	m_cache.push_back(input);

		if (input.empty())
			DEBUG_ERROR("Fail to get frame.");
		else {
			int w = input.cols; 
			int h = input.rows; 
			if (w > h) {
				int b = (w-h)/2; 
				crop = input(Rect(b, 0, w-2*b, h));
			} else if (h > w) {
				int b = (h-w)/2;
				crop = input(Rect(0, b, w, h-2*b));
			} else
				crop = input;
			resize(crop, frame, Size(300, 300));
			m_lastFrame = frame; 
			m_oriFrame = crop; 
		}

		return frame; 
	} else
		return m_lastFrame;
}

Size CRecordModule::GetFrameSize() {
	if (m_size == Size(-1, -1))
		m_size = GetFrame().size(); 
	return m_size; 
}

void CRecordModule::SaveFrames()  {
	//cout << m_cache.size() << endl; 
	//FOR_u (n, m_cache.size()) {
	//	SHOW_IMG(m_cache[n]);
	//	/*char str[255];
	//	sprintf_s(str, "saved//%4.4d.jpg", n);
	//	imwrite(str, m_cache[n]);*/
	//}
}

