#ifndef EMOJIMODEL_H
#define EMOJIMODEL_H

#include <QObject>
#include <QVariant>

class EmojiModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariant model READ getModel NOTIFY categoryChanged)
  Q_PROPERTY(QString category READ getCategory WRITE setCategory NOTIFY
                 categoryChanged)
 public:
  explicit EmojiModel(QObject *parent = nullptr);

  QVariant getModel();

  QString getCategory() { return m_category; }
  void setCategory(QString category) {
    if (category != m_category) {
      m_category = category;
      emit categoryChanged();
    }
  }

 private:
  static const QStringList people;
  static const QStringList nature;
  static const QStringList food;
  static const QStringList activity;
  static const QStringList travel;
  static const QStringList objects;
  static const QStringList symbols;
  static const QStringList flags;

  QString m_category = "people";

 signals:
  void categoryChanged();

 public slots:
};

#endif  // EMOJIMODEL_H
