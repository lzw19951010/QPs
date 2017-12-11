#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lbl_mouse.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open Image"),
                QDir::currentPath(),
                "All files (*.*);; JPG files(*.jpg) ;; BMP files(*.bmp);; PNG files(*.png)"
                );
    if(fileName.isEmpty()) return;
    ui->imgLabel->init();
    ui->imgLabel->opened = true;
    ui->imgLabel->rawImg = cv::imread(fileName.toStdString());
    ui->imgLabel->processedImg = ui->imgLabel->rawImg.clone();
    ui->imgLabel->displayImg(ui->imgLabel->rawImg);
    show_rgbHistogram(ui->imgLabel->rawImg);

}

void MainWindow::on_pushButton_2_clicked()
{
    if(!ui->imgLabel->opened) return;
    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save Image"),
                QDir::currentPath(),
                "All files (*.*);; JPG files(*.jpg) ;; BMP files(*.bmp);; PNG files(*.png)"
                );
    if(fileName.isEmpty()) return;
    cv::imwrite(fileName.toStdString(),ui->imgLabel->processedImg);
    ui->imgLabel->rawImg = ui->imgLabel->processedImg;
}

void MainWindow::on_pushButton_3_clicked()
{
    float delta;
    if((ui->imgLabel->horizontal && ui->imgLabel->vertical) || (!ui->imgLabel->horizontal && !ui->imgLabel->vertical)){
        delta = -90;
    }
    else{
        delta = 90;
    }
    ui->imgLabel->degree += delta;
    ui->imgLabel->processedImg = ui->imgLabel->rotate_bilinear(ui->imgLabel->rawImg, ui->imgLabel->degree);
    ui->imgLabel->processedImg = ui->imgLabel->horizontalFlip(ui->imgLabel->processedImg, ui->imgLabel->horizontal);
    ui->imgLabel->processedImg = ui->imgLabel->verticalFlip(ui->imgLabel->processedImg, ui->imgLabel->vertical);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_pushButton_4_clicked()
{
    float delta;
    if((ui->imgLabel->horizontal && ui->imgLabel->vertical) || (!ui->imgLabel->horizontal && !ui->imgLabel->vertical)){
        delta = 90;
    }
    else{
        delta = -90;
    }
    ui->imgLabel->degree += delta;
    ui->imgLabel->processedImg = ui->imgLabel->rotate_bilinear(ui->imgLabel->rawImg, ui->imgLabel->degree);
    ui->imgLabel->processedImg = ui->imgLabel->horizontalFlip(ui->imgLabel->processedImg, ui->imgLabel->horizontal);
    ui->imgLabel->processedImg = ui->imgLabel->verticalFlip(ui->imgLabel->processedImg, ui->imgLabel->vertical);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}







void MainWindow::on_pushButton_5_clicked()
{
    ui->imgLabel->horizontal = !ui->imgLabel->horizontal;
    ui->imgLabel->processedImg = ui->imgLabel->rotate_bilinear(ui->imgLabel->rawImg, ui->imgLabel->degree);
    ui->imgLabel->processedImg = ui->imgLabel->horizontalFlip(ui->imgLabel->processedImg, ui->imgLabel->horizontal);
    ui->imgLabel->processedImg = ui->imgLabel->verticalFlip(ui->imgLabel->processedImg, ui->imgLabel->vertical);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_pushButton_6_clicked()
{
    ui->imgLabel->vertical = !ui->imgLabel->vertical;
    ui->imgLabel->processedImg = ui->imgLabel->rotate_bilinear(ui->imgLabel->rawImg, ui->imgLabel->degree);
    ui->imgLabel->processedImg = ui->imgLabel->horizontalFlip(ui->imgLabel->processedImg, ui->imgLabel->horizontal);
    ui->imgLabel->processedImg = ui->imgLabel->verticalFlip(ui->imgLabel->processedImg, ui->imgLabel->vertical);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_pushButton_7_clicked()
{
    float delta;
    if((ui->imgLabel->horizontal && ui->imgLabel->vertical) || (!ui->imgLabel->horizontal && !ui->imgLabel->vertical)){
        delta = ui->doubleSpinBox->value();
    }
    else{
        delta = -ui->doubleSpinBox->value();
    }
    ui->imgLabel->degree += delta;
    ui->imgLabel->processedImg = ui->imgLabel->rotate_bilinear(ui->imgLabel->rawImg, ui->imgLabel->degree);
    ui->imgLabel->processedImg = ui->imgLabel->horizontalFlip(ui->imgLabel->processedImg, ui->imgLabel->horizontal);
    ui->imgLabel->processedImg = ui->imgLabel->verticalFlip(ui->imgLabel->processedImg, ui->imgLabel->vertical);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_pushButton_8_clicked()
{
    ui->imgLabel->type = 1;
    update();
}

void MainWindow::show_rgbHistogram(cv::Mat img)
{
    int bins = 256;
    int hist_size[] = {bins};
    float range[] = { 0, 256 };
    const float* ranges[] = { range};
    cv::MatND hist_r,hist_g,hist_b;
    int channels_r[] = {0};

    cv::calcHist( &img, 1, channels_r, cv::Mat(), // do not use mask
      hist_r, 1, hist_size, ranges,
      true, // the histogram is uniform
      false );

    int channels_g[] = {1};
    cv::calcHist( &img, 1, channels_g, cv::Mat(), // do not use mask
      hist_g, 1, hist_size, ranges,
      true, // the histogram is uniform
      false );

    int channels_b[] = {2};
    cv::calcHist( &img, 1, channels_b, cv::Mat(), // do not use mask
      hist_b, 1, hist_size, ranges,
      true, // the histogram is uniform
      false );
    double max_val_r,max_val_g,max_val_b;
    cv::minMaxLoc(hist_r, 0, &max_val_r, 0, 0);
    cv::minMaxLoc(hist_g, 0, &max_val_g, 0, 0);
    cv::minMaxLoc(hist_b, 0, &max_val_b, 0, 0);
    int scale = 1;
    int hist_height=256;
    cv::Mat hist_img = cv::Mat::zeros(hist_height,(bins)*3, CV_8UC3);
    for(int i=0;i<bins;i++)
    {
      float bin_val_r = hist_r.at<float>(i);
      float bin_val_g = hist_g.at<float>(i);
      float bin_val_b = hist_b.at<float>(i);
      int intensity_r = cvRound(bin_val_r*hist_height/max_val_r);  //要绘制的高度
      int intensity_g = cvRound(bin_val_g*hist_height/max_val_g);  //要绘制的高度
      int intensity_b = cvRound(bin_val_b*hist_height/max_val_b);  //要绘制的高度
      cv::rectangle(hist_img,cv::Point(i*scale,hist_height-1),
          cv::Point((i+1)*scale - 1, hist_height - intensity_r),
          CV_RGB(255,0,0));

      cv::rectangle(hist_img,cv::Point((i+bins)*scale,hist_height-1),
          cv::Point((i+bins+1)*scale - 1, hist_height - intensity_g),
          CV_RGB(0,255,0));

      cv::rectangle(hist_img,cv::Point((i+bins*2)*scale,hist_height-1),
          cv::Point((i+(bins)*2+1)*scale - 1, hist_height - intensity_b),
          CV_RGB(0,0,255));

    }

    cv::Mat hist_img_resized;
    cv::resize(hist_img,hist_img_resized,cv::Size(), 0.5, 0.5);
    QImage Qimg;
    cv::Mat rgb;
    if(hist_img_resized.channels()==3){
        cv::cvtColor(hist_img_resized,rgb,CV_BGR2RGB);

        Qimg = QImage((const unsigned char*)(rgb.data),
                rgb.cols,
                rgb.rows,
                rgb.cols*rgb.channels(),
                QImage::Format_RGB888);
    }
    else{
        Qimg = QImage((const unsigned char*)(img.data),
                img.cols,
                img.rows,
                img.cols*img.channels(),
                QImage::Format_RGB888);
    }
    ui->rgb_hgram->setPixmap(QPixmap::fromImage(Qimg));
    ui->rgb_hgram->resize(ui->rgb_hgram->pixmap()->size());
}
void MainWindow::show_dstHistogram(cv::Mat img)
{
    int bins = 256;
    int hist_size[] = {bins};
    float range[] = { 0, 256 };
    const float* ranges[] = { range};
    cv::MatND hist_r,hist_g,hist_b;
    int channels_r[] = {0};

    cv::calcHist( &img, 1, channels_r, cv::Mat(), // do not use mask
      hist_r, 1, hist_size, ranges,
      true, // the histogram is uniform
      false );

    int channels_g[] = {1};
    cv::calcHist( &img, 1, channels_g, cv::Mat(), // do not use mask
      hist_g, 1, hist_size, ranges,
      true, // the histogram is uniform
      false );

    int channels_b[] = {2};
    cv::calcHist( &img, 1, channels_b, cv::Mat(), // do not use mask
      hist_b, 1, hist_size, ranges,
      true, // the histogram is uniform
      false );
    double max_val_r,max_val_g,max_val_b;
    cv::minMaxLoc(hist_r, 0, &max_val_r, 0, 0);
    cv::minMaxLoc(hist_g, 0, &max_val_g, 0, 0);
    cv::minMaxLoc(hist_b, 0, &max_val_b, 0, 0);
    int scale = 1;
    int hist_height=256;
    cv::Mat hist_img = cv::Mat::zeros(hist_height,(bins)*3, CV_8UC3);
    for(int i=0;i<bins;i++)
    {
      float bin_val_r = hist_r.at<float>(i);
      float bin_val_g = hist_g.at<float>(i);
      float bin_val_b = hist_b.at<float>(i);
      int intensity_r = cvRound(bin_val_r*hist_height/max_val_r);  //要绘制的高度
      int intensity_g = cvRound(bin_val_g*hist_height/max_val_g);  //要绘制的高度
      int intensity_b = cvRound(bin_val_b*hist_height/max_val_b);  //要绘制的高度
      cv::rectangle(hist_img,cv::Point(i*scale,hist_height-1),
          cv::Point((i+1)*scale - 1, hist_height - intensity_r),
          CV_RGB(255,0,0));

      cv::rectangle(hist_img,cv::Point((i+bins)*scale,hist_height-1),
          cv::Point((i+bins+1)*scale - 1, hist_height - intensity_g),
          CV_RGB(0,255,0));

      cv::rectangle(hist_img,cv::Point((i+bins*2)*scale,hist_height-1),
          cv::Point((i+(bins)*2+1)*scale - 1, hist_height - intensity_b),
          CV_RGB(0,0,255));

    }
    cv::Mat hist_img_resized;
    cv::resize(hist_img,hist_img_resized,cv::Size(), 0.5, 0.5);
    QImage Qimg;
    cv::Mat rgb;
    if(hist_img_resized.channels()==3){
        cv::cvtColor(hist_img_resized,rgb,CV_BGR2RGB);

        Qimg = QImage((const unsigned char*)(rgb.data),
                rgb.cols,
                rgb.rows,
                rgb.cols*rgb.channels(),
                QImage::Format_RGB888);
    }
    else{
        Qimg = QImage((const unsigned char*)(img.data),
                img.cols,
                img.rows,
                img.cols*img.channels(),
                QImage::Format_RGB888);
    }
    ui->dst_img_hist->setPixmap(QPixmap::fromImage(Qimg));
    ui->dst_img_hist->resize(ui->dst_img_hist->pixmap()->size());
}
void MainWindow::on_circle_cut_btn_clicked()
{
    ui->imgLabel->type = 2;
    update();
}

void MainWindow::on_pushButton_9_clicked()
{
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    cv::Mat processed = ui->imgLabel->processedImg.clone();
    //cv::Mat temp = ui->imgLabel->processedImg.clone();
    double h,s,v;
    h=ui->hspin->value();
    s=ui->sspin->value();
    v=ui->vspin->value();

    ui->imgLabel->adjustHsv(rawImg, ui->imgLabel->processedImg,h,s,v);

    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
    ui->hspin->setValue(0);
    ui->sspin->setValue(0);
    ui->vspin->setValue(0);
}

void MainWindow::on_pushButton_10_clicked()
{
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();

    ui->imgLabel->colorHalftone(rawImg, ui->imgLabel->processedImg);

    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);

}

void MainWindow::on_linear_clicked()
{
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    double a= ui->linear_a->value();
    double b= ui->linear_b->value();
    double c= ui->linear_c->value();
    double d= ui->linear_d->value();
    double Mg= ui->linear_Mg->value();
    double Mf= ui->linear_Mf->value();
    ui->imgLabel->piecewise_linear_transformation(rawImg, ui->imgLabel->processedImg, a,b,c,d,Mg,Mf);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}


void MainWindow::on_log_trans_clicked()
{
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    double a= ui->log_a->value();
    double b= ui->log_b->value();
    double c= ui->log_c->value();
    ui->imgLabel->log_trans(rawImg, ui->imgLabel->processedImg, a,b,c);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_pow_trans_clicked()
{
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    double a= ui->pow_a->value();
    double b= ui->pow_b->value();
    double c= ui->pow_c->value();
    ui->imgLabel->pow_trans(rawImg, ui->imgLabel->processedImg, a,b,c);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_gamma_correction_clicked()
{
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    double gamma = ui->gamma_value->value();
    ui->imgLabel->gamma_trans(rawImg, ui->imgLabel->processedImg, gamma);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_histogram_equal_clicked()
{
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    ui->imgLabel->histogram_equalization(rawImg, ui->imgLabel->processedImg);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_histogram_equal_hsv_clicked()
{
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    ui->imgLabel->histogram_equalization_hsl(rawImg, ui->imgLabel->processedImg);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_pushButton_11_clicked()
{
    ui->imgLabel->init();
    ui->imgLabel->processedImg=ui->imgLabel->rawImg.clone();
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_open_dst_img_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open Image"),
                QDir::currentPath(),
                "All files (*.*);; JPG files(*.jpg) ;; BMP files(*.bmp);; PNG files(*.png)"
                );
    if(fileName.isEmpty()) return;
    ui->dst_img_label->init();
    ui->dst_img_label->opened = true;
    ui->dst_img_label->rawImg = cv::imread(fileName.toStdString());
    ui->dst_img_label->processedImg = ui->dst_img_label->rawImg.clone();
    ui->dst_img_label->displayImg(ui->dst_img_label->rawImg);
    show_dstHistogram(ui->dst_img_label->rawImg);
}

void MainWindow::on_his_sml_clicked()
{
    if(ui->imgLabel->processedImg.empty() || ui->dst_img_label->rawImg.empty()){
        QMessageBox::information(
            this,
            tr("Alert"),
            tr("请载入图片后再操作！") );
        return;
    }
    ui->imgLabel->histogram_sml(ui->imgLabel->processedImg, ui->dst_img_label->rawImg);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_his_gml_clicked()
{
    if(ui->imgLabel->processedImg.empty() || ui->dst_img_label->rawImg.empty()){
        QMessageBox::information(
            this,
            tr("Alert"),
            tr("请载入图片后再操作！") );
        return;
    }
    ui->imgLabel->histogram_gml(ui->imgLabel->processedImg, ui->dst_img_label->rawImg);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_median_naive_clicked()
{
    if(ui->imgLabel->processedImg.empty()){
        QMessageBox::information(
            this,
            tr("Alert"),
            tr("请载入图片后再操作！") );
        return;
    }
    QTime time;
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    int kSize = ui->median_kernal_val->value();
    time.start();
    ui->imgLabel->MedianBlurNaive(rawImg, ui->imgLabel->processedImg, kSize);
    int time_Diff = time.elapsed();
    float f = time_Diff/1000.0;
    QString tr_timeDiff = QString("%1").arg(f);
    QString content = "Total time of median blur naive version: " + tr_timeDiff + " seconds!";

    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
    QMessageBox::information(
        this,
        tr("Alert"),
        content );
    return;
}

void MainWindow::on_gaussian_maive_clicked()
{
    if(ui->imgLabel->processedImg.empty()){
        QMessageBox::information(
            this,
            tr("Alert"),
            tr("请载入图片后再操作！") );
        return;
    }
    QTime time;
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    int kSize = ui->gaussian_kernal_val->value();
    double sigma = ui->gaussian_sigma_val->value();
    time.start();
    ui->imgLabel->GaussianBlurNaive(rawImg, ui->imgLabel->processedImg, kSize, sigma);
    int time_Diff = time.elapsed();
    float f = time_Diff/1000.0;
    QString tr_timeDiff = QString("%1").arg(f);
    QString content = "Total time of gaussian blur naive version: " + tr_timeDiff + " seconds!";

    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
    QMessageBox::information(
        this,
        tr("Alert"),
        content );
    return;
}

void MainWindow::on_laplace_clicked()
{
    if(ui->imgLabel->processedImg.empty()){
        QMessageBox::information(
            this,
            tr("Alert"),
            tr("请载入图片后再操作！") );
        return;
    }

    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    ui->imgLabel->LaplaceSharp(rawImg, ui->imgLabel->processedImg);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_r_blur_clicked()
{
    if(ui->imgLabel->processedImg.empty()){
        QMessageBox::information(
            this,
            tr("Alert"),
            tr("请载入图片后再操作！") );
        return;
    }

    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    ui->imgLabel->RadialBlur(rawImg, ui->imgLabel->processedImg);
    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
}

void MainWindow::on_median_fast_clicked()
{
    if(ui->imgLabel->processedImg.empty()){
        QMessageBox::information(
            this,
            tr("Alert"),
            tr("请载入图片后再操作！") );
        return;
    }
    QTime time;
    cv::Mat rawImg = ui->imgLabel->processedImg.clone();
    int kSize = ui->median_fast_kernal_val->value();
    time.start();
    ui->imgLabel->MedianBlurFast(rawImg, ui->imgLabel->processedImg, kSize);
    int time_Diff = time.elapsed();
    float f = time_Diff/1000.0;
    QString tr_timeDiff = QString("%1").arg(f);
    QString content = "Total time of median blur fast version: " + tr_timeDiff + " seconds!";

    ui->imgLabel->displayImg(ui->imgLabel->processedImg);
    show_rgbHistogram(ui->imgLabel->processedImg);
    QMessageBox::information(
        this,
        tr("Alert"),
        content );
    return;
}
