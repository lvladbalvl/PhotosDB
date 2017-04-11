#include "qslideswitch.h"
#include <QRect>

class QPainter;
class QPropertyAnimation;


class QSlideSwitchPrivate : 
	public QObject
{
	Q_OBJECT
	Q_PROPERTY(qreal position READ position WRITE setPosition)
public:
	QSlideSwitchPrivate(QSlideSwitch* q);
	~QSlideSwitchPrivate();

	qreal position() const;

	void drawSlider(QPainter *painter);
	void updateSliderRect(const QSize& size);

public slots:
	void animate(bool);
	void setPosition(qreal value);

public:
	QPropertyAnimation *animation;

private:
	qreal pos;
	QRectF sliderShape;
	QLinearGradient gradient;
	QSlideSwitch *q_ptr;
};

