#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QImage>
#include <QObject>
#include <QPainter>
#include <QQuickItem>
#include <QQuickPaintedItem>

class ImageItem : public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
  Q_PROPERTY(QString hint READ hint WRITE setHint NOTIFY hintChanged)
  Q_PROPERTY(QString defaultColor READ defaultColor WRITE setDefaultColor NOTIFY
                 defaultColorChanged)
  Q_PROPERTY(bool round READ round WRITE setRound NOTIFY roundChanged)

 public:
  ImageItem(QQuickItem *parent = nullptr);

  void paint(QPainter *painter);

  QImage image() const { return m_image; }
  void setImage(const QImage &image);

  QString hint() { return m_hint; }
  void setHint(QString hint);

  QString defaultColor() { return m_color; }
  void setDefaultColor(QString color);

  bool round() { return m_round; }
  void setRound(bool value);

 signals:
  void imageChanged();
  void hintChanged();
  void defaultColorChanged();
  void roundChanged();

 private:
  QImage m_image;
  QString m_hint = "H";
  QString m_color = "#000000";
  bool m_round = true;
};

#endif  // IMAGEITEM_H
