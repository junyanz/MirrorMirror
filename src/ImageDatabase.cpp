#include "ImageDatabase.h"
#include "Utility.h"
#include "Timer.h"

void CImageDatabase::Clear() {
	m_numImgs = 0;
	m_imgNameVec.clear();
	m_extVec.clear();
	m_imgVec.clear();
	m_isColor = true; 
}

CImageDatabase::CImageDatabase(string _dbName, int _maxNumImgs, bool _colorImg) {
	Clear();
	m_dbName = _dbName; 
	m_isColor = _colorImg; 
	m_maxNumImgs = _maxNumImgs;
    AnalyzeDatabase();
	LoadAllImages(); 
}

CImageDatabase::CImageDatabase( string _dbName, vectorString& _imgNameVec, string _suffix, bool _colorImg) {
	Clear();
	m_dbName = _dbName; 
	m_isColor = _colorImg; 
	m_imgNameVec = _imgNameVec; 
	m_numImgs = (int)m_imgNameVec.size(); 
	m_suffix = _suffix; 
	m_extVec.resize(m_numImgs, m_suffix); 
	m_imgVec.resize(m_numImgs); 
	LoadAllImages(); 
}

void CImageDatabase::AnalyzeDatabase() {
    DEBUG_INFO("Analyzing Database");
	m_numImgs = utility::CUtility::FindImageFiles(m_dbName, m_imgNameVec, m_extVec);
	m_numImgs = min(m_numImgs, m_maxNumImgs); 
	vector<string> oldVec = m_imgNameVec; 
	m_imgNameVec.resize(m_numImgs); 

	FOR (i, m_numImgs)
		m_imgNameVec[i] = oldVec[i]; 
	
	m_imgVec.resize(m_numImgs); 

	if (!m_extVec.empty()) //junyanz: only one image format
		m_suffix = m_extVec[0];
}

bool CImageDatabase::LoadAllImages() {
    CTimer timer("Loading all images");

	#pragma omp parallel for 
	FOR (i, m_numImgs) {
		string imgName = m_dbName + m_imgNameVec[i] + m_extVec[i]; 
		//DEBUG_INFO("loading image (%s)", imgName.c_str());
		if (m_isColor)
			m_imgVec[i] = imread(imgName, CV_LOAD_IMAGE_COLOR); 
		else
			m_imgVec[i] = imread(imgName, CV_LOAD_IMAGE_GRAYSCALE); 
		if (m_imgVec[i].empty())
			DEBUG_INFO("Fail to load image (%s)", imgName.c_str()); 
	}

	return true; 
}

cv::Mat CImageDatabase::Image(int _imgIdx) const {
	if (!IsEmpty() && _imgIdx < m_numImgs && _imgIdx >= 0) 
		if (_imgIdx < (int) m_imgVec.size() && !m_imgVec[_imgIdx].empty()) 
			return m_imgVec[_imgIdx];

	return Mat();
}

cv::Mat CImageDatabase::LoadSingleImage(int _imgIdx) {
	string imgName = m_dbName + m_imgNameVec[_imgIdx] + "." + m_extVec[_imgIdx];
	Mat img = imread(imgName);
	img.convertTo(img, m_rType); 

	if (img.empty())
		DEBUG_ERROR("Fail to load image (%s)", imgName.c_str());

	return img;
}

void CImageDatabase::ShowAllImages() {
	FOR (i, m_numImgs) {
		Mat img = Image(i);
		img.convertTo(img, CV_8UC3); 
		imshow("Show Image Window", img);
		waitKey(500);
	}
}

string CImageDatabase::ImageName( int _imgIdx ) const {
	return m_imgNameVec[_imgIdx]; 
}

void CImageDatabase::PrintImageNames() {
	FOR (i, 50)
		cout << ImageName(i) << endl; 
}

void CImageDatabase::ThresholdImages() {
	#pragma omp parallel for 
	FOR (i, m_numImgs)
		threshold(m_imgVec[i], m_imgVec[i], 128, 255, THRESH_BINARY); 
}

void CImageDatabase::PrintList() {
	string imgListPath = m_dbName.substr(0, m_dbName.find_last_of("\\")) + ".list"; 
    ofstream fout(imgListPath.c_str());
	
	if (fout != NULL) {
		DEBUG_INFO("save image list to (%s)", imgListPath.c_str()); 
		FOR (i, m_numImgs)
			fout << m_imgNameVec[i] << endl; 
		fout.close();
		exit(0); 
	}
}


void CImageDatabase::ResizeImage( Size _sz ) {
	#pragma omp parallel for 
	FOR (i, m_numImgs)
		resize(m_imgVec[i], m_imgVec[i], _sz); 
}

void CImageDatabase::SetDBType( string _dbType ) {
	m_dbType = _dbType; 
}

bool CImageDatabase::IsEmpty() const {
	if (NumImages() == 0)
		return true; 
	if (m_imgVec.empty())
		return true; 

	FOR (i, (int)m_imgVec.size())
		if (m_imgVec[i].empty())
			return true; 

	return false; 
}

int CImageDatabase::ImageId( string _name ) {
	FOR_u (i, m_imgNameVec.size()) {
		if (_name == m_imgNameVec[i] + m_extVec[i])
			return (int)i; 
	}
	return -1; 
}
