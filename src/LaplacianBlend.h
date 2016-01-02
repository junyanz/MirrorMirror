#ifndef CLAPLACIAN_BLEND_
#define CLAPLACIAN_BLEND_
#include "common.h"

class CLaplacianBlend {
public:
    CLaplacianBlend(const Mat& _im1, const Mat& _im2, const Mat& _mask);
    Mat LaplacianBlend();
private:
    void buildGaussianPyramid();
    void buildLaplacianPyramid(const Mat& img, vector<Mat>& lapPyr, Mat& smallestLevel);

private:
    Mat m_im1;
    Mat m_im2;
    Mat m_mask;
    Mat m_residual1;
    Mat m_residual2;
    Mat m_residualRst;
    ImageSet m_lapPyr1;
    ImageSet m_lapPyr2;
    ImageSet m_lapPyrRst;

    ImageSet m_maskPyr;

    int m_levels;

};

#endif //CLAPLACIAN_BLEND_
