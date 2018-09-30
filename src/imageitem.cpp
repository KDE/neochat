#include "imageitem.h"

#include <QBitmap>
#include <QGraphicsOpacityEffect>
#include <QRect>

ImageItem::ImageItem(QQuickItem *parent) : QQuickPaintedItem(parent) {}

void ImageItem::paint(QPainter *painter) {
  QRectF bounding_rect = boundingRect();

  painter->setRenderHint(QPainter::Antialiasing, true);

  if (m_image.isNull()) {
    painter->setPen(Qt::NoPen);
    if (m_color.isEmpty())
      painter->setBrush(QColor(stringtoColor(m_hint)));
    else
      painter->setBrush(QColor(m_color));
    if (m_round)
      painter->drawEllipse(0, 0, int(bounding_rect.width()),
                           int(bounding_rect.height()));
    else
      painter->drawRect(0, 0, int(bounding_rect.width()),
                        int(bounding_rect.height()));
    painter->setPen(QPen(Qt::white, 2));
    QFont font;
    font.setStyleHint(QFont::SansSerif);

    font.setPixelSize(int(bounding_rect.width() / 2));
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(
        QRect(0, 0, int(bounding_rect.width()), int(bounding_rect.height())),
        Qt::AlignCenter, m_hint.at(0).toUpper());
    return;
  }

  QImage scaled = m_image.scaled(
      int(bounding_rect.width()) + 1, int(bounding_rect.height()) + 1,
      Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

  QPointF center = bounding_rect.center() - scaled.rect().center();

  if (m_round) {
    QPainterPath clip;
    clip.addEllipse(
        0, 0, bounding_rect.width(),
        bounding_rect.height());  // this is the shape we want to clip to
    painter->setClipPath(clip);
  }

  if (center.x() < 0) center.setX(0);
  if (center.y() < 0) center.setY(0);

  painter->drawImage(center, scaled);
}

void ImageItem::setImage(const QImage &image) {
  m_image = image;
  emit imageChanged();
  update();
}

void ImageItem::setHint(QString newHint) {
  if (m_hint != newHint) {
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

QString ImageItem::stringtoColor(QString string) {
  int hash = 0;
  for (int i = 0; i < string.length(); i++)
    hash = string.at(i).unicode() + ((hash << 5) - hash);
  QString colour = "#";
  for (int j = 0; j < 3; j++)
    colour += ("00" + QString::number((hash >> (j * 8)) & 0xFF, 16)).right(2);
  return colour;
}
