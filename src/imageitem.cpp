#include "imageitem.h"

#include <QApplication>
#include <QBitmap>
#include <QGraphicsOpacityEffect>
#include <QRect>
#include <QScreen>

ImageItem::ImageItem(QQuickItem *parent) : QQuickPaintedItem(parent) {}

inline static QString stringtoColor(QString string) {
  int hash = 0;
  for (int i = 0; i < string.length(); i++)
    hash = string.at(i).unicode() + ((hash << 5) - hash);
  QString colour = "#";
  for (int j = 0; j < 3; j++)
    colour += ("00" + QString::number((hash >> (j * 8)) & 0xFF, 16)).right(2);
  return colour;
}

inline static QImage getImageFromPaintable(QPointer<Paintable> p, int width,
                                           int height) {
  if (p.isNull()) return {};
  qreal dpi = QApplication::primaryScreen()->devicePixelRatio();
  QImage image(p->image(width * dpi, height * dpi));
  if (image.isNull()) return {};
  return image;
}

void ImageItem::paint(QPainter *painter) {
  QRectF bounding_rect(boundingRect());

  painter->setRenderHint(QPainter::Antialiasing, true);

  QImage image(getImageFromPaintable(m_paintable, bounding_rect.width(),
                                     bounding_rect.height()));

  if (image.isNull()) {
    painter->setPen(Qt::NoPen);
    if (m_color.isEmpty())
      painter->setBrush(QColor(stringtoColor(m_hint)));
    else
      painter->setBrush(QColor(m_color));
    if (m_round)
      painter->drawEllipse(bounding_rect);
    else
      painter->drawRect(bounding_rect);
    painter->setPen(QPen(Qt::white, 2));
    QFont font;
    font.setStyleHint(QFont::SansSerif);

    font.setPixelSize(int(bounding_rect.width() / 2));
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(bounding_rect, Qt::AlignCenter, m_hint.at(0).toUpper());
  } else {
    if (m_round) {
      QPainterPath clip;
      clip.addEllipse(bounding_rect);  // this is the shape we want to clip to
      painter->setClipPath(clip);
    }

    painter->drawImage(bounding_rect, image);
  }
}

void ImageItem::setPaintable(Paintable *paintable) {
  if (!paintable) return;
  if (!m_paintable.isNull()) m_paintable->disconnect(this);
  m_paintable = paintable;
  connect(m_paintable, &Paintable::paintableChanged, this,
          [=] { this->update(); });
  emit paintableChanged();
  update();
}

void ImageItem::setHint(QString newHint) {
  if (!m_hint.isNull() && m_hint != newHint) {
    m_hint = newHint;
    emit hintChanged();
    update();
  }
}

void ImageItem::setDefaultColor(QString color) {
  if (color != m_color) {
    m_color = color;
    emit defaultColorChanged();
    update();
  }
}

void ImageItem::setRound(bool value) {
  if (m_round != value) {
    m_round = value;
    emit roundChanged();
    update();
  }
}
