#include "error_correction.h"

#if MEASURE_ERROR_CORRECTION < 1

float correct_voltage(float input_v) {
    return input_v;
}

#else

// {observed (what device measures), expected (what multimeter shows)}
// ascending order, at least two rows
static const float observed[][2] = {
    {0.0, 0.0},
    {0.0035, 0.057},
    {0.008, 0.061},
    {0.019, 0.072},
    {0.021, 0.074},
    {0.034, 0.087},
    {0.060, 0.111},
    {0.086, 0.139},
    {0.110, 0.162},
    {0.192, 0.245},
    {0.369, 0.421},
    {0.445, 0.498},
    {0.547, 0.600},
    {1.013, 1.064},
    {1.847, 1.894},
    {2.751, 2.790},
    {3.012, 3.050},
    {3.461, 3.580},
    {4.836, 4.940},
    {4.884, 5.000},
    {6.261, 6.360},
    {7.899, 7.980},
    {9.157, 9.23}
};

static const int observed_size = sizeof(observed) / sizeof(observed[0]);

// https://en.wikipedia.org/wiki/Linear_interpolation
inline float lin_interpolation(float x0, float y0, float x1, float y1, float x) {
    return (y0 * (x1 - x) + y1 * (x - x0)) / (x1 - x0);
}

int find_points(float *x0, float *y0, float *x1, float *y1, float x) {
    // linear algorithm, in case of performance problems rewrite to binary search
    for (int i = 0; i < observed_size - 1; i++) {
        if (x >= observed[i][0] && x < observed[i + 1][0]) {
            *x0 = observed[i][0]; *y0 = observed[i][1];
            *x1 = observed[i + 1][0]; *y1 = observed[i + 1][1];
            return 1;
        }
    }

    return 0;
}

float correct_voltage(float input_v) {
    float x0, y0, x1, y1;
    if (find_points(&x0, &y0, &x1, &y1, input_v)) {
        return lin_interpolation(x0, y0, x1, y1, input_v);
    }

    // linear extrapolation is not supported yet
    return input_v;
}

#endif
