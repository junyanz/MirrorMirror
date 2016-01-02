#include "TrainModule.h"
#include "Utility.h"
#include "ImageDatabase.h"
#include "Parameter.h"
#include "FeatureModule.h"
#include "WarpingModule.h"
#include "SVMSolver.h"
#include "SaveModule.h"
#include "WarpUtils.h"
#include "CLMTrackingModule.h"

CTrainModule::CTrainModule(const CParameter* _param) {
	Clear(); 
	m_param = _param; 
	string dataDir = _param->DataDir(); 
	m_imgFold = dataDir + "imgs//";
	m_imgList = dataDir + "image.txt"; 
	m_scoreFold = dataDir + "scores//";
    m_modelFold = _param->ModelFile();
	//m_pntPath = dataDir + "point.txt"; 

	vectorString imgNames; 
    ifstream fin(m_imgList.c_str());
	if (fin != NULL) {
		while (!fin.eof()) {
			string name; 
			fin >> name; 
			if (!name.empty())
				imgNames.push_back(name); 
		}
		fin.close(); 
        DEBUG_INFO("reading (%d) images", (int)imgNames.size());
	} else
		DEBUG_ERROR("Fail to load image list");
	
	m_imgDB = new CImageDatabase(m_imgFold, imgNames, ".png", true);
	m_nImgs = m_imgDB->NumImages(); 
    DEBUG_INFO("load (%d) images", m_nImgs);
//	cout << "#images = " << m_nImgs << endl;
	m_warpModule = new CWarpingModule(); 
    m_featModule = new CFeatureModule();
    m_detectModule = new CCLMTrackingModule();
}

void CTrainModule::Train() {
	// set data
	vector<float*> data; //(m_nImgs, NULL);
	vectorb isValid(m_nImgs, true);

	FOR (i, m_nImgs) {
		Mat img = m_imgDB->Image(i);  
        bool isDetect = m_detectModule->TrackFrame(img, false); //pnt);
		if (isDetect) {
            PointSetd pnts = m_detectModule->GetLandmarks();
            m_warpModule->WarpToFrontalFace3D(img, pnts);
			Mat warpImg = m_warpModule->FrontalFace();
//            SHOW_IMG(warpImg);
			if (!warpImg.empty()) {
				//PointSetd warpLandmarks = m_warpModule->WarpedLandmark(); 
				float* f = m_featModule->GetFeature(warpImg);
				//float* f = m_featModule->GetFeature(warpImg, warpLandmarks);
				data.push_back(f);
			} else 
				isValid[i] = false; 
		} else
			isValid[i] = false; 
	}

	int ndims = m_featModule->FeatureLength(); 
//	cout << "ndims = " << ndims << endl;
    DEBUG_INFO("feature length = %d", ndims);
	vectorString scoreFiles; 
	int nScores = utility::CUtility::FindFiles(m_scoreFold, scoreFiles);
    DEBUG_INFO("load (%d) score files", nScores);

    vectorString tmp(1, m_modelFold);
	utility::CUtility::mkdirs(tmp);

	FOR (n, nScores) {
        DEBUG_INFO("loading score file (%s)", scoreFiles[n].c_str());
		vectord score;
		string scorePath = m_scoreFold + scoreFiles[n];
        ifstream fin_s(scorePath.c_str());
		if (fin_s != NULL) {
			double s; 
			FOR (i, m_nImgs) {
				fin_s >> s; //score[i];
				if (isValid[i])
					score.push_back(s);
			}
			fin_s.close(); 
		} else
            DEBUG_ERROR("Fail to load score file (%s)", scorePath.c_str());

        CSVMSolver* svm = new CSVMSolver();
		string modelPath = m_modelFold + scoreFiles[n].substr(0, scoreFiles[n].find('_')) + "_svm.model";
		cout << modelPath << endl; 
        svm->SetModelPath(modelPath);
		NormalizeScore(score); 
        svm->CrossValidation(data, score, ndims, 10);
        svm->Train(data, score, ndims);
        DELETE_OBJECT(svm);
		score.clear(); 
	}

	FOR_u (i, data.size())
		DELETE_ARRAY(data[i]);
	data.clear(); 
}

CTrainModule::~CTrainModule() {
	DELETE_OBJECT(m_warpModule); 
	DELETE_OBJECT(m_featModule); 
    DELETE_OBJECT(m_detectModule);
//	DELETE_OBJECT(m_svm);
	Clear(); 
}


void CTrainModule::Clear() {
	m_warpModule = NULL; 
    m_featModule = NULL;
    m_detectModule = NULL;
}

void CTrainModule::NormalizeScore( vectord& _score ) {
	double min_score = vecMin(_score); 
	double norm_score = vecMax(_score) - min_score; 

	FOR_u (i, _score.size())
		_score[i] = (_score[i] - min_score) / norm_score; 
}
