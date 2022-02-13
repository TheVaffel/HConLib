#include <HGraf/improc_ssim.hpp>

#include <HGraf/improc_filtering.hpp>

#include <cmath>
#include <iostream>

namespace hg {
    namespace {
        constexpr int WINDOW_SIZE = 11;

        const auto gaussian = ImageFilter<WINDOW_SIZE, float>::gaussianApprox();

        int signum(float val) {
            return (0 < val) - (val < 0);
        }

        std::tuple<float, float>
        computeGaussianMeans(const Image<float>& im0,
                             const Image<float>& im1,
                             int startx, int starty) {

            float mean0 = 0, mean1 = 0;
            for (int i = 0; i < WINDOW_SIZE; i++) {
                for (int j = 0; j < WINDOW_SIZE; j++) {
                    mean0 += gaussian[i][j] * im0.getPixel(startx + j, starty + i);
                    mean1 += gaussian[i][j] * im1.getPixel(startx + j, starty + i);
                }
            }

            return { mean0, mean1 };
        }

        std::tuple<float, float, float>
        computeGaussianVarsAndCovar(const Image<float>& im0,
                                    const Image<float>& im1,
                                    const std::pair<int, int>& spos,
                                    const std::pair<float, float>& means) {
            float variance0 = 0;
            float variance1 = 0;
            float covariance = 0;

            for (int i = 0; i < WINDOW_SIZE; i++) {
                for (int j = 0; j < WINDOW_SIZE; j++) {
                    float diff_im0 = im0.getPixel(spos.first + j, spos.second + i) - means.first;
                    float diff_im1 = im1.getPixel(spos.first + j, spos.second + i) - means.second;

                    variance0 += gaussian[i][j] * diff_im0 * diff_im0;
                    variance1 += gaussian[i][j] * diff_im1 * diff_im1;
                    covariance += gaussian[i][j] * diff_im0 * diff_im1;
                }
            }

            return { variance0, variance1, covariance };
        }

        float computeLocalSSIMFromMetrics(const std::pair<float, float>& means,
                                          const std::pair<float, float>& variances,
                                          float covariance) {
            float K1 = 0.01f;
            float K2 = 0.03f;
            float L = 1.0f;
            float C1 = (K1 * L) * (K1 * L);
            float C2 = (K2 * L) * (K2 * L);

            float mp = means.first * means.second;
            float msq = means.first * means.first + means.second * means.second;
            float stdsum = variances.first + variances.second;

            float ssim = ((2 * mp + C1) * (2 * covariance + C2 * signum(covariance))) /
                ((msq + C1) * (stdsum + C2));

            return ssim;
        }

        float computeLocalSSIM(const Image<float>& im0,
                               const Image<float>& im1,
                               const std::pair<int, int>& start_pos) {

            auto [ mean_im0, mean_im1 ] = computeGaussianMeans(im0, im1, start_pos.first, start_pos.second);

            auto [ varsum_im0, varsum_im1, covsum ] = computeGaussianVarsAndCovar(im0, im1, { start_pos.first, start_pos.second }, { mean_im0, mean_im1 });

            float ssim = computeLocalSSIMFromMetrics({ mean_im0, mean_im1 }, { varsum_im0, varsum_im1 }, covsum);

            return ssim;
        }

        void assertImagesSameSize(const Image<float>& im0,
                                  const Image<float>& im1) {
            fl_assert_eq(im0.getWidth(), im1.getWidth());
            fl_assert_eq(im0.getHeight(), im1.getHeight());
        }

        std::pair<int, int> getNumWindowsXY(const Image<float>& im0) {
            return {
                im0.getWidth() - WINDOW_SIZE + 1,
                im0.getHeight() - WINDOW_SIZE + 1
            };

        }

        float computeTotalSSIMOnWindows(const Image<float>& im0,
                                        const Image<float>& im1,
                                        const std::pair<int, int> numWindowsXY) {
            float ssim_sum = 0;

            for (int sy = 0; sy < numWindowsXY.second; sy++) {
                for (int sx = 0; sx < numWindowsXY.first; sx++) {
                    float ssim = computeLocalSSIM(im0, im1, { sx, sy });
                    ssim_sum += ssim;
                }
            }

            return ssim_sum;
        }

    };


    float computeSSIM(const Image<float>& im0, const Image<float>& im1) {
        assertImagesSameSize(im0, im1);

        auto [ numWindowsX, numWindowsY ] = getNumWindowsXY(im0);

        int numWindowsTotal = numWindowsX * numWindowsY;

        float ssim_sum = computeTotalSSIMOnWindows(im0, im1, { numWindowsX, numWindowsY });

        float ssim_average = ssim_sum / numWindowsTotal;

        return ssim_average;
    }
};
