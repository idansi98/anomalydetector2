

#ifndef ANOMALY_DETECTOR_ANOMALY_DETECTION_UTIL_H
#define ANOMALY_DETECTOR_ANOMALY_DETECTION_UTIL_H

// returns the variance of X and Y
float var(const float* x, int size);
// returns the covariance of X and Y
float cov(const float* x, const float* y, int size);
// returns the Pearson correlation coefficient of X and Y
float pearson(const float* x, const float* y, int size);
class Line{
public:
    float a,b;
    Line():a(0),b(0){}
    Line(float a, float b):a(a),b(b){}
    float f(float x){
        return a*x+b;
    }
};
class Point{
public:
    float x,y;
    Point(float x, float y):x(x),y(y){}
};
// performs a linear regression and return s the line equation.
Line linear_reg(Point** points, int size);
// returns the deviation between point p and the line equation of the points.
float dev(Point p,Point** points, int size);
// returns the deviation between point p and the line.
float dev(Point p,Line l);
// returns distance between two points.
float distance(Point a,Point b);
#endif ANOMALY_DETECTOR_ANOMALY_DETECTION_UTIL_H
