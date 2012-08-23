#ifndef __PNT_H__e8fff380_96e5_46b1_8212_4001e17f094a
#define __PNT_H__e8fff380_96e5_46b1_8212_4001e17f094a

struct pnt {
    pnt(): w(1.0) {}
    pnt(double x_, double y_);

    void draw() const;

    double len2(const pnt &other) const;
    double len(const pnt &other) const;

    bool inRect(const pnt &lb, const pnt &ub) const;

    pnt &operator += (const pnt &other);
    const pnt operator + (const pnt &other) const;

    pnt bound(const pnt &lb, const pnt &ub) const;

    const pnt operator * (double weight) const;

    // coordinates
    double x, y;
    // weight
    double w;
};

#endif
