#include "common.h"
#include "LaplacianBlend.h"

CLaplacianBlend::CLaplacianBlend(const Mat& _im1, const Mat& _im2, const Mat& _mask) {
    m_im1 = _im1;
    m_im2 = _im2;
    m_mask = _mask;
    m_levels = 4;
    #pragma omp parallel sections
    {
        #pragma omp section
        {
        buildLaplacianPyramid(m_im1,m_lapPyr1,m_residual1);
        }

        #pragma omp section
        {
            buildLaplacianPyramid(m_im2,m_lapPyr2,m_residual2);
        }
    }

    m_maskPyr.clear();
    Mat im;
    cvtColor(m_mask, im, CV_GRAY2BGR);
    m_maskPyr.push_back(im); //highest level

    im = m_mask;
    for (int i=1; i<m_levels+1; i++) {
        Mat mask;
        if ((int)m_lapPyr1.size() > i)
            pyrDown(im, mask, m_lapPyr1[i].size());
        else
            pyrDown(im, mask, m_residual1.size());

        Mat mask3;
        cvtColor(mask, mask3, CV_GRAY2BGR);
        m_maskPyr.push_back(mask3);
        im = mask;
    }

    m_residualRst = m_residual1.mul(m_maskPyr.back()) +
        m_residual2.mul(Scalar(1.0,1.0,1.0) - m_maskPyr.back());
    m_lapPyrRst.resize(m_levels);
    #pragma omp parallel for
    FOR (i, m_levels) {
        Mat im1 = m_lapPyr1[i].mul(m_maskPyr[i]);
        Mat m = Scalar(1.0,1.0,1.0) - m_maskPyr[i];
        Mat im2 = m_lapPyr2[i].mul(m);
        m_lapPyrRst[i] = im1 + im2;
    }
}

Mat CLaplacianBlend::LaplacianBlend() {
    Mat im = m_residualRst;
    for (int i = m_levels-1; i>=0; i--) {
        Mat up;
        pyrUp(im, up, m_lapPyrRst[i].size());
        im = up + m_lapPyrRst[i];
    }
    return im;
}


void CLaplacianBlend::buildLaplacianPyramid(const Mat& _im, vector<Mat>& _pyr, Mat& residual) {
    _pyr.clear();
    Mat im = _im;
    for (int l=0; l<m_levels; l++) {
        Mat down,up;
        pyrDown(im, down);
        pyrUp(down, up, im.size());
        Mat lap = im - up;
        _pyr.push_back(lap);
        im = down;
    }
    im.copyTo(residual);
}




