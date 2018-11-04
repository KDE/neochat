#ifndef PAINTABLE_H
#define PAINTABLE_H

#include <QImage>
#include <QObject>

class Paintable : public QObject {
  Q_OBJECT
 public:
  Paintable(QObject* parent = nullptr);

  virtual QImage image(int) = 0;
  virtual QImage image(int, int) = 0;

 signals:
  void paintableChanged();
};

#endif  // PAINTABLE_H
