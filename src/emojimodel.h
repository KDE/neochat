#ifndef EMOJIMODEL_H
#define EMOJIMODEL_H

#include <QObject>
#include <QVariant>
#include <QVector>

struct Emoji {
  Emoji(const QString& u, const QString& s) : unicode(u), shortname(s) {}
  Emoji() {}

  QString unicode;
  QString shortname;

  Q_GADGET
  Q_PROPERTY(QString unicode MEMBER unicode)
  Q_PROPERTY(QString shortname MEMBER shortname)
};

Q_DECLARE_METATYPE(Emoji)

class EmojiModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantMap model READ getModel CONSTANT)
 public:
  Q_INVOKABLE QVariantMap getModel();
  Q_INVOKABLE QVariantList filterModel(const QString& filter);

 private:
  static const QVariantList people;
  static const QVariantList nature;
  static const QVariantList food;
  static const QVariantList activity;
  static const QVariantList travel;
  static const QVariantList objects;
  static const QVariantList symbols;
  static const QVariantList flags;
};

#endif  // EMOJIMODEL_H
