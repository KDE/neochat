#ifndef EMOJIMODEL_H
#define EMOJIMODEL_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QVector>

struct Emoji {
  Emoji(const QString& u, const QString& s) : unicode(u), shortname(s) {}
  Emoji() {}

  friend QDataStream& operator<<(QDataStream& arch, const Emoji& object) {
    arch << object.unicode;
    arch << object.shortname;
    return arch;
  }

  friend QDataStream& operator>>(QDataStream& arch, Emoji& object) {
    arch >> object.unicode;
    arch >> object.shortname;
    return arch;
  }

  QString unicode;
  QString shortname;

  Q_GADGET
  Q_PROPERTY(QString unicode MEMBER unicode)
  Q_PROPERTY(QString shortname MEMBER shortname)
};

Q_DECLARE_METATYPE(Emoji)

class EmojiModel : public QObject {
  Q_OBJECT

  Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)

  Q_PROPERTY(QVariantList people MEMBER people CONSTANT)
  Q_PROPERTY(QVariantList nature MEMBER nature CONSTANT)
  Q_PROPERTY(QVariantList food MEMBER food CONSTANT)
  Q_PROPERTY(QVariantList activity MEMBER activity CONSTANT)
  Q_PROPERTY(QVariantList travel MEMBER travel CONSTANT)
  Q_PROPERTY(QVariantList objects MEMBER objects CONSTANT)
  Q_PROPERTY(QVariantList symbols MEMBER symbols CONSTANT)
  Q_PROPERTY(QVariantList flags MEMBER flags CONSTANT)

 public:
  explicit EmojiModel(QObject* parent = nullptr)
      : QObject(parent), m_settings(new QSettings()) {}

  Q_INVOKABLE QVariantList history();
  Q_INVOKABLE QVariantList filterModel(const QString& filter);

 signals:
  void historyChanged();

 public slots:
  void emojiUsed(QVariant modelData);

 private:
  static const QVariantList people;
  static const QVariantList nature;
  static const QVariantList food;
  static const QVariantList activity;
  static const QVariantList travel;
  static const QVariantList objects;
  static const QVariantList symbols;
  static const QVariantList flags;

  QSettings* m_settings;
};

#endif  // EMOJIMODEL_H
