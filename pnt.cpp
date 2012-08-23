#include "pnt.h"

#include <math.h>
#include <GL/gl.h>
#include "util.h"

pnt::pnt(double x, double y) : x(x), y(y), w(1.0) {}

pnt &pnt::operator += (const pnt &other) {
    x += other.x;
    y += other.y;
    return *this;
}

const pnt pnt::operator + (const pnt &other) const {
    pnt res = *this;
    res += other;
    return res;
}

void pnt::draw() const {
    glVertex2f(x, y);
}

bool pnt::inRect(const pnt &lb, const pnt &ub) const {
    return lb.x <= x && x <= ub.x &&
           lb.y <= y && y <= ub.y;
}

pnt pnt::bound(const pnt &lb, const pnt &ub) const {
    return pnt(::bound(x, lb.x, ub.x), ::bound(y, lb.y, ub.y));
}

double pnt::len2(const pnt &other) const {
    return sq(x - other.x) + sq(y - other.y);
}

double pnt::len(const pnt &other) const {
    double l2 = len2(other);
    return l2 <= 0 ? 0 : sqrt(l2);
}

const pnt pnt::operator*(double weight) const {
    return pnt(x * weight, y * weight);
}
