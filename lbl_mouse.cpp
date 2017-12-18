#include "lbl_mouse.h"

lbl_mouse::lbl_mouse(QWidget *parent) : QLabel(parent)
{
    init();
    opened = false;
}

void lbl_mouse::init(){
    type = 0;
    start = QPoint(0,0);
    end = QPoint(0,0);
    cur = QPoint(0,0);
    cutting = false;
    degree = 0;
    vertical = false;
    horizontal = false;
}

void lbl_mouse::mouseMoveEvent(QMouseEvent *es){
    this->cur.setX(es->x());
    this->cur.setY(es->y());
    emit Mouse_Pos();
    if(cutting){
        update();
    }
}

void lbl_mouse::mousePressEvent(QMouseEvent *es){
    if(type == 1){
        this->start.setX(es->x());
        this->start.setY(es->y());
        cutting = true;
        emit Mouse_Pressed();
    }
    else if(type == 2){
        this->start.setX(es->x());
        this->start.setY(es->y());
        cutting = true;
    }
}

void lbl_mouse::mouseReleaseEvent(QMouseEvent *es)
{
    cutting = false;
    if(es->y() >= this->processedImg.rows || es->y() < 0 || es->x() >= this->processedImg.cols || es->x() < 0){
        return;
    }

    this->end.setX(es->x());
    this->end.setY(es->y());

    emit Mouse_Released();

    int width = cvRound(fabs(end.x()-start.x()));
    int height = cvRound(fabs(end.y()-start.y()));

    cv::Mat matDst = cv::Mat(cv::Size(width, height), processedImg.type(), cv::Scalar::all(255));
    if(type == 1){
        for(int i=0; i<width; ++i){
            for(int j=0; j<height; ++j){
                for (int k=0; k<processedImg.channels(); ++k)
                {
                        matDst.at<cv::Vec3b>(j, i)[k] = processedImg.at<cv::Vec3b>(j+start.y(), i+start.x())[k];
                }
            }
        }
        rawImg = matDst;
        processedImg = matDst;
        displayImg(rawImg);
    }
    else if(type == 2){
        for(int i=0; i<width; ++i){
            for(int j=0; j<height; ++j){
                for (int k=0; k<processedImg.channels(); ++k)
                {
                        matDst.at<cv::Vec3b>(j, i)[k] = processedImg.at<cv::Vec3b>(j+start.y(), i+start.x())[k];
                }
            }
        }
        cv::Mat mask = cv::Mat::zeros(matDst.size(), matDst.type());
        cv::Point center = cv::Point(width/2,height/2);
        cv::Size axes = cv::Size(width/2,height/2);
        cv::ellipse(mask, center, axes, 0, 0, 360, cv::Scalar(255,255,255), -1, 8, 0);


        cv::Mat dstImage = cv::Mat::zeros(matDst.size(), matDst.type());
        //matDst.copyTo(dstImage, mask);
        cv::bitwise_and(mask,matDst,dstImage);
        //cv::imshow("debug",mask);
        //cv::imshow("debug1",matDst);
        //cv::imshow("debug2",dstImage);
        rawImg = dstImage;
        processedImg = dstImage;
        displayImg(rawImg);
    }
    init();
}

void lbl_mouse::paintEvent(QPaintEvent* e)
{
    QLabel::paintEvent(e);
    if(type != 0){
        QPainter painter(this);
        QPen paintpen(Qt::black);
        painter.setPen(paintpen);
        if(type == 1){
            QRect rect(start,cur);
            QRect full(0,0,back.width(),back.height());
            painter.setBrush(QColor(0,0,0,100));
            painter.setClipRegion(QRegion(full)-QRegion(rect));
            painter.drawRect(full);
        }
        else if(type == 2){
            QRect rect(start,cur);
            QRect full(0,0,back.width(),back.height());
            painter.setBrush(QColor(0,0,0,100));
            painter.setClipRegion(QRegion(full)-QRegion(rect,QRegion::Ellipse));
            painter.drawRect(full);
        }
    }
}

void lbl_mouse::displayImg(cv::Mat img){
    QImage Qimg;
    cv::Mat rgb;
    if(img.channels()==3){
        cv::cvtColor(img,rgb,CV_BGR2RGB);

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
    this->back = QPixmap::fromImage(Qimg);
    this->setPixmap(QPixmap::fromImage(Qimg));
    this->resize(this->pixmap()->size());
}
cv::Mat lbl_mouse::rotate_bilinear(cv::Mat matSrc, double degree){

    double angle = degree * CV_PI / 180.;
    double alpha = cos(angle);
    double beta = sin(angle);
    int iWidth = matSrc.cols;
    int iHeight = matSrc.rows;
    int iNewWidth = cvRound(iWidth * fabs(alpha) + iHeight * fabs(beta));
    int iNewHeight = cvRound(iHeight * fabs(alpha) + iWidth * fabs(beta));

    cv::Mat matDst1 = cv::Mat(cv::Size(iNewWidth, iNewHeight), matSrc.type(), cv::Scalar::all(0));


    if (matSrc.empty()) return matDst1;

    float deltax = (iNewWidth-iWidth)/2;
    float deltay = (iNewHeight-iHeight)/2;

    double m[6];
    m[0] = alpha;
    m[1] = beta;
    m[2] = (1 - alpha) * iWidth / 2. - beta * iHeight / 2.;
    m[3] = -m[1];
    m[4] = m[0];
    m[5] = beta * iWidth / 2. + (1 - alpha) * iHeight / 2.;

    cv::Mat M = cv::Mat(2, 3, CV_64F, m);

    double D = m[0]*m[4] - m[1]*m[3];
    D = D != 0 ? 1./D : 0;
    double A11 = m[4]*D, A22 = m[0]*D;
    m[0] = A11; m[1] *= -D;
    m[3] *= -D; m[4] = A22;
    double b1 = -m[0]*m[2] - m[1]*m[5];
    double b2 = -m[3]*m[2] - m[4]*m[5];
    m[2] = b1; m[5] = b2;

    for (int y=0; y<iNewHeight; ++y)
    {
        for (int x=0; x<iNewWidth; ++x)
        {
            float fx = m[0] * (x-deltax) + m[1] * (y-deltay) + m[2];
            float fy = m[3] * (x-deltax) + m[4] * (y-deltay) + m[5];

            int sy  = cvFloor(fy);
            fy -= sy;
            if (sy < 0 || sy >= iHeight) continue;

            short cbufy[2];
            cbufy[0] = cv::saturate_cast<short>((1.f - fy) * 2048);
            cbufy[1] = 2048 - cbufy[0];

            int sx = cvFloor(fx);
            fx -= sx;

            if (sx < 0 || sx >= iWidth) continue;

            short cbufx[2];
            cbufx[0] = cv::saturate_cast<short>((1.f - fx) * 2048);
            cbufx[1] = 2048 - cbufx[0];

            for (int k=0; k<matSrc.channels(); ++k)
            {
                if (sy == iHeight - 1 || sx == iWidth - 1) {
                    continue;
                } else {
                    matDst1.at<cv::Vec3b>(y, x)[k] = (matSrc.at<cv::Vec3b>(sy, sx)[k] * cbufx[0] * cbufy[0] +
                        matSrc.at<cv::Vec3b>(sy+1, sx)[k] * cbufx[0] * cbufy[1] +
                        matSrc.at<cv::Vec3b>(sy, sx+1)[k] * cbufx[1] * cbufy[0] +
                        matSrc.at<cv::Vec3b>(sy+1, sx+1)[k] * cbufx[1] * cbufy[1]) >> 22;
                }
            }
        }
    }
    return matDst1;
}
cv::Mat lbl_mouse::horizontalFlip(cv::Mat matSrc, bool horizontal){
    int iWidth = matSrc.cols;
    int iHeight = matSrc.rows;
    cv::Mat matDst1 = cv::Mat(cv::Size(iWidth, iHeight), matSrc.type(), cv::Scalar::all(0));
    if (!horizontal){
        return matSrc;
    }
    else{
        for (int x=0; x < iWidth; ++x){
            for (int y=0; y<iHeight; ++y){
                for (int k=0; k<matSrc.channels(); ++k)
                {
                        matDst1.at<cv::Vec3b>(y, x)[k] = matSrc.at<cv::Vec3b>(iHeight-y-1, x)[k];
                }
            }
        }
    }
    return matDst1;
}


cv::Mat lbl_mouse::verticalFlip(cv::Mat matSrc, bool vertical){
    int iWidth = matSrc.cols;
    int iHeight = matSrc.rows;
    cv::Mat matDst1 = cv::Mat(cv::Size(iWidth, iHeight), matSrc.type(), cv::Scalar::all(0));
    if (!vertical){
        return matSrc;
    }
    else{
        for (int x=0; x < iWidth; ++x){
            for (int y=0; y<iHeight; ++y){
                for (int k=0; k<matSrc.channels(); ++k)
                {
                        matDst1.at<cv::Vec3b>(y, x)[k] = matSrc.at<cv::Vec3b>(y, iWidth-x-1)[k];
                }
            }
        }
    }
    return matDst1;
}




void lbl_mouse::adjustHsv(cv::Mat matSrc, cv::Mat &dst, double hue, double saturation, double ilumination)
{
    if ( matSrc.empty()) return;



    //cvrgb2hsv(matSrc, temp);
    //cvtColor(matSrc, temp, CV_RGB2HSV);

    int i, j;
    cv::Size size = matSrc.size();

    for (  i= 0; i<size.height; ++i)
    {
        for (  j=0; j<size.width; ++j)
        {
            // r,g,b,s,l range in [0.0, 1.0] but h in [0.0, 360.0)
                double b = dst.at<cv::Vec3b>(i, j)[0]/255.0;
                double g = dst.at<cv::Vec3b>(i, j)[1]/255.0;
                double r = dst.at<cv::Vec3b>(i, j)[2]/255.0;
                double mx = std::max({r,g,b}), mn = std::min({r,g,b});
                double h, s, l = (mx + mn) / 2.0;

                //caculate h
                if(mx == mn)               h = 0.0;
                else if(mx == r && g >= b) h = 60.0*(g-b)/(mx-mn);
                else if(mx == r && g < b)  h = 60.0*(g-b)/(mx-mn)+360.0;
                else if(mx == g)           h = 60.0*(b-r)/(mx-mn)+120.0;
                else                       h = 60.0*(r-g)/(mx-mn)+240.0;

                //caculate s
                if(l == 0.0 || mx == mn) s = 0.0;
                else if(0.0 < l && l <= 0.5)  s = (mx-mn)/(2*l);
                else                     s = (mx-mn)/(2-2*l);

                raw_h = h;
                raw_s = s;
                raw_v = l;
                //change hsl with hi,si,li
                h += hue;
                if(h >= 360.0)   h -= 360.0;
                else if(h < 0.0) h += 360.0;

                s += saturation/100.0;
                if(s >= 1.0)      s = 1.0;
                else if(s <= 0.0) s = 0.0;

                l += ilumination/100.0;
                if(l >= 1.0)      l = 1.0;
                else if(l <= 0.0) l = 0.0;

                //convert new hsl to new rgb
                if(s == 0.0)
                {
                    r = l; g = l; b = l;
                }
                else
                {
                    double q = (l<0.5) ? (l*(1+s)) : (l+s-(l*s));
                    double p = 2 * l - q;
                    h = h/360.0;
                    double trgb[] = {h+1.0/3, h, h-1.0/3};

                    for(int k = 0; k < 3; ++k)
                    {
                        if(trgb[k] < 0.0) trgb[k] += 1.0;
                        else if(trgb[k] > 1.0) trgb[k] -= 1.0;

                        if(trgb[k] < 1.0/6) trgb[k] = p+(q-p)*6.0*trgb[k];
                        else if(trgb[k] < 0.5) trgb[k] = q;
                        else if(trgb[k] < 2.0/3) trgb[k] = p+(q-p)*6.0*(2.0/3-trgb[k]);
                        else trgb[k] = p;
                    }
                    r = trgb[0], g = trgb[1], b = trgb[2];
                }
                dst.at<cv::Vec3b>(i, j)[0] = b*255;
                dst.at<cv::Vec3b>(i, j)[1] = g*255;
                dst.at<cv::Vec3b>(i, j)[2] = r*255;
        }
    }


}

void lbl_mouse::colorHalftone(cv::Mat src, cv::Mat &dst)
{
    cv::Size sz = src.size();
    cv::Mat mask_r = cv::Mat::zeros(src.size(), src.type());
    cv::Mat mask_g = cv::Mat::zeros(src.size(), src.type());
    cv::Mat mask_b = cv::Mat::zeros(src.size(), src.type());
    cv::Mat channel_r = cv::Mat::zeros(src.size(), src.type());
    cv::Mat channel_g = cv::Mat::zeros(src.size(), src.type());
    cv::Mat channel_b = cv::Mat::zeros(src.size(), src.type());
    double ratio = sz.width/255*0.025;
    double step = sz.width/50;
    for(int x=0;x<sz.width;x+=step){
        for(int y=0;y<sz.height;y+=step){
            cv::circle(mask_b, cv::Point(x,y), src.at<cv::Vec3b>(y,x)[0]*ratio, cv::Scalar(255,255,255), -1, 8, 0);
            cv::circle(mask_g, cv::Point(x,y), src.at<cv::Vec3b>(y,x)[1]*ratio, cv::Scalar(255,255,255), -1, 8, 0);
            cv::circle(mask_r, cv::Point(x,y), src.at<cv::Vec3b>(y,x)[2]*ratio, cv::Scalar(255,255,255), -1, 8, 0);
        }
    }
    //cv::imshow("debug1",mask_b);
    //cv::imshow("debug2",mask_g);
    //cv::imshow("debug3",mask_r);

    cv::bitwise_and(mask_b,src,channel_b);
    cv::bitwise_and(mask_g,src,channel_g);
    cv::bitwise_and(mask_r,src,channel_r);

    //cv::imshow("debugb",channel_b);
    //cv::imshow("debugg",channel_g);
    //cv::imshow("debugr",channel_r);
    dst = cv::Mat(src.size(),src.type(),cv::Scalar::all(255));
    for(int x=0;x<dst.cols;++x){
        for(int y=0;y<dst.rows;++y){
            dst.at<cv::Vec3b>(y,x)[0] = channel_b.at<cv::Vec3b>(y,x)[0];
            dst.at<cv::Vec3b>(y,x)[1] = channel_g.at<cv::Vec3b>(y,x)[1];
            dst.at<cv::Vec3b>(y,x)[2] = channel_r.at<cv::Vec3b>(y,x)[2];
        }
    }
    //cv::imshow("half",dst);

}

hsv lbl_mouse::rgb2hsv(rgb in)
{
    double b = in.b;
    double g = in.g;
    double r = in.r;
    double mx = std::max({r,g,b}), mn = std::min({r,g,b});
    double h, s, l = (mx + mn) / 2.0;

    //caculate h
    if(mx == mn)               h = 0.0;
    else if(mx == r && g >= b) h = 60.0*(g-b)/(mx-mn);
    else if(mx == r && g < b)  h = 60.0*(g-b)/(mx-mn)+360.0;
    else if(mx == g)           h = 60.0*(b-r)/(mx-mn)+120.0;
    else                       h = 60.0*(r-g)/(mx-mn)+240.0;

    //caculate s
    if(l == 0.0 || mx == mn) s = 0.0;
    else if(0.0 < l && l <= 0.5)  s = (mx-mn)/(2*l);
    else                     s = (mx-mn)/(2-2*l);
    hsv ret;
    ret.h = h;
    ret.s = s;
    ret.v = l;
    return ret;

}

rgb lbl_mouse::hsv2rgb(hsv in)
{
    double h = in.h;
    double s = in.s;
    double l = in.v;
    double r,g,b;
    if(s == 0.0)
    {
        r = l; g = l; b = l;
    }
    else
    {
        double q = (l<0.5) ? (l*(1+s)) : (l+s-(l*s));
        double p = 2 * l - q;
        h = h/360.0;
        double trgb[] = {h+1.0/3, h, h-1.0/3};

        for(int k = 0; k < 3; ++k)
        {
            if(trgb[k] < 0.0) trgb[k] += 1.0;
            else if(trgb[k] > 1.0) trgb[k] -= 1.0;

            if(trgb[k] < 1.0/6) trgb[k] = p+(q-p)*6.0*trgb[k];
            else if(trgb[k] < 0.5) trgb[k] = q;
            else if(trgb[k] < 2.0/3) trgb[k] = p+(q-p)*6.0*(2.0/3-trgb[k]);
            else trgb[k] = p;
        }
        r = trgb[0], g = trgb[1], b = trgb[2];
    }
    rgb ret;
    ret.r = r;
    ret.g = g;
    ret.b = b;
    return ret;
}

void lbl_mouse::piecewise_linear_transformation(cv::Mat src, cv::Mat &dst, double a, double b, double c, double d, double Mg, double Mf)
{
    if ((a > b) || (c > d))
    {
        QMessageBox::information(
            this,
            tr("Alert"),
            tr("分段线性变化输入参数错误！") );
        return;
    }
    dst = src.clone();
    for(int x=0; x<src.cols; ++x){
        for(int y=0; y<src.rows; ++y){
            for(int k=0;k<src.channels();++k){
                if(src.at<cv::Vec3b>(y,x)[k] >= 0 && src.at<cv::Vec3b>(y,x)[k] < a){
                    dst.at<cv::Vec3b>(y,x)[k] = c/a * src.at<cv::Vec3b>(y,x)[k];
                }
                else if(src.at<cv::Vec3b>(y,x)[k] >= a && src.at<cv::Vec3b>(y,x)[k] < b){
                    dst.at<cv::Vec3b>(y,x)[k] = (d-c)/(b-a)*(src.at<cv::Vec3b>(y,x)[k]-a)+c;
                }
                else if(src.at<cv::Vec3b>(y,x)[k] <= Mf){
                    dst.at<cv::Vec3b>(y,x)[k] = (Mg-d)/(Mf-b)*(src.at<cv::Vec3b>(y,x)[k]-b)+d;
                }
            }
        }
    }
}

void lbl_mouse::log_trans(cv::Mat src, cv::Mat &dst, double a, double b, double c)
{
    dst = src.clone();
    for(int x=0; x<src.cols; ++x){
        for(int y=0; y<src.rows; ++y){
            for(int k=0;k<src.channels();++k){
                dst.at<cv::Vec3b>(y,x)[k] = a+(log(src.at<cv::Vec3b>(y,x)[k]+1))/(b*log(c));
            }
        }
    }
}

void lbl_mouse::pow_trans(cv::Mat src, cv::Mat &dst, double a, double b, double c)
{
    dst = src.clone();
    for(int x=0; x<src.cols; ++x){
        for(int y=0; y<src.rows; ++y){
            for(int k=0;k<src.channels();++k){
                dst.at<cv::Vec3b>(y,x)[k] = pow(b,(c*floor(src.at<cv::Vec3b>(y,x)[k]-a)))-1;
            }
        }
    }
}

void lbl_mouse::gamma_trans(cv::Mat src, cv::Mat &dst, double gamma)
{
    CV_Assert(src.data);

    // accept only char type matrices
    CV_Assert(src.depth() != sizeof(uchar));

    // build look up table
    unsigned char lut[256];
    for( int i = 0; i < 256; i++ )
    {
        lut[i] = cv::saturate_cast<uchar>(pow((float)(i/255.0), gamma) * 255.0f);
    }

    dst = src.clone();
    const int channels = dst.channels();
    switch(channels)
    {
        case 1:
        {

            cv::MatIterator_<uchar> it, end;
            for( it = dst.begin<uchar>(), end = dst.end<uchar>(); it != end; it++ )
                *it = lut[(*it)];

            break;
        }
        case 3:
        {

            cv::MatIterator_<cv::Vec3b> it, end;
            for( it = dst.begin<cv::Vec3b>(), end = dst.end<cv::Vec3b>(); it != end; it++ )
            {
                (*it)[0] = lut[((*it)[0])];
                (*it)[1] = lut[((*it)[1])];
                (*it)[2] = lut[((*it)[2])];
            }

            break;
        }
    }
}

void lbl_mouse::histogram_equalization(cv::Mat src, cv::Mat &dst)
{
    int height = src.rows;
    int width = src.cols;
    double cdf_r[256] = {0};
    double cdf_g[256] = {0};
    double cdf_b[256] = {0};
    double fPs_R[256],fPs_G[256],fPs_B[256], temp_r[256], temp_g[256], temp_b[256];
    dst = src.clone();

    cv::MatIterator_<cv::Vec3b> it, end;
    for(it = src.begin<cv::Vec3b>(), end = src.end<cv::Vec3b>(); it != end; it++){
        cdf_b[(*it)[0]] ++;
        cdf_g[(*it)[1]] ++;
        cdf_r[(*it)[2]] ++;
    }

    for(int i = 0; i < 256; i++)
    {
        fPs_R[i] = cdf_r[i] / (height * width * 1.0f);
        fPs_G[i] = cdf_g[i] / (height * width * 1.0f);
        fPs_B[i] = cdf_b[i] / (height * width * 1.0f);
    }
    for(int i = 0; i < 256; i++)
    {

        if(i == 0)
        {
            temp_r[0] = fPs_R[0];
            temp_g[0] = fPs_G[0];
            temp_b[0] = fPs_B[0];
        }
        else
        {
            temp_r[i] = temp_r[i-1] + fPs_R[i];
            temp_g[i] = temp_g[i-1] + fPs_G[i];
            temp_b[i] = temp_b[i-1] + fPs_B[i];
        }

        cdf_r[i] = (int)(255.0f * temp_r[i] + 0.5f);
        cdf_g[i] = (int)(255.0f * temp_g[i] + 0.5f);
        cdf_b[i] = (int)(255.0f * temp_b[i] + 0.5f);
    }

    for(it = dst.begin<cv::Vec3b>(), end = dst.end<cv::Vec3b>(); it != end; it++){
        (*it)[0] = cdf_b[((*it)[0])];
        (*it)[1] = cdf_g[((*it)[1])];
        (*it)[2] = cdf_r[((*it)[2])];
    }


}

void lbl_mouse::histogram_equalization_hsl(cv::Mat src, cv::Mat &dst)
{
    std::vector<double> cdf(256,0);
    dst = src.clone();

    cv::MatIterator_<cv::Vec3b> it, end;
    get_histogram_hsv(src,cdf);

    for(it = dst.begin<cv::Vec3b>(), end = dst.end<cv::Vec3b>(); it != end; it++){
        rgb in;
        in.b = (*it)[0]/255.0;
        in.g = (*it)[1]/255.0;
        in.r = (*it)[2]/255.0;
        hsv out = rgb2hsv(in);
        out.v = cdf[int(out.v*255.0)]/255.0;
        rgb equalized = hsv2rgb(out);
        (*it)[0] = equalized.b*255;
        (*it)[1] = equalized.g*255;
        (*it)[2] = equalized.r*255;
    }

}

void lbl_mouse::get_histogram_hsv(cv::Mat src, std::vector<double> &cdf)
{
    int height = src.rows;
    int width = src.cols;
    double fPs[256] = {0}, temp[256] = {0};
    cdf.assign(256,0);


    cv::MatIterator_<cv::Vec3b> it, end;
    for(it = src.begin<cv::Vec3b>(), end = src.end<cv::Vec3b>(); it != end; it++){
        rgb in;
        in.b = (*it)[0]/255.0;
        in.g = (*it)[1]/255.0;
        in.r = (*it)[2]/255.0;
        hsv out = rgb2hsv(in);
        cdf[int(out.v*255.0)] ++;
    }

    for(int i = 0; i < 256; i++)
    {
        fPs[i] = cdf[i] / (height * width * 1.0f);
    }
    for(int i = 0; i < 256; i++)
    {

        if(i == 0)
        {
            temp[0] = fPs[0];
        }
        else
        {
            temp[i] = temp[i-1] + fPs[i];
        }

        cdf[i] = (int)(255.0f * temp[i] + 0.5f);
    }
}

std::vector<std::vector<int> > lbl_mouse::get_hist_matrix(cv::Mat src, cv::Mat dst)
{
    std::vector<std::vector<int> > result(256,std::vector<int>(256,0));
    std::vector<double> scr(256,0),dest(256,0);
    get_histogram_hsv(src,scr);
    get_histogram_hsv(dst,dest);
    for(int i=0;i<256;++i){
        for(int j=0;j<256;++j){
            result[j][i] = fabs(scr[i]-dest[j]);
        }
    }
    return result;
}

void lbl_mouse::histogram_sml(cv::Mat &src, cv::Mat dst)
{
    std::vector<std::vector<int> > matrix = get_hist_matrix(src,dst);
    int sml_mapping[256] = {0};
    for(int y=0;y<256;y++){
        sml_mapping[y] = y;
    }
    for(int y=0;y<256;y++){
        int minX = 0;
        double minValue = matrix[0][y];
        for(int x=1;x<256;++x){
            if(minValue > matrix[x][y]){
                minValue = matrix[x][y];
                minX = x;
            }
        }
        sml_mapping[y] = minX;
    }
    cv::MatIterator_<cv::Vec3b> it, end;
    for(it = src.begin<cv::Vec3b>(), end = src.end<cv::Vec3b>(); it != end; it++){
        rgb in;
        in.b = (*it)[0]/255.0;
        in.g = (*it)[1]/255.0;
        in.r = (*it)[2]/255.0;
        hsv out = rgb2hsv(in);
        out.v = sml_mapping[int(out.v*255.0)]/255.0;
        rgb equalized = hsv2rgb(out);
        (*it)[0] = equalized.b*255;
        (*it)[1] = equalized.g*255;
        (*it)[2] = equalized.r*255;
    }
}

void lbl_mouse::histogram_gml(cv::Mat &src, cv::Mat dst)
{
    std::vector<std::vector<int> > matrix = get_hist_matrix(src,dst);
    int gml_mapping[256] = {0};
    for(int y=0;y<256;y++){
        gml_mapping[y] = y;
    }
    int lastStartY = 0, lastEndY = 0, startY = 0, endY = 0;
    for(int x=0;x<256;++x){
        double minValue = matrix[x][0];
        for(int y=0;y<256;++y){
            if(minValue>matrix[x][y]){
                endY = y;
                minValue = matrix[x][y];
            }
        }
        if(startY != lastStartY || endY != lastEndY){
            for(int i=startY;i<=endY;++i){
                gml_mapping[i] = x;
            }
            lastStartY = startY;
            lastEndY = endY;
            startY = lastEndY + 1;
        }
    }
    cv::MatIterator_<cv::Vec3b> it, end;
    for(it = src.begin<cv::Vec3b>(), end = src.end<cv::Vec3b>(); it != end; it++){
        rgb in;
        in.b = (*it)[0]/255.0;
        in.g = (*it)[1]/255.0;
        in.r = (*it)[2]/255.0;
        hsv out = rgb2hsv(in);
        out.v = gml_mapping[int(out.v*255.0)]/255.0;
        rgb equalized = hsv2rgb(out);
        (*it)[0] = equalized.b*255;
        (*it)[1] = equalized.g*255;
        (*it)[2] = equalized.r*255;
    }
}

void lbl_mouse::MedianBlurNaive(cv::Mat src, cv::Mat &dst, int kSize)
{
    int height = src.rows;
    int width = src.cols;
    int chns = src.channels();
    int border = (kSize-1)/2;    // 边界像素不处理
    int totalNum = kSize*kSize;  // 模板大小
    int iMid = (totalNum-1)/2;   // 模板中间值索引号

    if ( dst.empty())
        dst.create(height, width, src.type());

    std::vector<int> ucharVal;
    int i, j, k, p , q;

    for ( i =border; i<height-border; i++)
    {
        unsigned char* dstData = (unsigned char*)dst.data + dst.step*i;
        for ( j=border; j<width-border; j++)
        {
            for ( k=0; k<chns; k++)
            {
                ucharVal.clear();
                for ( p = -border; p<=border; p++)
                {
                    for ( q = -border; q<=border; q++)
                    {
                        ucharVal.push_back(src.at<cv::Vec3b>(i+p,j+q)[k]);
                    }
                }
                std::sort(ucharVal.begin(), ucharVal.end());  // 排序

                dstData[j*chns+k] = ucharVal[iMid];
            }   // for k

        }
    }
}

void lbl_mouse::MedianBlurFast(cv::Mat src, cv::Mat &dst, int kSize)
{

    dst = src.clone();

    for(int channel=0; channel<3 ; ++channel){
        MedianBlurFastSingleChannel(src,dst,kSize,channel);
    }

}

void lbl_mouse::MedianBlurFastSingleChannel(cv::Mat src, cv::Mat &dst, int kSize, int channel)
{
    int med, delta_l;
    int m, n;
    int prev, next;
    int h[256];
    int direction = 1;
    int row1, row2, col1, col2;
    int row, col, r, c;
    int Nx = (kSize-1)/2;
    int Ny = (kSize-1)/2;
    int middle=((Ny*2+1)*(Nx*2+1)+1)/2;
    int ROWNUM = src.rows;
    int COLNUM = src.cols;
    memset(h, 0, sizeof(h));
    // Histogram For (0,0)-element
    qDebug() << "channel" << channel << "\n";
    for(row = -Ny; row <= Ny; row++)
    {
        for(col = -Nx; col <= Nx; col++)

            h[get_Mat_val(src,row,col,channel)]++;
    }
    // Median
    for(m=0,n=0; n < 256; n++)
    {
        m += h[n];
        if(m>=middle)
            break;
    }
    med=n;
    delta_l = m - h[n];


    // Now, Median Is Defined For (0,0)-element
    // Begin Scanning: direction - FORWARD
    dst.at<cv::Vec3b>(0,0)[channel] = med;


    // main loop
    for(col=1, row=0; row<ROWNUM; row++)
    {
        // Prepare to Horizontal Scanning
        row1=row-Ny;
        row2=row+Ny;
        for(; col>=0 && col<COLNUM; col += direction)
        {
            // Determine Previous and Next Columns
            // Pay Attention To Defined Direction !!!
            prev = col-direction*(Nx+1);
            next = col+direction*Nx;

            // Now Change Old Histogram
            // New Histogram
            // delete previous

            for(r=row1; r<=row2; r++)
            {
                int value_out = get_Mat_val(src,r,prev,channel);
                int value_in = get_Mat_val(src,r,next,channel);
                if(value_out == value_in)
                    continue;
                h[value_out]--;
                if(value_out < med)
                    delta_l--;
                h[value_in]++;
                if(value_in < med)
                    delta_l++;
            }

            // Update new median
            if(delta_l >= middle)
            {
                while(delta_l>=middle)
                {
                    if(h[--med] > 0)
                        delta_l -= h[med];
                }
            }
            else
            {
                while(delta_l + h[med] < middle)
                {
                    if(h[med] > 0)
                        delta_l += h[med];
                    med++;
                }
            }
            dst.at<cv::Vec3b>(row,col)[channel] = med;
        } // end of column loop


        if(row == ROWNUM-1)
            return;

        // go back to the last/first pixel of the line
        col -= direction;
        // change direction to the opposite
        direction *= -1;


        // Shift Down One Line
        prev = row1;
        next = row2+1;


        col1 = col - Nx;
        col2 = col + Nx;

        for(c=col1; c<=col2; c++)
        {
            int value_out = get_Mat_val(src,prev,c,channel);
            int value_in = get_Mat_val(src,next,c,channel);
            if(value_out == value_in)
                continue;
            h[value_out]--;
            if(value_out<med)
                delta_l--;
            h[value_in]++;
            if(value_in<med)
                delta_l++;
        }

        if(delta_l>=middle)
        {
            while(delta_l>=middle)
            {
                if(h[--med]>0)
                    delta_l -= h[med];
            }
        }
        else
        {
            while(delta_l + h[med] < middle)
            {
                if(h[med]>0)
                    delta_l += h[med];
                med++;
            }
        }
        dst.at<cv::Vec3b>(row+1,col)[channel] = med;
        col += direction;
    }
}


int lbl_mouse::get_Mat_val(cv::Mat src, int row, int col, int channel)
{
    if(row >= 0 && col >= 0 && row < src.rows && col < src.cols)
        return src.at<cv::Vec3b>(row,col)[channel];
    return 0;
}

std::vector<std::vector<double> > lbl_mouse::getGaussianMatrix(int kSize, double sigma)
{
    float dDis;  //数组中一点到中心点距离
    float dCenter = (kSize-1)/2;

    float dSum = 0;
    float dValue;
    int i, j;
    std::vector<std::vector<double> > result(kSize, std::vector<double>(kSize,0));
    for ( i=0; i<kSize; i++)
    {
        for ( j=0; j<kSize; j++)
        {
            dDis = fabsf(i-dCenter) + fabsf(j-dCenter);  // 用和来近似平方和的开方
            dValue = exp(-dDis*dDis/(2*sigma*sigma+DBL_EPSILON));
            result[i][j] = dValue;
            dSum+=dValue;
        }
    }

    //归一化
    for ( i=0; i<kSize; i++)
    {
        for ( j=0; j<kSize; j++)
        {
            result[i][j] /= (dSum+DBL_EPSILON);
        }
    }
    return result;
}

void lbl_mouse::GaussianBlurNaive(cv::Mat src, cv::Mat &dst, int kSize, double sigma)
{
    int height = src.rows;
    int width = src.cols;
    int chns = src.channels();

    int i, j, k,p, q;

    std::vector<std::vector <double>> pKernel = getGaussianMatrix(kSize,sigma);


    if ( dst.empty())
        dst.create(height, width, src.type());

    float sum = 0;
    int border = (kSize-1)/2;

    for ( i =border; i<height-border; i++)
    {
        unsigned char* dstData = (unsigned char*)dst.data + dst.step*i;
        for ( j=border; j<width-border; j++)
        {
            for ( k=0; k<chns; k++)
            {
                sum = 0;
                for ( p = -border; p<=border; p++)
                {
                    for ( q = -border; q<=border; q++)
                    {
                        sum += src.at<cv::Vec3b>(i+p,j+q)[k]*pKernel[p+border][q+border];
                    }
                }

                dstData[j*chns+k] = cv::saturate_cast<uchar>(sum+0.5);
            }   // for k
        }
    } // for i
}

void lbl_mouse::LaplaceSharp(cv::Mat src, cv::Mat &dst)
{
    if ( dst.empty())
            dst.create(src.rows, src.cols, src.type());

        int height = src.rows;
        int width = src.cols;
        int chns = src.channels();
        int border = 1;
        int i, j, k;

        for ( i=border; i<height-border; i++)
        {
            unsigned char* dstData = (unsigned char*)dst.data + dst.step*i;
            for ( j=border; j<width-border; j++)
            {
                for ( k=0; k<chns; k++)
                {
                    int sum = 9*src.at<cv::Vec3b>(i,j)[k] - src.at<cv::Vec3b>(i-1,j-1)[k] - src.at<cv::Vec3b>(i-1,j)[k]
                            - src.at<cv::Vec3b>(i-1,j+1)[k] - src.at<cv::Vec3b>(i,j-1)[k] - src.at<cv::Vec3b>(i,j+1)[k]
                        - src.at<cv::Vec3b>(i+1,j-1)[k] - src.at<cv::Vec3b>(i+1,j)[k] - src.at<cv::Vec3b>(i+1,j+1)[k];

                    dstData[j*chns+k] = cv::saturate_cast<uchar>(sum) ;
                }
            }
        }
}

std::vector<std::vector<double> > lbl_mouse::getRadialMatrix(int kSize)
{
    std::vector<std::vector<double> > result(kSize, std::vector<double>(kSize,0));
    return result;
}

void lbl_mouse::RadialBlur(cv::Mat src, cv::Mat &dst)
{
    int height = src.rows;
    int width = src.cols;
    int chns = src.channels();
    int C_x = width/2;
    int C_y = height/2;
    dst = src.clone();
    for(int x=0;x<width;++x){
        for(int y=0;y<height;++y){
            int kernel_size = sqrt((x-C_x)*(x-C_x)+(y-C_y)*(y-C_y))*80/(height+width);
            if(kernel_size <= 0){
                continue;
            }
            int border = (kernel_size-1)/2;
            std::vector<std::vector <double> > pKernel = getRadialMatrix(kernel_size);
            double sum = 0;
            double conv_b = 0;
            double conv_g = 0;
            double conv_r = 0;
            for ( int p = -border; p<=border; p++)
            {
                for ( int q = -border; q<=border; q++)
                {
                    if(y+p >= height || y+p < 0 | x+q < 0 || x+q >=width){
                        continue;
                    }
                    double d = get_distance(x+q,y+p,x,y,C_x,C_y);
                    pKernel[p+border][q+border] = 1-d > 0 ? 1-d : 0;
                    sum += pKernel[p+border][q+border];
                }
            }
            for ( int p = -border; p<=border; p++)
            {
                for ( int q = -border; q<=border; q++)
                {
                    if(y+p >= height || y+p < 0 | x+q < 0 || x+q >=width){
                        continue;
                    }
                    pKernel[p+border][q+border] /= sum;
                    conv_b += pKernel[p+border][q+border]*src.at<cv::Vec3b>(y+p,x+q)[0];
                    conv_g += pKernel[p+border][q+border]*src.at<cv::Vec3b>(y+p,x+q)[1];
                    conv_r += pKernel[p+border][q+border]*src.at<cv::Vec3b>(y+p,x+q)[2];
                }
            }
            dst.at<cv::Vec3b>(y,x)[0] = cv::saturate_cast<uchar>(conv_b+0.5);
            dst.at<cv::Vec3b>(y,x)[1] = cv::saturate_cast<uchar>(conv_g+0.5);
            dst.at<cv::Vec3b>(y,x)[2] = cv::saturate_cast<uchar>(conv_r+0.5);
            //qDebug() << "x " << x << " y"<< y << "\n";
        }
    }
}

double lbl_mouse::get_distance(int x0, int y0, int x1, int y1, int x2, int y2)
{
    return fabs((y0-y1)*(x2-x1)+(y2-y1)*(x1-x0))/(sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1)));
}

void lbl_mouse::GaussFilters(cv::Mat src, cv::Mat &dst, int D0, int type)
{
    dst = src.clone();
    for(int k=0; k<3; ++k){
        GaussFiltersHelper(src,dst,D0,k,type);
    }
    if(type==2){
        histogram_equalization_hsl(dst.clone(), dst);
    }
}

void lbl_mouse::GaussFiltersHelper(cv::Mat src, cv::Mat &dst, int D0, int channel, int type)
{
    int r=2*D0*D0;

    cv::Mat bgr[3];   //destination array
    cv::split(src,bgr);
    //cv::Mat f = bgr[channel];
    int M = src.cols;
    int N = src.rows;
    int P = 2*src.cols;
    int Q = 2*src.rows;
    int M_padding = cv::getOptimalDFTSize(M);
    int N_padding = cv::getOptimalDFTSize(N);
    cv::Mat pad(N_padding,M_padding,CV_32FC1);
    for(int x=0;x<pad.cols;++x){
        for(int y=0;y<pad.rows;++y){
            if(x < M && y < N){
               pad.at<float>(y,x) = bgr[channel].at<uchar>(y,x) * int(pow(-1,(x+y)));
            }
            else{
                pad.at<float>(y,x) = 0;
            }
        }
    }
    cv::Mat planes[] = {cv::Mat_<float>(pad), cv::Mat::zeros(pad.size(), CV_32FC1)};
    cv::Mat F(pad.size(), CV_32FC2);
    cv::merge(planes, 2, F);
    cv::Mat H(F.size(), CV_32FC2);
    cv::dft(F,F, cv::DFT_SCALE|cv::DFT_COMPLEX_OUTPUT);
    for(int x=0; x < H.cols; ++x){
        for(int y=0; y < H.rows; ++y){
            double D = pow(x-M_padding/2,2) + pow(y-N_padding/2,2);
            if(type == 0){
                H.at<cv::Vec2f>(y,x)[0] = expf(-D/r);
                H.at<cv::Vec2f>(y,x)[1] = expf(-D/r);
            }
            else{
                H.at<cv::Vec2f>(y,x)[0] = 2-expf(-D/r);
                H.at<cv::Vec2f>(y,x)[1] = 2-expf(-D/r);
            }
        }
    }
    //cv::Mat G(H.size(), CV_32FC2);
    cv::multiply(F,H,H);
    cv::dft(H, H, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);
    //cv::Mat imgout;
    cv::split(H,planes);
    //cv::imshow("debug", planes[0]);
    //cv::normalize(planes[0], imgout, 0, 1, CV_MINMAX);
    for(int x=0; x<planes[0].cols; ++x){
        for(int y=0; y<planes[0].rows; ++y){
            int pixel = int(planes[0].at<float>(y,x) * int(pow(-1,(x+y))));
            if(pixel > 255) pixel = 255;
            if(pixel < 0) pixel = 0;
            dst.at<cv::Vec3b>(y,x)[channel] = pixel;
            //qDebug() << "debug x: " << x << "y: " << y << "pixel: " << dst.at<cv::Vec3b>(y,x)[channel];
        }
    }

}


