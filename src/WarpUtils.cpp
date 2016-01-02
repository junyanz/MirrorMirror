#include "WarpUtils.h"

PointSetd CWarpUtils::Mat2Pnts( Mat _matPnts ) {
    int nLandmarks = _matPnts.rows * _matPnts.cols/2;
    PointSetd pnts(nLandmarks);
    FOR (i, nLandmarks)
            pnts[i] = Point2d(_matPnts.at<double>(i), _matPnts.at<double>(i+nLandmarks));
    return pnts;
}

PointSetd CWarpUtils::Mat2Pnts2( Mat _matPnts ) {
    int nLandmarks = _matPnts.rows * _matPnts.cols/2;
    PointSetd pnts(nLandmarks);
    FOR (i, nLandmarks)
            pnts[i] = Point2d(_matPnts.at<float>(0,i), _matPnts.at<float>(1,i));//_matPnts.at<double>(i+nLandmarks));
    return pnts;
}


Mat CWarpUtils::Pnts2Mat( const PointSetd& _pnts ) {
    int nLandmarks = (int)_pnts.size();
    Mat landmarks(Size(nLandmarks*2, 1), CV_64FC1);
    FOR (i, nLandmarks) {
        landmarks.at<double>(i) = _pnts[i].x;
        landmarks.at<double>(i+nLandmarks) = _pnts[i].y;
    }
    return landmarks;
}

PointSetd CWarpUtils::MedianPnts(const vector<PointSetd>& _allPnts ) {
    if (_allPnts.empty())
        return PointSetd();
    int nData = _allPnts.size();
    int nPnts = _allPnts[0].size();
    PointSetd medianPnts(nPnts);
    FOR (i, nPnts) {
        vectord x(nData, 0.0);
        vectord y(nData, 0.0);
        FOR (n, nData) {
            x[n] = _allPnts[n][i].x;
            y[n] = _allPnts[n][i].y;
        }
        sort(x.begin(), x.end());
        sort(y.begin(), y.end());
        if (nData % 2 == 0)
            medianPnts[i] = Point2d(0.5f*(x[nData/2-1]+x[nData/2]), 0.5f*(y[nData/2-1]+y[nData/2]));
        else
            medianPnts[i] = Point2d(x[nData/2], y[nData/2]);
    }

    return medianPnts;
}

Rect CWarpUtils::GetRect( Mat _pnts, int _border ) {
    int min_x, min_y, max_x, max_y;
    min_x = 10000;
    min_y = 10000;
    max_x = 0;
    max_y = 0;

    FOR (i, _pnts.cols) {
        min_x = MIN(min_x, (int)_pnts.at<double>(0, i));
        min_y = MIN(min_y, (int)_pnts.at<double>(1, i));
        max_x = MAX(max_x, (int)_pnts.at<double>(0, i));
        max_y = MAX(max_y, (int)_pnts.at<double>(1, i));
    }
    // add border
    min_x = min_x - _border;
    min_y = min_y - _border;
    max_x = max_x + _border;
    max_y = max_y + _border;
    return Rect(min_x, min_y, max_x-min_x+1, max_y-min_y+1);
}

Rect CWarpUtils::GetRect( PointSetd _pnts, int _border ) {
    int min_x, min_y, max_x, max_y;
    min_x = 10000;
    min_y = 10000;
    max_x = 0;
    max_y = 0;

    FOR_u (i, _pnts.size()) {
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
    return Rect(min_x, min_y, max_x-min_x+1, max_y-min_y+1);
}

double CWarpUtils::PntDist( Point2d _p1, Point2d _p2 ) {
    return sqrt(SQUARE(_p1.x-_p2.x)+SQUARE(_p1.y-_p2.y));
}

PointSetd CWarpUtils::ProjPnts( const PointSetd _p, const Mat& _H ) {
    int nLandmarks = (int)_p.size();
    Mat pnts_3d = Mat::zeros(Size(nLandmarks, 3), CV_64FC1);

    FOR (i, nLandmarks) {
        pnts_3d.at<double>(0, i) = _p[i].x; //.at<double>(i);
        pnts_3d.at<double>(1, i) = _p[i].y; //_srcPnts.at<double>(i+nLandmarks);
        pnts_3d.at<double>(2, i) = 1.0;
    }

    Mat pnts_proj = _H * pnts_3d;
    PointSetd projPnts(nLandmarks);

    FOR (i, nLandmarks) {
        double x = pnts_proj.at<double>(0, i);
        double y = pnts_proj.at<double>(1, i);
        projPnts[i] = Point2d(x, y);
    }
    return projPnts;
}

Mat CWarpUtils::CropFrame( const Mat& _img, PointSetd _pnts, vectori _border) {
    int min_x, min_y, max_x, max_y;
    min_x = 10000;
    min_y = 10000;
    max_x = 0;
    max_y = 0;

    FOR_u (i, _pnts.size()){
        min_x = MIN(min_x, (int)_pnts[i].x);
        min_y = MIN(min_y, (int)_pnts[i].y);
        max_x = MAX(max_x, (int)_pnts[i].x);
        max_y = MAX(max_y, (int)_pnts[i].y);
    }
    // add border
    min_x = min_x - _border[0];
    min_y = min_y - _border[1];
    max_x = max_x + _border[2];
    max_y = max_y + _border[3];
    Rect rect = Rect(min_x, min_y, max_x-min_x+1, max_y-min_y+1);
    return _img(rect).clone();
}

Mat CWarpUtils::CropFrame( const Mat& _img, PointSetd _pnts, int _border) {
    int min_x, min_y, max_x, max_y;
    min_x = 10000;
    min_y = 10000;
    max_x = 0;
    max_y = 0;

    FOR_u (i, _pnts.size()){
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
    Rect rect = Rect(min_x, min_y, max_x-min_x+1, max_y-min_y+1);
    return _img(rect).clone();
}

Mat CWarpUtils::CropFace( const Mat& _img, PointSetd _pnts, vectord _ratios) {
    /*Rect roi_e = FaceROI(_pnts, _ratios);
    Mat crop = _img(roi_e).clone();
    return crop;*/
    Rect roi = RecfityROI(FaceROI(_pnts, _ratios), _img.size());
    // extend image
    //Mat crop = _img(roi_e).clone();
    Mat crop;
    float ratio = 1.5;
    int w = _img.cols;
    int h = _img.rows;
    int w_e = roundInt(w * ratio);
    int h_e = roundInt(h * ratio);
    int bw = (w_e-w)/2;
    int bh = (h_e-h)/2;
    Mat img_e = Mat::zeros(Size(w_e, h_e), CV_8UC3);
    //Rect roi()
    _img.copyTo(img_e(Rect(bw-1, bh-1, w, h)));
    crop = img_e(Rect(roi.x+bw-1, roi.y+bh-1, roi.width, roi.height)).clone();
    return crop;
}

Rect CWarpUtils::FaceROI( PointSetd _pnts, vectord _ratios) {
    //vectord ratios(4, 0.0);
    if (_ratios.empty()) {
        _ratios.assign(4, 0.0);
        _ratios[0] = 0.30; //5 * _scale; // x1
        _ratios[1] = 0.65;//* _scale; // y1
        _ratios[2] = 0.30;// * _scale; // x2
        _ratios[3] = 0.05;// * _scale; // y2
    }
    Rect roi = GetRect(_pnts);
    Rect roi_e;
    roi_e.x = roundInt(roi.x - roi.width*_ratios[0]);
    roi_e.y = roundInt(roi.y - roi.height*_ratios[1]);
    roi_e.width = roundInt(roi.width * (1+_ratios[0]+_ratios[2]));
    roi_e.height = roundInt(roi.height * (1+_ratios[1]+_ratios[3]));
    return roi_e;
}

void CWarpUtils::EstimateHWeighted(const PointSetd a, const PointSetd b, const vectord _w, CvMat* M, int full_affine ) {
    int count = (int)a.size();
    if( full_affine )
    {
        double sa[36], sb[6];
        CvMat A = cvMat( 6, 6, CV_64F, sa ), B = cvMat( 6, 1, CV_64F, sb );
        CvMat MM = cvMat( 6, 1, CV_64F, M->data.db );

        int i;

        memset( sa, 0, sizeof(sa) );
        memset( sb, 0, sizeof(sb) );

        for( i = 0; i < count; i++ )
        {
            sa[0] += a[i].x*a[i].x*_w[i];
            sa[1] += a[i].y*a[i].x*_w[i];
            sa[2] += a[i].x*_w[i];

            sa[6] += a[i].x*a[i].y*_w[i];
            sa[7] += a[i].y*a[i].y*_w[i];
            sa[8] += a[i].y*_w[i];

            sa[12] += a[i].x*_w[i];
            sa[13] += a[i].y*_w[i];
            sa[14] += 1*_w[i];

            sb[0] += a[i].x*b[i].x*_w[i];
            sb[1] += a[i].y*b[i].x*_w[i];
            sb[2] += b[i].x*_w[i];
            sb[3] += a[i].x*b[i].y*_w[i];
            sb[4] += a[i].y*b[i].y*_w[i];
            sb[5] += b[i].y*_w[i];
        }

        sa[21] = sa[0];
        sa[22] = sa[1];
        sa[23] = sa[2];
        sa[27] = sa[6];
        sa[28] = sa[7];
        sa[29] = sa[8];
        sa[33] = sa[12];
        sa[34] = sa[13];
        sa[35] = sa[14];

        cvSolve( &A, &B, &MM, CV_SVD );
    }
    else
    {
        double sa[16], sb[4], m[4], *om = M->data.db;
        CvMat A = cvMat( 4, 4, CV_64F, sa ), B = cvMat( 4, 1, CV_64F, sb );
        CvMat MM = cvMat( 4, 1, CV_64F, m );

        int i;

        memset( sa, 0, sizeof(sa) );
        memset( sb, 0, sizeof(sb) );

        for( i = 0; i < count; i++ )
        {
            sa[0] += a[i].x*a[i].x*_w[i] + a[i].y*a[i].y*_w[i];
            sa[1] += 0;
            sa[2] += a[i].x*_w[i];
            sa[3] += a[i].y*_w[i];

            sa[4] += 0;
            sa[5] += a[i].x*a[i].x*_w[i] + a[i].y*a[i].y*_w[i];
            sa[6] += -a[i].y*_w[i];
            sa[7] += a[i].x*_w[i];

            sa[8] += a[i].x*_w[i];
            sa[9] += -a[i].y*_w[i];
            sa[10] += 1*_w[i];
            sa[11] += 0;

            sa[12] += a[i].y*_w[i];
            sa[13] += a[i].x*_w[i];
            sa[14] += 0;
            sa[15] += 1*_w[i];

            sb[0] += a[i].x*b[i].x*_w[i] + a[i].y*b[i].y*_w[i];
            sb[1] += a[i].x*b[i].y*_w[i] - a[i].y*b[i].x*_w[i];
            sb[2] += b[i].x*_w[i];
            sb[3] += b[i].y*_w[i];
        }

        cvSolve( &A, &B, &MM, CV_SVD );

        om[0] = om[4] = m[0];
        om[1] = -m[1];
        om[3] = m[1];
        om[2] = m[2];
        om[5] = m[3];
    }
}

Mat CWarpUtils::EstimateHWeighted( PointSetd _pnt1, PointSetd _pnt2, bool _flag ) {
    double m[6]={0};
    CvMat M = cvMat( 2, 3, CV_64F, m);
    vectord weights(66, 1);

    for (int i = 48; i <= 65; i++)
        weights[i] = 0;
    weights[39] = 10; weights[42] = 10; weights[27] = 7;

    weights[36] = 7; weights[45] = 7;  weights[30] = 2;  weights[33] = 2; weights[48] = 4; weights[54] = 4;
    weights[31] = 7; weights[35] = 7; weights[17] = 4; weights[21] = 5; weights[22] = 5; weights[26] = 4;
    EstimateHWeighted(_pnt1, _pnt2, weights, &M, _flag);
    Mat H = cvarrToMat(&M).clone();
    return H;

}

Mat CWarpUtils::LoadMeanPnts3D( string _path ) {
    ifstream fin(_path.c_str());
    if (fin != NULL) {
        Mat landmark = Mat::zeros(Size(3, 9), CV_64FC1);
        FOR (i, 9) {
            double x, y, z; //= 0;
            fin >> x >> y >> z;
            landmark.at<double>(i, 0) = x;
            landmark.at<double>(i, 1) = y;
            landmark.at<double>(i, 2) = z;
        }
        fin.close();
        return landmark;
    } else {
        DEBUG_ERROR("cannot load mean pnts from (%s)", _path.c_str());
        return Mat();
    }
}

Rect CWarpUtils::RecfityROI( Rect _roi, Size _imgSz ) {
    Rect out;
    int x2, y2;
    //out.x = max(0, _roi.x);
    //out.y = max(0, _roi.y);
    x2 = min(_imgSz.width-1, _roi.x + _roi.width-1);
    y2 = min(_imgSz.height-1, _roi.y+_roi.height-1);
    out.x = max(0, _roi.x);
    out.y = max(0, _roi.y);
    out.width = x2 - out.x  + 1;
    out.height = y2 - out.y + 1;
    //out.x = max(0, )
    //out.width = _roi.width;
    //out.height = _roi.height;
    //out.x = max(0, x2 - _roi.width+1);
    //out.y = max(0, y2 - _roi.height+1);
    return out;
}

Mat CWarpUtils::LoadZ(string _zFile) {
    int width = 174;
    int height = 224;
    Mat Z = Mat(Size(width, height), CV_64FC1);

    ifstream fin(_zFile.c_str());
    if (fin == NULL)
        DEBUG_ERROR("cannot not load depth image (%s)", _zFile.c_str());
    else {
        FOR (h, height) {
            FOR (w, width) {
                double z;
                fin >> z;
                Z.at<double>(h,w) = z;
                /*if (z > 0)
                    m_mask.at<uchar>(h,w)=255;*/
            }
        }
        fin.close();
    }
    return Z;
}
