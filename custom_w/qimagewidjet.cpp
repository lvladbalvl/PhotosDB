#include "qimagewidjet.h"
#include <QMouseEvent>
#include <QPainter>
#include <QFileDialog>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDebug>

QImageWidjet::QImageWidjet(QWidget *parent) :
    QWidget(parent)
{
    _noImageMessage = "NO IMAGE";
    _backgroundColor = Qt::white;
    _margin = 4;
    connect(this, SIGNAL(marginChanged()), SLOT(repaint()));
    connect(this, SIGNAL(pixmapChanged()), SLOT(repaint()));
}

void QImageWidjet::setPixmap(QPixmap pixmap)
{
    _originalImage = pixmap;
    emit pixmapChanged();
}

void QImageWidjet::setMargin(int value)
{
    if (value < 2)
        value = 2;
    if (value > 30)
        value = 30;
    if (value != _margin)
        _margin = value;
    emit marginChanged();
}

void QImageWidjet::imageLoad(QString fileName)
{
  //  cv::Mat image = cv::imread(fileName.toUtf8().constData());
  //  cv::namedWindow("Tulips", CV_WINDOW_NORMAL);
  //  cv::imshow("Tulips", image);
 //   cv::waitKey(1000);
    _originalImage.load((fileName));
    _noImageMessage = fileName;
    emit pixmapChanged();
}


void QImageWidjet::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    QString filename = QFileDialog::getOpenFileName(this);
    if (filename.isEmpty())
        return;
    _noImageMessage = filename;
    //setPixmap(QPixmap(filename));
}

void QImageWidjet::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.setBrush(_backgroundColor);
    painter.drawRect(rect());
    painter.restore();
    if (_originalImage.isNull()){
        painter.drawText(rect(),Qt::AlignCenter, _noImageMessage);
    }
    else {
        QRect imageRect = rect();
        int originalWidth=imageRect.width();
        _originalImage=_originalImage.scaled(imageRect.width(),imageRect.height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        imageRect.setWidth(_originalImage.width());
        imageRect.setHeight(_originalImage.height());
        qDebug()<<originalWidth;
        qDebug() << imageRect.width();
        painter.drawPixmap((originalWidth-imageRect.width())/2,1, imageRect.width()-2,imageRect.height()-2, _originalImage);
    }
}


