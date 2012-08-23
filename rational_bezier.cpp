#include <cassert>
#include <cstdio>
#include <vector>

#include "pnt.h"
#include "rational_bezier.h"

using std::vector;

static void interpolate(pnt &a, const pnt &b, double t) {
    assert(0.0 <= t && t <= 1.0);

    double tmp = (1 - t) * a.w + t * b.w;
    a = a * ((1 - t) * a.w / tmp) + b * ((t * b.w) / tmp);
    a.w = tmp;
}

// Using de Casteljau's algorithm computes a point on the rational bezier
// curve for a given parameter value t
//
// controls - coordinates and weights of the control points
// t        - evaluation parameter
static pnt curve_point(vector<pnt> controls, double t) {
    vector<pnt> v = controls;

    int n = v.size() - 1;
    for (int i = 0; i < n; ++ i) {
        for (int j = 0; j < n - i; ++ j) {
            interpolate(v[j], v[j + 1], t);
        }
    }
    return v[0];
}

vector<pnt> rational_bezier(vector<pnt> controls) {
    vector<pnt> res;
    if (controls.size() > 1) {
        // number of line segments that comprize the Bezier curve
        const static int N = 1000;
        double step = 1.0 / N;
        double t = 0.0;
        for (int i = 1; i < N; ++ i) {
            t += step;
            res.push_back(curve_point(controls, t));
        }
    }
    return res;
}

