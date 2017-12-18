#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <cmath>
#include <QImage>
#include <QPoint>
#include <QDir>
#include <QMouseEvent>
#include <QTime>
#include "opencv2/opencv.hpp"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();


    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();


    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

private:
    Ui::MainWindow *ui;

public:
    void show_rgbHistogram(cv::Mat img);
    void show_dstHistogram(cv::Mat img);

private slots:

    void on_circle_cut_btn_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_10_clicked();
    void on_linear_clicked();
    void on_log_trans_clicked();
    void on_pow_trans_clicked();
    void on_gamma_correction_clicked();
    void on_histogram_equal_clicked();
    void on_histogram_equal_hsv_clicked();
    void on_pushButton_11_clicked();
    void on_open_dst_img_clicked();
    void on_his_sml_clicked();
    void on_his_gml_clicked();
    void on_median_naive_clicked();
    void on_gaussian_maive_clicked();
    void on_laplace_clicked();
    void on_r_blur_clicked();
    void on_median_fast_clicked();
    void on_low_Gauss_clicked();
    void on_Gauss_High_clicked();
    void on_gauss_fusion_clicked();
};

#endif // MAINWINDOW_H
