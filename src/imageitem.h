#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QImage>
#include <QObject>
#include <QPainter>
#include <QQuickItem>
#include <QQuickPaintedItem>

#include "paintable.h"

class ImageItem : public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(Paintable* source READ paintable WRITE setPaintable NOTIFY
                 paintableChanged)
  Q_PROPERTY(QString hint READ hint WRITE setHint NOTIFY hintChanged)
  Q_PROPERTY(QString defaultColor READ defaultColor WRITE setDefaultColor NOTIFY
                 defaultColorChanged)
  Q_PROPERTY(bool round READ round WRITE setRound NOTIFY roundChanged)

 public:
  ImageItem(QQuickItem* parent = nullptr);

  void paint(QPainter* painter);

  Paintable* paintable() { return m_paintable; }
  void setPaintable(Paintable* paintable);

  QString hint() { return m_hint; }
  void setHint(QString hint);

  QString defaultColor() { return m_color; }
  void setDefaultColor(QString color);

  bool round() { return m_round; }
  void setRound(bool value);

 signals:
  void paintableChanged();
  void hintChanged();
  void defaultColorChanged();
  void roundChanged();

 private:
  Paintable* m_paintable = nullptr;
  QString m_hint = "H";
  QString m_color;
  bool m_round = true;

  QString stringtoColor(QString string);
  void paintHint(QPainter* painter, QRectF bounding_rect);
};

#endif  // IMAGEITEM_H
