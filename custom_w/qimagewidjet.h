#ifndef QIMAGEWIDJET_H
#define QIMAGEWIDJET_H

#include <QWidget>
#include <QPixmap>
#include <QMargins>
#include <QFileSystemModel>
#include <QImageReader>

class QImageWidjet : public QWidget
{
    Q_OBJECT
public:
    explicit QImageWidjet(QWidget *parent = 0);
private:
    QPixmap _originalImage;
    QString _noImageMessage;
    int _margin;
    QMargins _margins;
    QColor _backgroundColor;
public:
    void setPixmap (QPixmap pixmap);
    QPixmap pixmap() {return _originalImage;}
    int margin() {return _margin;}
signals:
    void pixmapChanged();
    void marginChanged();
public slots:
    void setMargin(int value);
    // QWidget interface
    void imageLoad(QString fileName);
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);
};

#endif // QIMAGEWIDJET_H
