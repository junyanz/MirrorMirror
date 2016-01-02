#include "SaveModule.h"
#include "Utility.h"
#include "ImageTools.h"

CSaveModule::CSaveModule(string _foldPath) {
	Clear();
	m_fold = _foldPath; 
    string countFile = m_fold + "_count.txt";
    ifstream fin(countFile.c_str());
	DEBUG_INFO("read count file (%s)", countFile.c_str());
	if (fin != NULL) {
		fin >> m_nArchives; 
		fin.close(); 
	} else
		m_nArchives = 0; 
}

CSaveModule::~CSaveModule() {
	//DEBUG_INFO("DELETE SAVEMODULE");
	ShowAll();
	Clear(); 
}

void CSaveModule::Clear() {
	m_nFrames = 0;
	m_nArchives = -1;
	m_isCreated = false; 
	m_frames.clear(); 
	m_disps.clear(); 
	m_scores.clear();
}

void CSaveModule::SaveFrame(const Mat& _disp, const Mat& _ref, const Mat& _frame, const Mat& _warp3d, const Mat& _half,
							const Mat& _full, const vectord& _scores, 
	double _cost, double _t, const PointSetd& _landmarks,  string _print) {
	stringstream ss; 
	if (!m_isCreated)
		CreateFolder();
	ss << m_saveFold+"//";
	ss << _print << "_";

	ss << "frame_"<< m_nFrames; 
	string filePath = ss.str() + "_meta.txt";
	string dispName = ss.str() + "_scored.png";
	string oriName = ss.str() + "_ori.png";
	string warpName = ss.str() + "_warp3d.png";
	string blendName = ss.str() + "_blend.png";
	string finalName = ss.str() + "_final.png";
	string refName = ss.str() + "_ref.png";

	m_frames.push_back(_frame);
	m_disps.push_back(_disp);
	if (!_disp.empty())
		imwrite(dispName, _disp); 
	if (!_ref.empty())
		imwrite(refName, _ref);
	if (!_frame.empty())
		imwrite(oriName, _frame);
	if (!_warp3d.empty())
		imwrite(warpName, _warp3d);
	if (!_full.empty())
		imwrite(blendName, _full);

	if (_print != "control") {
		ImageSet imgs; 
		Size sz = _half.size();
		Mat frame_s; 
		resize(_frame, frame_s, sz);
		imgs.push_back(frame_s);
		imgs.push_back(_half);
		imgs.push_back(_full);
		imgs.push_back(_warp3d);
		Mat final = ImageTools::CImageTools::MakeRows(imgs);
		if (!final.empty())
			imwrite(finalName, final);
	}
	//m_scores = _preds; 
	m_scores.push_back(_scores[0]);
    ofstream fout(filePath.c_str());
	if (fout == NULL) {
		DEBUG_INFO("cannot save meta data (%s)", filePath.c_str());
	} else {
		fout << "timestamp: " << _t << endl; 
		fout << "finalName: " << finalName << endl; 
		//fout << "oriName: " << oriName << endl; 
		fout << "dispName: " << dispName << endl; 
		//FOR (n, _scores.size())
		fout << "attractive: " << _scores[0] << endl; //" ";
		fout << "serious: " << _scores[1] << endl;
		fout << "matchCost: " << _cost << endl; 
		if (_print != "control") {
		fout << "points:" << (int)_landmarks.size() << endl;

		FOR_u (i, _landmarks.size())
			fout << _landmarks[i] << endl; 
		}

		fout.close();
	}

	m_nFrames++;
}

void CSaveModule::CreateFolder() {
	stringstream ss; 
    ss << m_fold + "_archive_" << m_nArchives;
	m_saveFold = ss.str();
	utility::CUtility::mkdirs(m_saveFold);
    string countFile = m_fold + "_count.txt";
    ofstream fout(countFile.c_str());
	DEBUG_INFO("write count file (%s)", countFile.c_str());
	m_nArchives++;
	fout << m_nArchives; 
	fout.close(); 
	m_isCreated = true; 
}

void CSaveModule::ShowAll() {
	DEBUG_INFO("show saved images");
	if (m_nFrames == 0)
		return; 

	int nCols = 3;

	int nRows = (int)ceil(m_nFrames/(double)nCols);
	ImageSet rowIms(nRows);

	double height = 0.0;
	double weight = 0.0;
	FOR_u (i, m_disps.size()) {
		height += m_disps[i].rows;
		weight += m_disps[i].cols;
	}
	

	double h = height/(double)m_disps.size();
	double w = weight/(double)m_disps.size();
	//w = 200/h*w;
	h = 360.0/w*h;
	w = 360.0;
	Size sz = Size(roundInt(w),roundInt(h));
	Mat emptyIm = Mat(sz, m_disps[0].type(), Scalar(255, 255,255));
	FOR (r, nRows) {
		ImageSet colIms(nCols);
		FOR (c, nCols) {
			int id = c + r * nCols;
			if (id < m_nFrames) {
				resize(m_disps[id], colIms[c], sz);
				stringstream title; 
				title <<  std::setfill('0') << std::fixed << setprecision(2) <<  m_scores[id];
				cv::putText(colIms[c], title.str(), cv::Point(10, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
			} else 
				colIms[c] = emptyIm.clone();

		}
		rowIms[r] = ImageTools::CImageTools::MakeRows(colIms);
	}

	Mat finalDisp = ImageTools::CImageTools::MakeCols(rowIms);
    imshow("selected attractive expressions", finalDisp);
    waitKey();
}

void CSaveModule::AutoSave( const Mat& _disp, const Mat& _frame, vectord _preds ) {

}

void CSaveModule::SavePeak(vectord _preds) {

}

void CSaveModule::Reset() {

}



