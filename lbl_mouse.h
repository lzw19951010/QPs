#ifndef LBL_MOUSE_H
#define LBL_MOUSE_H

#include <QWidget>
#include <QMouseEvent>
#include <QDebug>
#include <QEvent>
#include <QLabel>
#include <QPainter>
#include <QRect>
#include <QPoint>
#include <QPixmap>
#include <QMessageBox>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "opencv2/opencv.hpp"

struct rgb{
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
};

struct hsv {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
};

class lbl_mouse : public QLabel
{
    Q_OBJECT
public:


    explicit lbl_mouse(QWidget *parent = 0);
    void mouseMoveEvent(QMouseEvent *es);
    void mousePressEvent(QMouseEvent *es);
    void mouseReleaseEvent(QMouseEvent *es);
    void paintEvent(QPaintEvent* e);
    QPoint start;
    QPoint end;
    QPoint cur;
    bool cutting;
    QPixmap back;
    int type;
    cv::Mat rawImg;
    cv::Mat processedImg;
    cv::Mat rotate_bilinear(cv::Mat src, double degree);
    cv::Mat horizontalFlip(cv::Mat src, bool horizontal);
    cv::Mat verticalFlip(cv::Mat src, bool vertical);

    void adjustHsv(cv::Mat src, cv::Mat &dst, double hue, double saturation, double ilumination);
    void colorHalftone(cv::Mat src, cv::Mat &dst);
    hsv rgb2hsv(rgb in);
    rgb hsv2rgb(hsv in);
    void piecewise_linear_transformation(cv::Mat src, cv::Mat &dst, double a, double b, double c, double d, double Mg, double Mf);
    void log_trans(cv::Mat src, cv::Mat &dst, double a, double b, double c);
    void pow_trans(cv::Mat src, cv::Mat &dst, double a, double b, double c);
    void gamma_trans(cv::Mat src, cv::Mat &dst, double gamma);
    void histogram_equalization(cv::Mat src, cv::Mat &dst);
    void histogram_equalization_hsl(cv::Mat src, cv::Mat &dst);
    void get_histogram_hsv(cv::Mat src, std::vector<double> &cdf);
    std::vector<std::vector<int>> get_hist_matrix(cv::Mat src, cv::Mat dst);
    void histogram_sml(cv::Mat &src, cv::Mat dst);
    void histogram_gml(cv::Mat &src, cv::Mat dst);
    void MedianBlurNaive(cv::Mat src, cv::Mat &dst, int kSize=3);
    void MedianBlurFast(cv::Mat src, cv::Mat &dst, int kSize=3);
    void MedianBlurFastSingleChannel(cv::Mat src, cv::Mat &dst, int kSize=3, int channel=3);
    int get_Mat_val(cv::Mat src, int rol, int col, int channel);
    std::vector<std::vector<double>> getGaussianMatrix(int kSize, double sigma);
    void GaussianBlurNaive(cv::Mat src, cv::Mat &dst, int kSize=3, double sigma=1.0);
    void LaplaceSharp(cv::Mat src, cv::Mat &dst);
    std::vector<std::vector<double>> getRadialMatrix(int kSize);
    void RadialBlur(cv::Mat src, cv::Mat &dst);
    double get_distance(int x0, int y0, int x1, int y1, int x2, int y2);
    float degree;
    bool vertical;
    bool horizontal;
    bool opened;
    double raw_h;
    double raw_s;
    double raw_v;
    void displayImg(cv::Mat img);
    void init();
signals:
    void Mouse_Pressed();
    void Mouse_Pos();
    void Mouse_Released();
public slots:

};

#endif // LBL_MOUSE_H
