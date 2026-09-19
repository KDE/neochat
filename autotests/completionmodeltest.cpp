// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QAbstractItemModelTester>
#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <qurl.h>

#include "completionlist.h"
#include "completionmodel.h"

using namespace Qt::Literals::StringLiterals;

class TestCompletionList : public CompletionList
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TestCompletionList(QObject *parent = nullptr, qsizetype size = 0, const QString &listName = {})
        : CompletionList(parent)
        , m_size(size)
        , m_listName(listName) { };

    qsizetype size() const override
    {
        return m_size;
    }

    std::optional<Completion> at(qsizetype i) const override
    {
        if (i < 0 || i >= m_size) {
            return std::nullopt;
        }
        return Completion{
            .title = u"Title %1 Index %2"_s.arg(m_listName, QString::number(i)),
            .description = u"Description %1 Index %2"_s.arg(m_listName, QString::number(i)),
            .avatarSource = QUrl(u"Avatar %1 Index %2"_s.arg(m_listName, QString::number(i))),
            .startsequence = u"Start Sequence %1 Index %2"_s.arg(m_listName, QString::number(i)),
            .matchSequences = {u"Match Sequence %1 Index %2"_s.arg(m_listName, QString::number(i))},
            .replaceString = u"Replace String %1 Index %2"_s.arg(m_listName, QString::number(i)),
            .hRef = QUrl(u"hRef %1 Index %2"_s.arg(m_listName, QString::number(i))),
        };
    }

private:
    qsizetype m_size;
    QString m_listName;
};

class CompletionModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void multipleLists();
};

void CompletionModelTest::multipleLists()
{
    auto model = new CompletionModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);

    const auto list1 = new TestCompletionList(this, 4, u"List 1"_s);
    const auto list2 = new TestCompletionList(this, 3, u"List 2"_s);
    model->setCompletionLists({list1, list2});

    QCOMPARE(model->rowCount(), 7);

    // This item should be in list 1
    QCOMPARE(model->index(0).data(CompletionModel::TitleRole), u"Title List 1 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::DescriptionRole), u"Description List 1 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::AvatarSourceRole), QUrl(u"Avatar List 1 Index 0"_s));
    QCOMPARE(model->index(0).data(CompletionModel::StartSequenceRole), u"Start Sequence List 1 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::MatchSequencesRole), QStringList{u"Match Sequence List 1 Index 0"_s});
    QCOMPARE(model->index(0).data(CompletionModel::ReplaceStringRole), u"Replace String List 1 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::HRefRole), QUrl(u"hRef List 1 Index 0"_s));

    // This item should be in list 2
    QCOMPARE(model->index(4).data(CompletionModel::TitleRole), u"Title List 2 Index 0"_s);
    QCOMPARE(model->index(4).data(CompletionModel::DescriptionRole), u"Description List 2 Index 0"_s);
    QCOMPARE(model->index(4).data(CompletionModel::AvatarSourceRole), QUrl(u"Avatar List 2 Index 0"_s));
    QCOMPARE(model->index(4).data(CompletionModel::StartSequenceRole), u"Start Sequence List 2 Index 0"_s);
    QCOMPARE(model->index(4).data(CompletionModel::MatchSequencesRole), QStringList{u"Match Sequence List 2 Index 0"_s});
    QCOMPARE(model->index(4).data(CompletionModel::ReplaceStringRole), u"Replace String List 2 Index 0"_s);
    QCOMPARE(model->index(4).data(CompletionModel::HRefRole), QUrl(u"hRef List 2 Index 0"_s));
}

QTEST_MAIN(CompletionModelTest)
#include "completionmodeltest.moc"
