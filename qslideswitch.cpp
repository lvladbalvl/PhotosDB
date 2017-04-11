#include "qslideswitch_p.h"
#include <QPen>
#include <QBrush>
#include <QPalette>
#include <QPainter>

#include <QPropertyAnimation>

#include <QPaintEvent>
#include <QResizeEvent>

QSlideSwitchPrivate::QSlideSwitchPrivate(QSlideSwitch *q) :
	q_ptr(q), animation(new QPropertyAnimation(this)), pos(0.0)
{
	animation->setTargetObject(this);
	animation->setPropertyName("position");
	animation->setStartValue(0.0);
	animation->setEndValue(1.0);
	animation->setDuration(300);
	animation->setEasingCurve(QEasingCurve::InOutExpo);
	gradient.setSpread(QGradient::PadSpread);
}

QSlideSwitchPrivate::~QSlideSwitchPrivate()
{
	delete animation;
}

void QSlideSwitchPrivate::setPosition(qreal value)
{
	pos = value;
	q_ptr->repaint();
}

qreal QSlideSwitchPrivate::position() const
{
	return pos;
}

void QSlideSwitchPrivate::updateSliderRect(const QSize& size)
{
	sliderShape.setWidth(size.width() / 2.0);
	sliderShape.setHeight(size.height()-1.0);
}

void QSlideSwitchPrivate::drawSlider(QPainter *painter)
{
	const qreal margin = 3;

	QRectF r = q_ptr->rect().adjusted(0, 0, -1, -1);
	qreal dx = (r.width() - sliderShape.width()) * pos;
	QRectF sliderRect = sliderShape.translated(dx, 0);
	
	painter->setPen(Qt::NoPen);

	//painter->setPen(Qt::black);
	//painter->setBrush(Qt::white);

	// basic settings
	QColor shadow = q_ptr->palette().color(QPalette::Dark);
	QColor light = q_ptr->palette().color(QPalette::Light);
	QColor button = q_ptr->palette().color(QPalette::Button);
	
	// draw background
	// draw background outer
	gradient.setColorAt(0, shadow.darker(130));
	gradient.setColorAt(1, light.darker(130));
	gradient.setStart(0, r.height());
	gradient.setFinalStop(0, 0);
	painter->setBrush(gradient);
	painter->drawRoundedRect(r, 15, 15);

	// draw background inner
	gradient.setColorAt(0, shadow.darker(140));
	gradient.setColorAt(1, light.darker(160));
	gradient.setStart(0, 0);
	gradient.setFinalStop(0, r.height());
	painter->setBrush(gradient);
	painter->drawRoundedRect(r.adjusted(margin, margin, -margin, -margin), 15, 15);

	// draw slider
	gradient.setColorAt(0, button.darker(130));
	gradient.setColorAt(1, button);
	
	// draw outer slider
	gradient.setStart(0, r.height());
	gradient.setFinalStop(0, 0);
	painter->setBrush(gradient);
	painter->drawRoundedRect(sliderRect.adjusted(margin, margin, -margin, -margin), 10, 15);

	// draw inner slider
	gradient.setStart(0, 0);
	gradient.setFinalStop(0, r.height());
	painter->setBrush(gradient);
	painter->drawRoundedRect(sliderRect.adjusted(2.5 * margin, 2.5 *  margin, -2.5 * margin, -2.5 * margin), 5, 15);

	// draw text
	if (animation->state() == QPropertyAnimation::Running)
		return; // don't draw any text while animation is running

	QFont font = q_ptr->font();
	gradient.setColorAt(0, light);
	gradient.setColorAt(1, shadow);
	gradient.setStart(0, r.height()/2.0 + font.pointSizeF());
	gradient.setFinalStop(0, r.height()/2.0 - font.pointSizeF());
	painter->setFont(font);
	painter->setPen(QPen(QBrush(gradient), 0));
	if (q_ptr->isChecked()) {
		painter->drawText(0, 0, r.width()/2, r.height()-1, Qt::AlignCenter, tr("ON"));
	} else {
		painter->drawText(r.width()/2, 0, r.width()/2, r.height()-1, Qt::AlignCenter, tr("OFF"));
	}
}

void QSlideSwitchPrivate::animate(bool checked)
{
	animation->setDirection(checked ? QPropertyAnimation::Forward : QPropertyAnimation::Backward);
	animation->start();
}



QSlideSwitch::QSlideSwitch(QWidget *parent /*= 0*/) : 
	QAbstractButton(parent), d_ptr(new QSlideSwitchPrivate(this))
{
	// animate changing button state
	connect(this, SIGNAL(clicked(bool)), d_ptr, SLOT(animate(bool))); 
	// tigger update on animation finished
	connect(d_ptr->animation, SIGNAL(finished()), this, SLOT(update()));
}

QSlideSwitch::~QSlideSwitch()
{
	delete d_ptr;
}

QSize QSlideSwitch::sizeHint() const
{
	return QSize(48, 28);
}

bool QSlideSwitch::hitButton(const QPoint& p) const
{
	return rect().contains(p);
}

void QSlideSwitch::paintEvent(QPaintEvent *e)
{
	Q_D(QSlideSwitch);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	d->drawSlider(&painter);
}

void QSlideSwitch::resizeEvent(QResizeEvent *e)
{
	Q_D(QSlideSwitch);
	d->updateSliderRect(e->size());
	repaint();
}





