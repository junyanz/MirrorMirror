#include "MirrorMain.h"
#include "RecordModule.h"
#include "WarpingModule.h"
#include "TrainModule.h"
#include "PredictModule.h"
#include "FadeModule.h"
#include "CLMTrackingModule.h"
#include "SaveModule.h"
#include "WarpUtils.h"
#include "ImageTools.h"
#include "Timer.h"
#include "Utility.h"
#include "Parameter.h"


#ifdef WIN32
void GetDesktopResolution(int& horizontal, int& vertical) {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

#endif
CMirrorMain::CMirrorMain(const CParameter* _param) {

    Clear();
    m_param = _param;
    // read parameter
    m_timer = new CTimer("MirrorMirror");
    EWorkMode workMode = m_param->WorkMode();

    if (workMode == WORKMODE_UI) { // run UI
        printf("*****************************************************************************************\n");
        printf("Press <s> to start the tracker\n");
        printf("Press <r> to restart the tracker if the tracker fails\n");
        printf("Press <p> to save the current expression\n");
        printf("Press <d> to pause the system to see fine-grained differences at a frozen moment of time.\n");
        printf("Press <q> to quite the program\n");
        printf("*****************************************************************************************\n");

        m_recordModule = new CRecordModule(m_param->VideoFile());
        m_tracker = new CCLMTrackingModule();
        m_frameRate = new utils::CFrameRate(50);

        m_warpModule = new CWarpingModule();
        m_saveModule = new CSaveModule(m_param->SaveDir());
        m_predictModule = new CPredictModule(m_param);

        m_ratio = ReadRatio();
        SetUpWindows();
    }

    if (workMode == WORKMODE_TRAIN)
        m_trainModule = new CTrainModule(m_param);
}

CMirrorMain::~CMirrorMain() {
    DELETE_OBJECT(m_saveModule);
    DELETE_OBJECT(m_recordModule);
    DELETE_OBJECT(m_warpModule);
    DELETE_OBJECT(m_trainModule);
    DELETE_OBJECT(m_predictModule);
    DELETE_OBJECT(m_timer);
    Clear();
}

void CMirrorMain::Clear() {
    m_isPause = false;
    m_nFrames = 0;
    m_timer = NULL;
    m_param = NULL;
    m_recordModule = NULL;
    m_warpModule = NULL;
    m_saveModule = NULL;
    m_trainModule = NULL;
    m_predictModule = NULL;
}

void CMirrorMain::Run() {
    switch (m_param->WorkMode()) {
    case WORKMODE_UI:
        RunWebCameraMode();
        break;

    case WORKMODE_TRAIN:
        TrainModel();
        break;

    default:
        break;
    }
}

void CMirrorMain::RunWebCameraMode() {
    int nFrames = 0;

    while (true) {
//        waitKey(100);
        nFrames++;
        Mat frame = m_recordModule->GetFrame();
        char key;
        /*if (nFrames == 5)
            CSoundModule::MirrorSound();*/
        if (!frame.empty()) {
            key = KeyboardInput();
            m_frame = frame.clone();

            if (m_tracker->IsStart()) {
                bool isTrack = m_tracker->TrackFrame(frame,  false);

                if (isTrack)
                    ProcessFrame(frame);
                else {
                    printf("tracker fails in (%5.5d)-th frame\r", nFrames);
                    ImShowRatio(m_mainName, frame);
                }
            } else {
                Mat ref = m_predictModule->ReferenceFrame();
                imshow(m_refName, ref);
                ImShowRatio(m_mainName, frame);
            }
        } else
            break;
    }
}

void CMirrorMain::TrainModel() {
    m_trainModule->Train();
}

char CMirrorMain::KeyboardInput() {
    char key;
    //int waitT = m_param->WaitTime();
    int waitT = 1;
    if (m_param->WaitTime() > 0)
        key = waitKey(waitT);
    else if (waitT <= 0)
        key = waitKey();
    else
        key = waitKey(1);

    m_isReset = false;
    m_isPrint = false;
    //m_saveName = "print";

    if (key == 'q') {
        m_saveModule->ShowAll();
        m_recordModule->SaveFrames();
        exit(0);
    }

    if (key == 'p')
        m_isPrint = true;

    if (key == 'r') {
        DEBUG_INFO("restart tracking");
        m_tracker->Restart();
        m_saveModule->Reset();
        m_predictModule->Reset(true);
    }

    if (key == 's')
        m_tracker->StartTracking();

    if (key == 'c') {
        m_isReset = true;
        m_predictModule->Reset(true);
    }

    if (key == '+' || key == '=') {
        m_ratio *= 1.03;
        SaveRatio(m_ratio);
    }

    if (key == '-') {
        m_ratio /= 1.03;
        SaveRatio(m_ratio);
    }

    if (key == 'i') {
        SetUpWindows();
        m_tracker->StartTracking();
    }

    if (key == 'd') {
        m_recordModule->Pause();
        m_isPause = !m_isPause;
        if (m_isPause)
            m_isPrint = true;
        //m_saveName = "debug";
        SetUpWindows();
    }
    return key;
}

void CMirrorMain::SetUpWindows() {
    m_oriName = "Current Expression";

    if (!m_isPause) {
        if (!m_mainName.empty())
            destroyWindow(m_mainName);
        m_mainName = "MirrorMirror";
    } else
        m_mainName = "MirrorMirror: Paused";

    m_refName = "Target Expression";
    namedWindow(m_oriName);  //  CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
    namedWindow(m_refName);   //CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
    namedWindow(m_mainName);  //CV_WINDOW_NORMAL |  CV_WINDOW_KEEPRATIO);
}

void CMirrorMain::ProcessFrame(Mat _frame) {
    m_frameRate->tick();
    PointSetd pnts = m_tracker->GetLandmarks();

    if ((int)m_recentPnts.size() >= m_param->NumFrameAlign())
        m_recentPnts.erase(m_recentPnts.begin());
    m_recentPnts.push_back(pnts);
    PointSetd medianPnts = CWarpUtils::MedianPnts(m_recentPnts);
    m_warpModule->WarpToFrontalFace3D(_frame, medianPnts);
    // draw points
    //    PointSetd pnts = CWarpUtils::Mat2Pnts2(medianPnts);
    /*FOR_u (i, pts.size())
        circle(_frame, pts[i], 2, Scalar(0, 255, 0), -1);*/
    Mat warpImg = m_warpModule->FrontalFace();
    //    SHOW_IMG(warpImg);
    bool isSave = false;
    Mat disp;
    Mat final;
    Mat oriFrame = m_recordModule->GetOriFrame().clone();
    //Mat printframe;
    float r = 0.0f;
    //bool isAutoSave;
    Mat ref;
    if (!warpImg.empty()) {
        m_predictModule->SetImageAndPnts(_frame, warpImg, medianPnts); // warpLandmarks);

        if (m_predictModule->IsValid()) {
            if (!m_isPause)
                m_preds = m_predictModule->PredictScore();


            Mat blendIm = m_predictModule->CrossFade(m_isReset, r, m_isPause);
            //disp = blendIm;
            if (blendIm.empty()) { // fail to generate cross-fading result
                disp = _frame.clone();
            } else {
                disp = blendIm;
                m_predictModule->VisualizeScore(disp, m_preds, true);
                m_predictModule->VisualizeGTScore(disp);
                m_cost = m_predictModule->MatchCost();
                //Mat final;

                bool isAutoSave = m_predictModule->IsAutoSave(m_preds);
                if (m_isPause)  // if pause, don't save
                    isAutoSave = false;

                bool isPeak = m_predictModule->IsPeakFrame();
                ref = m_predictModule->ReferenceFrame();
                Mat refDisp = ref.clone();
                circle(refDisp, Point(refDisp.cols-15, refDisp.rows-15), 10, Scalar(0, 0, 255), -1, CV_AA);
                ImShowRatio(m_refName, refDisp);

                if (isPeak || m_isPrint) {
                    //						pnts = CWarpUtils::Mat2Pnts2(medianPnts);
                    Mat faceCrop = CWarpUtils::CropFace(_frame, medianPnts, m_param->Ratios());
                    double ratio = (double)faceCrop.rows / (double)ref.rows;
                    ImageSet imgs;
                    Mat refFrame =ref.clone();
                    resize(refFrame, refFrame, Size(), ratio, ratio);
                    imgs.push_back(faceCrop);
                    imgs.push_back(refFrame);
                    final = ImageTools::CImageTools::MakeRows(imgs);
                    imgs.clear();
                }


                if (isAutoSave) {
                    isSave = true;
                    printf("<<<BINGO>>>\n\n");
                    final = m_peakDisp;
                    m_saveModule->SaveFrame(m_peakDisp, m_peakRef, m_peakFrame, m_peak3D,
                                            m_peakHalf, m_peakFull, m_peakPreds, m_peakCost, m_peakT, m_peakPnts, "bingo");
                } else if (isPeak) {
                    m_peakFrame = oriFrame;
                    m_peakRef = m_predictModule->OriReferenceFrame().clone();
                    m_peakDisp = final.clone();
                    m_peakPreds = m_preds;
                    m_peakT = m_timer->Time();
                    m_peakCost = m_cost;
                    m_peakPnts = pnts;
                    m_peak3D = m_predictModule->Warp3D();
                    m_peakHalf = m_predictModule->HalfBlend();
                    m_peakFull = m_predictModule->FullBlend();
                }
            }
        }
    }

    // draw data
    int waitTime = 500;

    if (isSave) {
        //disp = m_frame.clone();
        Mat peak_s;
        Size size = disp.size();
        resize(m_peakFrame, peak_s, size);
        ImShowRatio(m_mainName, peak_s);
        //m_mainWin.ShowImage(m_peakFrame);
        waitKey(waitTime);
        m_predictModule->Reset();
    } else if (m_isPrint) {
        disp = m_frame.clone();
        double t = m_timer->Time();
        Mat warp3d = m_predictModule->Warp3D();
        Mat half = m_predictModule->HalfBlend();
        Mat full = m_predictModule->FullBlend();
        Mat ref_o = m_predictModule->OriReferenceFrame();
        if (m_isPause)
            m_saveModule->SaveFrame(final, ref_o, oriFrame, warp3d, half, full, m_preds, m_cost, t, pnts, "debug");
        else
            m_saveModule->SaveFrame(final, ref_o, oriFrame, warp3d, half, full, m_preds, m_cost, t, pnts, "print");
        ImShowRatio(m_mainName, disp); //
        //m_mainWin.ShowImage(disp);
        waitKey(waitTime);
        printf("<<<SAVE FRAME>>>\n\n");
        m_predictModule->Reset();
    } else {
        Scalar c(0, 0, 255*(1-r)) ;
        circle(disp, Point(disp.cols-15, disp.rows-15), 10, c, -1, CV_AA);
        ImShowRatio(m_mainName, disp);
        //m_mainWin.ShowImage(disp);
    }

    Mat oriDisp = _frame.clone();
    DrawFPS(oriDisp);
    circle(oriDisp, Point(oriDisp.cols-15, oriDisp.rows-15), 10, Scalar(0, 0, 0), -1, CV_AA);
    ImShowRatio(m_oriName, oriDisp);
    m_nFrames++;
}

void CMirrorMain::DrawFPS( Mat& _disp ) { // right-top corner
    int fps = (int)m_frameRate->getFrameRate();
    char fpsC[255];
    sprintf(fpsC, "%d", fps);
    //itoa((int)fps, fpsC, 10);
    string fpsSt("fps:");
    fpsSt += fpsC;
    cv::putText(_disp, fpsSt, cv::Point(_disp.cols-60,20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0,255,0), 2);
}


void CMirrorMain::ImShowRatio( string _name, const Mat& _im ) {
    double ratio = m_ratio;
    if (_name == "Pause") {
        ratio *= 1.5;
    }
    Mat disp;
    resize(_im, disp, Size(), ratio, ratio);
    imshow(_name, disp);
}

double CMirrorMain::ReadRatio() {
    ifstream fin("ratio.txt");
    if (fin == NULL) {
        SaveRatio(1.0);
        return 1.0;
    } else {
        double ratio;
        fin >> ratio;
        return ratio;
        fin.close();
    }
}

void CMirrorMain::SaveRatio( double _r ) {
    ofstream fout("ratio.txt");
    if (fout != NULL) {
        fout << _r;
        fout.close();
    }
}

