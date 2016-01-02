#ifndef RECORD_MODULE_H_
#define RECORD_MODULE_H_
#include "common.h"

class CRecordModule {
public: 
	CRecordModule(string _videoFile); 
	~CRecordModule();
	bool IsOpened(); 
	Mat GetFrame(); 
	Mat GetOriFrame() { return m_oriFrame; }
	Size GetFrameSize(); 
	void StartSave() {/*m_isSave = true;*/ } 
	void SaveFrames(); 
	void Pause() { m_isPause = !m_isPause;}

private: 
	void Clear(); 
private: 
	//bool m_isInit; 
	VideoCapture m_cap; 
	Size m_size; 
	bool m_isSave;
	bool m_isPause;
	ImageSet m_cache; 
	Mat m_lastFrame;
	Mat m_oriFrame; 
}; 

#endif //RECORD_MODULE_H_
