#ifndef IMAGECLIPBOARD_H
#define IMAGECLIPBOARD_H

#include <QClipboard>
#include <QImage>
#include <QObject>

class ImageClipboard : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool hasImage READ hasImage NOTIFY imageChanged)
  Q_PROPERTY(QImage image READ image NOTIFY imageChanged)

 public:
  explicit ImageClipboard(QObject* parent = nullptr);

  bool hasImage();
  QImage image();

 private:
  QClipboard* m_clipboard;

 signals:
  void imageChanged();

 public slots:
  void saveImage(const QUrl& localPath);
};

#endif  // IMAGECLIPBOARD_H
