#include <HGraf/improc_ssim.hpp>

#include <HGraf/improc_filtering.hpp>

#include <cmath>
#include <iostream>

namespace hg {
    namespace {
        constexpr int WINDOW_SIZE = 11;
    };

    int signum(float val) {
        return (0 < val) - (val < 0);
    }

    float computeSSIM(const Image<float>& im0, const hg::Image<float>& im1) {
        fl_assert_eq(im0.getWidth(), im1.getWidth());
        fl_assert_eq(im0.getHeight(), im1.getHeight());

        auto gaussian = ImageFilter<WINDOW_SIZE, float>::gaussianApprox();

        int endSx = im0.getWidth() - WINDOW_SIZE + 1;
        int endSy = im0.getHeight() - WINDOW_SIZE + 1;

        int numWindows = endSx * endSy;

        float ssim_sum = 0;

        for (int sy = 0; sy < endSy; sy++) {
            for (int sx = 0; sx < endSx; sx++) {

                float mean_im0 = 0, mean_im1 = 0;

                for (int i = 0; i < WINDOW_SIZE; i++) {
                    for (int j = 0; j < WINDOW_SIZE; j++) {
                        mean_im0 += gaussian[i][j] * im0.getPixel(sx + j, sy + i);
                        mean_im1 += gaussian[i][j] * im1.getPixel(sx + j, sy + i);
                    }
                }

                float varsum_im0 = 0;
                float varsum_im1 = 0;
                float covsum = 0;

                for (int i = 0; i < WINDOW_SIZE; i++) {
                    for (int j = 0; j < WINDOW_SIZE; j++) {
                        float diff_im0 = im0.getPixel(sx + j, sy + i) - mean_im0;
                        float diff_im1 = im1.getPixel(sx + j, sy + i) - mean_im1;

                        varsum_im0 += gaussian[i][j] * diff_im0 * diff_im0;
                        varsum_im1 += gaussian[i][j] * diff_im1 * diff_im1;
                        covsum += gaussian[i][j] * diff_im0 * diff_im1;
                    }
                }

                float std_im0 = varsum_im0;
                float std_im1 = varsum_im1;

                float K1 = 0.01f;
                float K2 = 0.03f;
                float L = 1.0f;
                float C1 = (K1 * L) * (K1 * L);
                float C2 = (K2 * L) * (K2 * L);

                float mp = mean_im0 * mean_im1;
                float msq = mean_im0 * mean_im0 + mean_im1 * mean_im1;
                float stdsum = std_im0 + std_im1;

                float ssim = ((2 * mp + C1) * (2 * covsum + C2 * signum(covsum))) /
                    ((msq + C1) * (stdsum + C2));

                ssim_sum += ssim;
            }
        }

        float ssim_average = ssim_sum / numWindows;

        return ssim_average;
    }
};
