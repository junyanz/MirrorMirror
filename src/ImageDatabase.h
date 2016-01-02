#ifndef IMAGE_DATABASE_H_
#define IMAGE_DATABASE_H_
#include "common.h"

class CImageDatabase
{
public: 
	CImageDatabase() {Clear(); } 
	CImageDatabase(string _dbName, int _maxNumImgs, bool _colorImg = true); 
	CImageDatabase(string _dbName, vectorString& _imgNameVec, 
		string _suffix, bool _colorImg = true); 
	~CImageDatabase() {Clear(); }

	void SetDBType(string _dbType); 
	int NumImages() const {return m_numImgs; }
	string ImageFormat() const {return m_suffix; }
	Mat Image (int _imgIdx) const;
	string ImageName(int _imgIdx) const; 
	vectorString ImageNames() const {return m_imgNameVec; }
	//string DBName() const {return m_dbName; }
	int ImageChannel() const {return Image(0).channels(); }
	int ImageType() const {return Image(0).type(); }
	void ShowAllImages(); 
	void PrintImageNames(); 
	void ThresholdImages(); 
	void ResizeImage(Size _sz); 
	void PrintList(); 
	bool IsEmpty() const; 
	int ImageId(string _name);

private: 
	void Clear(); 
	void AnalyzeDatabase(); 
	bool LoadAllImages(); 
	Mat LoadSingleImage(int _imgIdx); 

private:
	bool m_isColor; 
	int m_maxNumImgs; 
	int m_numImgs;
	int m_rType; 
	string m_dbName;
	string m_suffix; 
	string m_dbType; 
	vectorString m_imgNameVec; 
	vectorString m_extVec; 
	MatSet m_imgVec; 
};

#endif //DATABASEMANAGER_H_
