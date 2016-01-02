#include "FadeModule.h"
#include "Timer.h"
#include "ImageTools.h"
#include "WarpingModule.h"
#include "BlendModule.h"
#include "WarpUtils.h"

const int NUM_BINS = 256; 
const int NUM_FRAMES_COLOR = 30;
const double MAX_ERROR = 2.5f;
#ifdef __linux__
#include<X11/Xlib.h>
#include<GL/glx.h>
Display                 *dpy;
Window                  root;
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
XWindowAttributes       gwa;
XEvent                  xev;


void DrawAQuad() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1., 1., -1., 1., 1., 20.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);

    glBegin(GL_QUADS);
    glColor3f(1., 0., 0.); glVertex3f(-.75, -.75, 0.);
    glColor3f(0., 1., 0.); glVertex3f( .75, -.75, 0.);
    glColor3f(0., 0., 1.); glVertex3f( .75,  .75, 0.);
    glColor3f(1., 1., 0.); glVertex3f(-.75,  .75, 0.);
    glEnd();
}

void EnableOpenGL() {
    dpy = XOpenDisplay(NULL);

    if(dpy == NULL)
        DEBUG_ERROR("cannot connect to X server");

    root = DefaultRootWindow(dpy);

    vi = glXChooseVisual(dpy, 0, att);

    if(vi == NULL)
        DEBUG_ERROR("no appropriate visual found");
    else
        DEBUG_INFO("visual %p selected", (void *)vi->visualid); /* %p creates hexadecimal output like in glxinfo */

    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;

    win = XCreateWindow(dpy, root, 0, 0, 224, 224, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

//    XMapWindow(dpy, win);
    XStoreName(dpy, win, "3D Face");

    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
    glEnable(GL_DEPTH_TEST);
}

#endif

#ifdef _WIN32
#include <windows.h>
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {

    case WM_CREATE:
        return 0;

    case WM_CLOSE:
        PostQuitMessage( 0 );
        return 0;

    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
        switch ( wParam )
        {

        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;

        }
        return 0;

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );

    }

}

// Enable OpenGL

void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int format;

    // get the device context (DC)
    *hDC = GetDC( hWnd );

    // set the pixel format for the DC
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    format = ChoosePixelFormat( *hDC, &pfd );
    SetPixelFormat( *hDC, format, &pfd );

    // create and enable the render context (RC)
    *hRC = wglCreateContext( *hDC );
    wglMakeCurrent( *hDC, *hRC );

}

// Disable OpenGL

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC) {
    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( hWnd, hDC );
}

#endif

CFadeModule::CFadeModule() {
    DEBUG_INFO("Loading CFadeModule...");
    Clear();
    Reset();
    m_isDebug = false;
    m_isFirst = true;
    m_isInitList = false;
    m_warpModule = new CWarpingModule();
    m_Z  = CWarpUtils::LoadZ("models/Z.img");
    LoadContours();
    m_pnts3d = CWarpUtils::LoadMeanPnts3D("models/3d.txt");
}


void CFadeModule::Reset() {
    m_nFrames = 0;
    m_Hs.clear();
    FOR_u (i, m_srcCDF.size())
            m_srcCDF[i].clear();
    FOR_u (i, m_tgtCDF.size())
            m_tgtCDF[i].clear();
    m_srcCDF.clear();
    m_tgtCDF.clear();
    m_isInitList = false;
    m_error = -1;
}


void CFadeModule::Clear() {
    m_alpha = 0.0f;
    m_gridSz = 0;
    m_transRatio = 0;
    m_nFrames = 0;
}


CFadeModule::~CFadeModule() {
    Clear();
}


Mat CFadeModule::DrawPoints( const Mat& _img, const PointSetd& _p, const PointSetd& _q ) {
    Mat showImg = _img.clone();
    FOR_u (i, _p.size())
            cv::circle(showImg, _p[i], 2, Scalar(0, 0, 255), 2); // original

    FOR_u (i, _q.size())
            cv::circle(showImg, _q[i], 2, Scalar(0, 255, 0), 2); // projection
    return showImg;

}


Rect LocalizeParts( PointSeti _pnts, int _border) {
    int min_x = 10000;
    int min_y = 10000;
    int max_x = 0;
    int max_y = 0;

    FOR (j, (int)_pnts.size()) {
        int i = j;
        min_x = MIN(min_x, (int)_pnts[i].x);
        min_y = MIN(min_y, (int)_pnts[i].y);
        max_x = MAX(max_x, (int)_pnts[i].x);
        max_y = MAX(max_y, (int)_pnts[i].y);
    }

    // add border
    min_x = min_x - _border;
    min_y = min_y - _border;
    max_x = max_x + _border;
    max_y = max_y + _border;
    //cout << min_x << " " << min_y << " " << max_x << " " << max_y << endl;
    return Rect(min_x, min_y, max_x-min_x+1, max_y-min_y+1);
}

void CFadeModule::DrawEllipse( const Mat& _img, string _name, RotatedRect _e ) {
    Mat showImg = _img.clone();
    ellipse(showImg, _e, Scalar(0, 255, 0), 3);
    imshow(_name, showImg);
}


Mat  CFadeModule::EstimateHistMatch( const Mat& _src, const Mat& _tgt, const Mat& _mask) {
    //CTimer timer("EstimateHistMatch");
    // match src to tgt
    //Mat src_f, tgt_f;
//    Mat src_hsv, tgt_hsv;

    Mat src = _src.clone();
    Mat tgt = _tgt.clone();
    m_luts.clear();

    ImageSet srcBGR;
    ImageSet tgtBGR;
    split(src, srcBGR);
    split(tgt, tgtBGR);

    int histSz = NUM_BINS;
    float range[] = { 0, (float)256 } ;
    const float* histRange = { range };
    bool uniform = true;
    bool accumulate = false;
    CDF srcCDFs;
    CDF tgtCDFs;

    FOR (chn, 3) {
        Mat srcHist, tgtHist;
        // compute histogram
        //SHOW_IMG(srcBGR[chn]);
        calcHist( &tgtBGR[chn], 1, 0, _mask, tgtHist, 1, &histSz, &histRange, uniform, accumulate );
        calcHist( &srcBGR[chn], 1, 0, _mask, srcHist, 1, &histSz, &histRange, uniform, accumulate );
        // compute CDF
        vectord srcCDF(NUM_BINS, 0.0);
        vectord tgtCDF(NUM_BINS, 0.0);
        srcCDF[0] = (double)srcHist.at<float>(0);
        tgtCDF[0] = (double)tgtHist.at<float>(0);
        for (int n = 1; n < NUM_BINS; n++) {
            srcCDF[n] = srcCDF[n-1]+(double)srcHist.at<float>(n);
            tgtCDF[n] = tgtCDF[n-1]+(double)tgtHist.at<float>(n);
        }

        srcCDFs.push_back(srcCDF);
        tgtCDFs.push_back(tgtCDF);
    }

    CDF srcCDF_mean;
    CDF tgtCDF_mean;
    //DEBUG_INFO("queue size = %d", m_tgtCDF.size());
    if (m_tgtCDF.size() >= NUM_FRAMES_COLOR) {
        FOR_u (i, m_srcCDF[0].size())
                m_srcCDF[0][i].clear();
        FOR_u (i, m_tgtCDF[0].size())
                m_tgtCDF[0][i].clear();
        m_srcCDF[0].clear();
        m_tgtCDF[0].clear();

        m_srcCDF.erase(m_srcCDF.begin());
        m_tgtCDF.erase(m_tgtCDF.begin());
    }

    /*FOR (k, m_tgtCDF.size()) {
        cout << m_tgtCDF[k][0][128] << " ";
    }
    cout << endl; */

    m_srcCDF.push_back(srcCDFs);
    m_tgtCDF.push_back(tgtCDFs);


    FOR (chn, 3) { // compute mean CDF over 30 frames
        vectord srcCDF_mean(NUM_BINS, 0.0);
        vectord tgtCDF_mean(NUM_BINS, 0.0);
        FOR (i, NUM_BINS) {
            FOR_u (k, m_tgtCDF.size()) {
                srcCDF_mean[i] += m_srcCDF[k][chn][i];
                tgtCDF_mean[i] += m_tgtCDF[k][chn][i];
            }
            srcCDF_mean[i] /= (double)m_srcCDF.size();
            tgtCDF_mean[i] /= (double)m_tgtCDF.size();
        }


        Mat Mv(1,NUM_BINS,CV_8UC1);
        uchar* M = Mv.ptr<uchar>();
        //int window = 200;
        FOR (n, NUM_BINS) {
            double cdf = srcCDF_mean[n];
            //histMap[n] = lastK;
            int min_idx = -1;
            double min_dist = 1e10;
            FOR (k, NUM_BINS) {
                double dist = abs(cdf-tgtCDF_mean[k]);
                if (dist <= min_dist) {
                    min_dist = dist;
                    min_idx = k;
                }
            }

            M[n] = (uchar)min_idx;
            //if (chn == 0) {
            //cout << min_idx - n << " ";
            //}
        }

        //if (chn == 0)
        //cout << endl;

        Mat lut(1,NUM_BINS,CV_8UC1,M);
        LUT(srcBGR[chn],lut,srcBGR[chn]);
        //SHOW_IMG(srcBGR[chn]);
    }



    Mat out;
    merge(srcBGR, out);
    //cvtColor(out, out, CV_Luv2BGR);
    //SHOW_IMG(out);
    srcBGR.clear();
    tgtBGR.clear();
    return out;

}

PointSeti CFadeModule::CreatFaceMask( const Mat& _img, const PointSetd& _pnts, Mat& _mask, Rect& _roi) {
    //PointSeti drawP;
    PointSetd drawP_d;

    drawP_d.push_back(m_meanPnts[0]+Point2d(3, 0));
    for (int i = 1; i < 16; i++)
        drawP_d.push_back(m_meanPnts[i]);
    drawP_d.push_back(m_meanPnts[16]-Point2d(3, 0));
    for (int i = 26; i >= 17; i--)
        drawP_d.push_back(m_meanPnts[i]-Point2d(0, 20));

    Mat H = CWarpUtils::EstimateHWeighted(m_meanPnts, _pnts, false);
    //Mat H = CWarpUtils::EstimateH(m_meanPnts, _pnts, false);
    if (H.empty()) {
        DEBUG_INFO("empty H");
        _mask = Mat();
        _roi = Rect(-1, -1, 0, 0);
        return PointSeti();
    }
    PointSetd projP_d = CWarpUtils::ProjPnts(drawP_d, H);
    PointSeti projP;

    FOR_u (i, projP_d.size())
            projP.push_back(Point2i(projP_d[i].x, projP_d[i].y));

    //cout << drawP << endl;
    _roi = CWarpUtils::GetRect(projP_d, 15);
    //cout << _roi << endl;
    _mask = Mat::zeros(_img.size(), CV_8UC1);
    vector<PointSeti> countours;
    countours.push_back(projP);
    drawContours(_mask, countours, 0, Scalar(255), -1);
    int dilate_size = 5;
    Mat element = getStructuringElement( MORPH_CROSS,
                                         Size( 2*dilate_size + 1, 2*dilate_size+1 ),
                                         Point( dilate_size, dilate_size ) );
    dilate(_mask, _mask, element);
    //fillPoly(mask, drawP, Scalar(255), -1);
    //drawP.clear();
    //projP.clear();
    projP_d.clear();
    drawP_d.clear();
    //imwrite("mask.jpg", _mask);
    //SHOW_IMG(_mask);

    //imshow("mask", _mask);
    countours.clear();
    return projP;
}

Mat CFadeModule::ColorMatch( const Mat& _src, const ImageSet& _luts ) {
    ImageSet rgb;
    split(_src, rgb);
    FOR (k, 3)
            LUT(rgb[k], _luts[k], rgb[k]);
    Mat out;
    merge(rgb, out);
    return out;
}

Mat CFadeModule::DrawMask( const Mat& _img, const PointSeti& _maskPnts ) {
    Mat showImg = _img.clone();
    vector<PointSeti> countours;
    countours.push_back(_maskPnts);
    drawContours(showImg, countours, 0, Scalar(0, 255, 0), 2);

    return showImg;
}

double CFadeModule::AlignmentError( const PointSetd& _pnts1, const PointSetd& _pnts2 ) {
    double error = 0.0;
    FOR_u (i, _pnts1.size()) {
        double e = CWarpUtils::PntDist(_pnts1[i], _pnts2[i]);
        error = e * e;
    }

    error /= (double)_pnts1.size();
    error = sqrt(error);

    return error;
}

Mat TranslateImage( const Mat& _img, Point2d _t) { 

    Mat out;
    Mat H = Mat::zeros(2, 3, CV_64FC1);
    H.at<double>(0, 0) = 1.0; H.at<double>(1, 1) = 1.0;
    H.at<double>(0, 2) = _t.x; H.at<double>(1, 2) = _t.y;
    warpAffine(_img, out, H, _img.size());
    return out;
    //}
}
Mat CFadeModule::Fade3D(const Mat& _tgt, PointSetd _srcPnts, PointSetd _tgtPnts, float _r, bool _flag ) {
    vectord weights(_srcPnts.size(), 1.0);
    double m[6]={0};
    CvMat M = cvMat( 2, 3, CV_64F, m);
    CWarpUtils::EstimateHWeighted(_srcPnts, _tgtPnts, weights, &M, true);
    m_H = cvarrToMat(&M).clone();

    if (m_H.empty()) {
        DEBUG_INFO("Fail to fit");
        return Mat();
    }

    PointSetd projPnts = CWarpUtils::ProjPnts(_srcPnts, m_H);
    Rect rect = CWarpUtils::GetRect(_tgtPnts);
    m_error = AlignmentError(projPnts, _tgtPnts)/sqrt(rect.width*rect.height)*25;

    if (m_error >= MAX_ERROR) {
        DEBUG_INFO("large alignment error (%4.4f)", m_error);
        return Mat();
    }

    m_warpModule->WarpToFrontalFace3D(_tgt, _tgtPnts, false);

    Mat ar, H;
    Point2d t;
    m_warpModule->Matrix(ar, H, t);
    invertAffineTransform(H, H);
    Mat warpImg = Mat::zeros(_tgt.size(), _tgt.type());
    Mat face = Render3D(m_Z, m_texture, ar, t);
    if (face.empty())
        return Mat();
    warpAffine(face, warpImg, H, _tgt.size());
    PointSeti contours = Contours(m_Z, ar, t, H);
    Mat warpMask = Mat::zeros(_tgt.size(), CV_8UC1);
    vector<PointSeti> pts;
    pts.push_back(contours);
    drawContours(warpMask, pts, 0, Scalar(255), -1);

    Mat disp;
    Rect roi = Rect(0, 0, warpImg.cols, warpImg.rows);
    Mat color = EstimateHistMatch(warpImg, _tgt, warpMask);
    m_warp3d = warpImg;

    disp = CBlendModule::LaplacianBlend(color, _tgt, warpMask, roi, _r, m_halfBlend, m_fullBlend);
    Mat tgt2 = _tgt.clone();

    if (m_isDebug) {
        Mat dispHalf = m_halfBlend.clone();
        Mat dispFull = m_fullBlend.clone();

        FOR_u (i, 9) {
            circle(tgt2, _tgtPnts[i], 2, Scalar(0, 0, 255), -1);
            circle(dispHalf, _tgtPnts[i], 2, Scalar(0, 0, 255), -1);
            circle(dispFull, _tgtPnts[i], 2, Scalar(0, 0, 255), -1);
        }
        PointSetd prjPnts = ProjPnts(ar, t, H);
        FOR_u (i, 9) {
            circle(tgt2, prjPnts[i], 2, Scalar(0, 255, 0), -1);
            circle(dispHalf, prjPnts[i], 2, Scalar(0, 255, 0), -1);
            circle(dispFull, prjPnts[i], 2, Scalar(0, 255, 0), -1);
        }

        imshow("50% blend", dispHalf);
        imshow("100% blend", dispFull);
        imshow("ori", tgt2);
    }

    return disp;
}


void CFadeModule::Init () {
    DEBUG_INFO("CFaceModule::Init");
    // select projection matrix
    glMatrixMode(GL_PROJECTION);
    // set the viewport
    glViewport(0, 0, 174, 224);
    // set matrix mode
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, 0.1f, 500.0f);
    // specify which matrix is the current matrix
    glMatrixMode(GL_MODELVIEW);
    glShadeModel( GL_SMOOTH );
    // specify the clear value for the depth buffer
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

Mat CFadeModule::DrawFace(Size _sz,const Mat& _Z, const Mat& _texture) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0f );
    glEnable( GL_DEPTH_TEST );
    glCallList(m_list);
    float _size = (float)max(_sz.width, _sz.height);
    unsigned char* buffer = new unsigned char[(int)(_size*_size*3)];
    glReadPixels(0, 0, (int)_size, (int)_size, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    cv::Mat img((int)_size, (int)_size, CV_8UC3, buffer);
    cvtColor(img, img, CV_RGB2BGR);
    //    SHOW_IMG(img);
    flip(img, img, 0);
    Mat final = img(Rect(0, 0, _sz.width, _sz.height));
    //    glutSwapBuffers();

    return final;
}

Mat CFadeModule::Render3D(const Mat& _Z, const Mat& _texture, const Mat& _ar, Point2d _t) {
#ifdef WIN32
    WNDCLASS wc;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;
#endif
    float size = (float)max(_Z.cols, _Z.rows);
    if (m_isFirst) {
        //		win.width = (int)size;//g_Z.cols;
        //		win.height = (int)size;//g_Z.rows;
        //		win.title = "face";
        //        win.z_near = 0.1f;
        //		win.z_far = 500.0f;
        // set up OpenGL context

        // register window class
#ifdef _WIN32
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = NULL;
        wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
        wc.hCursor = LoadCursor( NULL, IDC_ARROW );
        wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
        wc.lpszMenuName = NULL;
        wc.lpszClassName = L"GLSample";
        RegisterClass( &wc );

        // create main window
        hWnd = CreateWindow(
                    L"GLSample", L"OpenGL Sample",
                    WS_POPUPWINDOW,
                    0, 0, (int)size, (int)size,
                    NULL, NULL, NULL, NULL );

        EnableOpenGL( hWnd, &hDC, &hRC );
#endif
#ifdef __linux__
        EnableOpenGL();
#endif
        Init();
        m_isFirst = false;
    }
#ifdef __linux__
    XGetWindowAttributes(dpy, win, &gwa);
    glViewport(0, 0, gwa.width, gwa.height);
#endif
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0,0,100, 0,0,0, 0,1,0);
    glPushMatrix();

    double m[16] = {0};
    FOR (i, 3) FOR (j, 3) m[4*i+j] = _ar.at<double>(j, i);
    m[15]=1.0f;

    glMultMatrixd(m);
    glTranslated(_t.x/size, _t.y/size, 0);
    if (!m_isInitList) {
        m_isInitList = true;
        InitList(_Z, _texture, size);
    }

    Mat face = DrawFace(_Z.size(), _Z, _texture);
    glPopMatrix();
#ifdef WIN32
    SwapBuffers( hDC );
#endif
#ifdef __linux__
    glXSwapBuffers(dpy, win);
#endif
    return face;
}

void CFadeModule::InitList(const Mat& _Z, const Mat& _texture, float _size) {
    //    SHOW_IMG(_Z);
//    DEBUG_INFO(" CFadeModule::InitList");
    m_list = glGenLists(1);
    glNewList(m_list, GL_COMPILE );
    int width = _Z.cols;
    int height = _Z.rows;

    int dx[] = {0, 0, 1, 1};
    int dy[] = {0, 1, 1, 0};

    FOR (w, width-1) {
        FOR (h, height-1) {
            bool flag = true;
            FOR (k, 4) {
                flag = flag && (_Z.at<double>(h+dy[k], w+dx[k])>0.0f);
            }

            if (true) {
                glBegin(GL_QUADS);
                FOR (k, 4) {
                    int hh = h  + dy[k];
                    int ww = w  + dx[k];
                    float r = _texture.at<Vec3b>(hh,ww)[2]/255.0f;
                    float g = _texture.at<Vec3b>(hh,ww)[1]/255.0f;
                    float b = _texture.at<Vec3b>(hh,ww)[0]/255.0f;
                    glColor3f(r,g,b);
//                    glColor3b(1.0, 1.0, 1.0);
                    float x = ww/_size; //2*(-ww+width/2)/_size;//*(174/224.0);
                    float y = hh/_size; //2 * (height/2-hh) / _size;
                    float z = (float)_Z.at<double>(hh,ww)/_size;//103.9104f;
//                                        DEBUG_INFO("(x,y,z) = (%3.3f, %3.3f, %3.3f)", x, y, z);
                    glVertex3f(x,y,z);
                }
                glEnd();
            }
        }
    }

    glEndList();

}



void CFadeModule::LoadContours() {
    string maskpath  = "./models/mask.png";
    Mat mask = imread(maskpath, 0);
    if (mask.empty()) {
        DEBUG_ERROR("cannot load mask image (%s)", maskpath.c_str());
    }
    int dilate_size = 5;
    Mat element = getStructuringElement( MORPH_CROSS,
                                         Size( 2*dilate_size + 1, 2*dilate_size+1 ),
                                         Point( dilate_size, dilate_size ) );
    erode(mask, mask, element);
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // Detect edges using Threshold
    threshold(mask, threshold_output, 128, 255, THRESH_BINARY );
    //Find contours
    findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    PointSeti c = contours[0];
    m_countours.clear();
    FOR_u (i, c.size())
            m_countours.push_back(Point2d(c[i].x, c[i].y));
}

PointSeti CFadeModule::Contours(const Mat& _Z, const Mat& _ar, Point2d _t, const Mat& _H ) {
    PointSetd outP;
    FOR_u (i, m_countours.size()) {
        int w = (int)m_countours[i].x;
        int h = (int)m_countours[i].y;
        Mat pnt3d =  Mat(Size(1, 3), CV_64FC1);
        pnt3d.at<double>(0, 0) = w;// +dx[k];
        pnt3d.at<double>(1, 0) = h;// +dy[k];
        pnt3d.at<double>(2, 0) = m_Z.at<double>(h,w);
        Mat pnt2d = _ar * pnt3d;
        double nx = pnt2d.at<double>(0, 0)+ _t.x;
        double ny = pnt2d.at<double>(1, 0) + _t.y;
        outP.push_back(Point2d(nx, ny));
    }

    PointSetd out = CWarpUtils::ProjPnts(outP, _H);
    PointSeti outi;
    FOR_u (i, out.size())
            outi.push_back(Point2i(roundInt(out[i].x), roundInt(out[i].y)));
    return outi;
}

PointSetd CFadeModule::ProjPnts( const Mat& _ar, Point2d _t, const Mat& _H ) {

    Mat pnt2d = _ar * m_pnts3d.t();
    PointSetd pnts(9);
    FOR (i, 9) {
        pnts[i].x = pnt2d.at<double>(0, i) + _t.x;
        pnts[i].y = pnt2d.at<double>(1, i) + _t.y;
    }

    PointSetd out = CWarpUtils::ProjPnts(pnts, _H);
    return out;
}
