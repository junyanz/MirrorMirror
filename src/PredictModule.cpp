#include "Parameter.h"
#include "ImageDatabase.h"
#include "ImageTools.h"
#include "Utility.h"
#include "FastImage.h"
#include "PredictModule.h"
#include "FeatureModule.h"
#include "WarpingModule.h"
#include "FadeModule.h"
#include "WarpUtils.h"
#include "SVMSolver.h"
#include "ImageTools.h"
#include "CLMTrackingModule.h"
const int NUM_CACHE = 11;

CPredictModule::CPredictModule(const CParameter* _param) {
    DEBUG_INFO("Loading CPredictModule...");
    Clear();
    m_param = _param;
    m_featModule = new CFeatureModule();

    m_warpModule = new CWarpingModule();
    m_dissolveModule = new CFadeModule();
    m_dataDir = m_param->DataDir();

    LoadModels(m_param->ModelFile());
    LoadReference(m_param->DataDir());
}

CPredictModule::~CPredictModule() {
    DELETE_OBJECT(m_featModule);
    DELETE_ARRAY(m_feat);
    DELETE_OBJECT(m_warpModule);
    DELETE_OBJECT(m_dissolveModule);

    Clear();
}

void CPredictModule::Clear() {
    m_featModule = NULL;
    m_feat = NULL;
    m_warpModule = NULL;
    m_dissolveModule = NULL;
    m_nFrames = 0;
    m_bestScore = 0.5;

    m_modelScores.clear();
    FOR_u (i, m_models.size())
            DELETE_OBJECT(m_models[i]);

    m_models.clear();
}

void CPredictModule::LoadReference(string _imgFile) {
    Mat img = imread(_imgFile);
    if (img.empty())
        DEBUG_ERROR("cannot read reference face (%s)", _imgFile.c_str());
    else {
        m_example = img.clone();
        unsigned nAttrs = m_models.size();
        m_modelScores.assign(nAttrs, 0.0);
        CCLMTrackingModule* trackModule = new CCLMTrackingModule();
        trackModule->TrackFrame(m_example, false);
        m_landmark = trackModule->GetLandmarks();
        m_refFace = CWarpUtils::CropFace(img, m_landmark, m_param->Ratios()).clone();
//        SHOW_IMG(m_refFace);
        m_warpModule->WarpToFrontalFace3D(img, m_landmark);
        Mat warpImg = m_warpModule->FrontalFace();
        //        SHOW_IMG(warpImg);
        m_dissolveModule->SetTexture(warpImg.clone());
        float* f = m_featModule->GetFeature(warpImg);
        vector<float*> ff(1, f);
        vectord ss;

        FOR_u (n, nAttrs) {
            ss = m_models[n]->Predict(ff, m_featModule->FeatureLength());
            m_modelScores[n] = ss[0];
            DEBUG_INFO("attrbute (%s), score (%3.3f)", m_attrs[n].c_str(), m_modelScores[n]);
        }
        DELETE_ARRAY(f);
        ff.clear();
    }
}

void CPredictModule::LoadModels(string _modelFold) {
    vector<string> modelNames;
    int nModels = utility::CUtility::FindFiles(_modelFold, modelNames);
    m_models.clear();

    FOR (n, nModels) {
        string modelPath = _modelFold + modelNames[n];
        CSVMSolver* svm = new CSVMSolver();
        svm->SetModelPath(modelPath);
        svm->LoadModel();
        m_models.push_back(svm);
        string name = modelNames[n].substr(0, modelNames[n].find("_"));
        DEBUG_INFO("Loading (%s) model.", name.c_str());
        m_attrs.push_back(name);
    }
}

vectord CPredictModule::PredictScore() {
    unsigned nAttrs = m_models.size();
    vectord preds(nAttrs, 0);
    vector<float*> ff(1, m_feat);
    vectord ss;

    FOR_u (n, m_models.size()) {
        ss = m_models[n]->Predict(ff, m_featModule->FeatureLength());
        preds[n] = ss[0];
    }

    ff.clear();
    return preds;
}

void CPredictModule::SetImageAndPnts(Mat& _frame, Mat& _cropFrame, PointSetd _pnts) {
    m_cropFrame = _cropFrame.clone();
    m_frame = _frame;
    m_pnts = _pnts;
    DELETE_ARRAY(m_feat);
    m_feat = m_featModule->GetFeature(m_cropFrame, true);
    m_isValid = (m_feat != NULL);
}

void CPredictModule::VisualizeScore( Mat& _img, const vectord& _preds, bool _isDraw = true, float _ratio) {
    int nAttrs = (int)m_models.size();
    int h = _img.rows;
    int w = _img.cols;
    vector<Scalar> colors;
    vectord preds = _preds;
    colors.push_back(Scalar(230, 216, 173));  // light blue
    colors.push_back(Scalar(255, 0, 0));  // bright blue
    colors.push_back(Scalar(112, 25, 25));  // dark blue

    vectorString attrs = m_attrs;
    attrs.push_back("similarity");
    double matchCost = max(0.0, min(1.0, 1-m_cost)); // transform error
    preds.push_back(matchCost);

    int nLength = 12;
    FOR_u (n, preds.size()) {
        double pred = preds[n];
        if (_isDraw) {
            int height  = roundInt(h * min(1.0, max(pred, 0.0))/2.0);
            Rect rect(roundInt(w/12)*(n)+2, h-height/2, roundInt(w/16), height/2);
            rectangle(_img, rect, colors[n], -1, CV_AA);
        }

        stringstream title;
        title << attrs[n];
        title << ": " << std::setfill('0') << std::fixed << setprecision(2) << pred;
        cv::putText(_img, title.str(), cv::Point(10, 20+roundInt(n*20*_ratio)), CV_FONT_HERSHEY_SIMPLEX, 0.5*_ratio, colors[n], roundInt(2*_ratio));
    }
}

Mat CPredictModule::CrossFade(bool _isReset, float& _r, bool _isPause) {
    if (_isReset) {
        m_nFrames = 0;
        m_dissolveModule->Reset();
        return Mat();
    }

    _r = 0.0f;
    m_isReset = false;
    int nForthFrames = 20;
    int nBackFrames = 20;
    int nFrames = nForthFrames + nBackFrames;

    m_nFrames++;
    Mat src = m_frame;
    Mat ref = m_example;

    PointSetd refPnts = m_landmark;
    PointSetd srcPnts = m_pnts;
    int frameId = m_nFrames % nFrames;
    if (frameId < nForthFrames)
        _r = frameId / (float)(nForthFrames-1);
    else
        _r = 1 - (frameId-nForthFrames)/(float)(nBackFrames-1);
    _r = 1-_r;
    _r = _r * 0.75f;

    Mat blend_img = m_dissolveModule->Fade3D(src,
                                             refPnts, srcPnts, _r, false);
    if (!_isPause) {
         m_cost = m_dissolveModule->Error();
        //DEBUG_INFO("compute cost  = %3.3f", m_cost);
    }
    return blend_img;
}

void CPredictModule::VisualizeGTScore( Mat& _img) {
    vectord preds;
    FOR_u (n, m_models.size())
            preds.push_back(m_modelScores[n]);

    int nAttrs = (int)preds.size();
    int h = _img.rows;
    int w = _img.cols;
    vector<Scalar> colors;
    colors.push_back(Scalar(230, 216, 173));  // light blue
    colors.push_back(Scalar(255, 0, 0));  // bright Blue
    colors.push_back(Scalar(112, 25, 25));  // dark blue

    preds.push_back(1.0);

    int border = 2;

    FOR_u (n, preds.size()) {
        double pred = preds[n];
        int height  = roundInt(h * min(1.0, max(pred, 0.0))/2.0);
        Rect rect(roundInt(w/12)*(n)+2-border, h-height/2, roundInt(w/16)+2*border, height/2);
        rectangle(_img, rect, colors[n], 2, CV_AA);
    }
}

bool CPredictModule::IsAutoSave( vectord preds ) {
    //return false; // hack

    bool flag = false;
    double score = preds[0];
    double cost = abs(preds[0]-m_modelScores[0])
            + abs(preds[1]-m_modelScores[1])
            + min(1.0, m_cost/2);
    if (preds[0] > m_modelScores[0] * 0.85    // attractive
            && preds[1] > m_modelScores[1] * 0.8 // serious
            && preds[1] < m_modelScores[1] * 1.25 // serious
            && m_cost < 0.20) {  // matching cost

    } else {
        score -= 2.0;
        cost += 100.0;
    }


    if ((int)m_scoreCache.size() == NUM_CACHE)
        m_scoreCache.erase(m_scoreCache.begin());
    m_scoreCache.push_back(score);

    if ((int)m_scoreCache.size() == NUM_CACHE) {
        int maxIdx = -1;
        double minCost = -1;
        FOR_u (i, NUM_CACHE) {
            if (m_scoreCache[i] > minCost) {
                minCost = m_scoreCache[i];
                maxIdx = (int)i;
            }
        }

        if (maxIdx == (NUM_CACHE-1)/2+1 && minCost > m_bestScore) {
            flag = true;
            m_bestScore = minCost;
            cout << "pick up: " << minCost << endl << endl;
        }
    }


    return flag;
}

bool CPredictModule::IsPeakFrame() {
    int maxIdx = -1;
    double maxValue = -1;
    FOR_u (i, m_scoreCache.size()) {
        if (m_scoreCache[i] > maxValue) {
            maxValue = m_scoreCache[i];
            maxIdx = (int)i;
        }
    }

    if (maxIdx == (int)m_scoreCache.size()-1)
        return true;
    else
        return false;
}


void CPredictModule::Reset(bool _resetAll) {
    if (_resetAll) {
        m_cost = 0;
        m_bestScore = 0.5;
        m_scoreCache.clear();
    }

    m_nFrames = 0;
    m_dissolveModule->Reset();
    m_isReset = true;
}


Mat CPredictModule::ReferenceFrame() {
    Mat img = m_refFace;
    Mat out;
    double ratio = 300.0 / img.rows;
    resize(img, out, Size(), ratio, ratio);
    return out;
}

Mat CPredictModule::HalfBlend() {
    return m_dissolveModule->HalfBlend().clone();
}

Mat CPredictModule::Warp3D() {
    return m_dissolveModule->Warp3d().clone();
}

Mat CPredictModule::FullBlend() {
    return m_dissolveModule->FullBlend().clone();
}

Mat CPredictModule::OriReferenceFrame() {
    return m_example.clone();
}
