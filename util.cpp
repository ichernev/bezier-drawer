#include "util.h"

double sq(double a) {
    return a * a;
}

double bound(double val, double lb, double ub) {
    if (val < lb) return lb;
    if (val > ub) return ub;
    return val;
}
