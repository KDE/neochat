// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QAbstractItemModelTester>
#include <QObject>
#include <QTest>

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
        , m_listName(listName)
    {
    }

    qsizetype size() const override
    {
        return m_size;
    }

    QVariant data(qsizetype row, int role = Qt::DisplayRole) const override
    {
        if (row < 0 || row >= m_size) {
            return {};
        }
        switch (role) {
        case CompletionModel::TitleRole:
            return u"Title %1 Index %2"_s.arg(m_listName, QString::number(row));
        case CompletionModel::DescriptionRole:
            return u"Description %1 Index %2"_s.arg(m_listName, QString::number(row));
        case CompletionModel::AvatarSourceRole:
            return QUrl(u"Avatar %1 Index %2"_s.arg(m_listName, QString::number(row)));
        case CompletionModel::StartSequenceRole:
            return startSequence();
        case CompletionModel::MatchSequencesRole:
            return QStringList{u"Match Sequence %1 Index %2"_s.arg(m_listName, QString::number(row))};
        case CompletionModel::ReplaceStringRole:
            return u"Replace String %1 Index %2"_s.arg(m_listName, QString::number(row));
        case CompletionModel::HRefRole:
            return QUrl(u"hRef %1 Index %2"_s.arg(m_listName, QString::number(row)));
        default:
            return {};
        }
    }

    QString startSequence() const override
    {
        return m_listName;
    }

    void setSize(qsizetype size)
    {
        if (size > m_size) {
            Q_EMIT CompletionList::completionsAdded(this, m_size, size - 1);
            m_size = size;
        } else if (size < m_size) {
            Q_EMIT CompletionList::completionsRemoved(this, size, m_size - 1);
            m_size = size;
        }
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
    void longCurrentText();
};

void CompletionModelTest::multipleLists()
{
    auto model = new CompletionModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);

    const auto list1 = new TestCompletionList(this, 4, u"1"_s);
    const auto list2 = new TestCompletionList(this, 3, u"2"_s);
    const auto list3 = new TestCompletionList(this, 5, u"3"_s);
    const auto list4 = new TestCompletionList(this, 2, u"3"_s);
    // large fake data to make sure to produce invalid indexes if wrongly considered part of the model data
    const auto list5 = new TestCompletionList(this, 1000, u"5"_s);
    model->setCompletionLists({list1, list2, list3, list4, list5});

    QCOMPARE(model->rowCount(), 0);

    model->setCurrentText(u"1"_s);
    QCOMPARE(model->rowCount(), 4);

    // This item should be in list 1
    QCOMPARE(model->index(0).data(CompletionModel::TitleRole), u"Title 1 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::DescriptionRole), u"Description 1 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::AvatarSourceRole), QUrl(u"Avatar 1 Index 0"_s));
    QCOMPARE(model->index(0).data(CompletionModel::StartSequenceRole), u"1"_s);
    QCOMPARE(model->index(0).data(CompletionModel::MatchSequencesRole), QStringList{u"Match Sequence 1 Index 0"_s});
    QCOMPARE(model->index(0).data(CompletionModel::ReplaceStringRole), u"Replace String 1 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::HRefRole), QUrl(u"hRef 1 Index 0"_s));

    model->setCurrentText(u"2"_s);
    QCOMPARE(model->rowCount(), 3);

    // This item should be in list 2
    QCOMPARE(model->index(0).data(CompletionModel::TitleRole), u"Title 2 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::DescriptionRole), u"Description 2 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::AvatarSourceRole), QUrl(u"Avatar 2 Index 0"_s));
    QCOMPARE(model->index(0).data(CompletionModel::StartSequenceRole), u"2"_s);
    QCOMPARE(model->index(0).data(CompletionModel::MatchSequencesRole), QStringList{u"Match Sequence 2 Index 0"_s});
    QCOMPARE(model->index(0).data(CompletionModel::ReplaceStringRole), u"Replace String 2 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::HRefRole), QUrl(u"hRef 2 Index 0"_s));

    model->setCurrentText(u"3"_s);
    QCOMPARE(model->rowCount(), 7);

    // This item should be in list 3 index 0
    QCOMPARE(model->index(0).data(CompletionModel::TitleRole), u"Title 3 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::DescriptionRole), u"Description 3 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::AvatarSourceRole), QUrl(u"Avatar 3 Index 0"_s));
    QCOMPARE(model->index(0).data(CompletionModel::StartSequenceRole), u"3"_s);
    QCOMPARE(model->index(0).data(CompletionModel::MatchSequencesRole), QStringList{u"Match Sequence 3 Index 0"_s});
    QCOMPARE(model->index(0).data(CompletionModel::ReplaceStringRole), u"Replace String 3 Index 0"_s);
    QCOMPARE(model->index(0).data(CompletionModel::HRefRole), QUrl(u"hRef 3 Index 0"_s));

    // This item should be in list 4 index 0
    QCOMPARE(model->index(5).data(CompletionModel::TitleRole), u"Title 3 Index 0"_s);
    QCOMPARE(model->index(5).data(CompletionModel::DescriptionRole), u"Description 3 Index 0"_s);
    QCOMPARE(model->index(5).data(CompletionModel::AvatarSourceRole), QUrl(u"Avatar 3 Index 0"_s));
    QCOMPARE(model->index(5).data(CompletionModel::StartSequenceRole), u"3"_s);
    QCOMPARE(model->index(5).data(CompletionModel::MatchSequencesRole), QStringList{u"Match Sequence 3 Index 0"_s});
    QCOMPARE(model->index(5).data(CompletionModel::ReplaceStringRole), u"Replace String 3 Index 0"_s);
    QCOMPARE(model->index(5).data(CompletionModel::HRefRole), QUrl(u"hRef 3 Index 0"_s));

    // These should not cause any change visible in the QAbstractItemModel interface of CompletionModel, in particular
    // no signals or calls that mention indexes not actually in the model.
    list5->setSize(2000);
    QCOMPARE(model->rowCount(), 7);
    list5->setSize(500);
    QCOMPARE(model->rowCount(), 7);
}

void CompletionModelTest::longCurrentText()
{
    auto model = new CompletionModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);

    const auto list1 = new TestCompletionList(this, 4, u"1"_s);
    model->setCompletionLists({list1});

    QCOMPARE(model->rowCount(), 0);

    model->setCurrentText(u"1test"_s);
    QCOMPARE(model->rowCount(), 4);

    model->setCurrentText(u"1testtest"_s);
    QCOMPARE(model->rowCount(), 4);

    model->setCurrentText({});
    QCOMPARE(model->rowCount(), 0);
}

QTEST_MAIN(CompletionModelTest)
#include "completionmodeltest.moc"
