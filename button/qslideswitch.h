#pragma once
#include <QAbstractButton>

class QSlideSwitchPrivate;
class QSlideSwitch : 
	public QAbstractButton
{
	Q_OBJECT
public:
	QSlideSwitch(QWidget *parent = 0);
	~QSlideSwitch();


protected:
	bool hitButton(const QPoint& p) const;
	void paintEvent(QPaintEvent *e);
	void resizeEvent(QResizeEvent *e);

private:
	QSlideSwitchPrivate *d_ptr;
	Q_DISABLE_COPY(QSlideSwitch)
	Q_DECLARE_PRIVATE(QSlideSwitch)
};
