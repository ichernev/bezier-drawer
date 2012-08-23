#include <cassert>
#include <cstdio>
#include <vector>

using std::vector;

struct Point {
  double x, y, z;
  Point(): x(0), y(0), z(0) {}
  Point(double x, double y, double z): x(x), y(y), z(z) {}

  Point operator* (double weight) const {
    return Point(x * weight, y * weight, z * weight);
  }
  //Point operator*=

  Point operator+ (const Point& rhs) const {
    return Point(x + rhs.x, y + rhs.y, z + rhs.z);
  }

  void print() const {
    printf("(%lf, %lf, %lf)\n", x, y, z);
  }
};

Point interpolate(const Point& a, const Point& b, double t) {
  assert(0.0 <= t && t <= 1.0);
  return a * (1 - t) + b * t;
}

double interpolate(double a, double b, double t) {
  assert(0.0 <= t && t <= 1.0);
  return a * (1 - t) + b * t;
}

// Using de Casteljau's algorithm computes a point on the rational bezier
// curve for parameter value t
//
// note: the sizes of the following two vectors must be equal
// controls - coordinates of the control points
// weights  - weights of the control points
// t        - evaluation parameter
Point rational_bezier(vector<Point> controls, vector<double> weights, double t) {
  assert(v.size == w.size());
  vector<Point> v = controls;
  vector<double> w = weights;
  int n = v.size() - 1;
  for (int i = 0; i < n; ++ i) {
    for (int j = 0; j < n - i; ++ j) {
      double tmp = interpolate(w[j], w[j + 1], t);
      v[j] = interpolate(v[j] * (w[j] / tmp), v[j + 1] * (w[j + 1] / tmp), t);
      w[j] = tmp;
    }
  }
  return v[0];
}

int main() {
  int n;
  double t;
  scanf("%d %lf ", &n, &t);
  Point p;
  vector<Point> v;
  vector<double> w;
  double weight;
  for (int i = 0; i < n; ++ i) {
    scanf("%lf %lf %lf %lf", &p.x, &p.y, &p.z, &weight);
    v.push_back(p);
    w.push_back(weight);
  }
  printf("DC: ");
  rational_bezier(v, w, t).print();

  return 0;
}
