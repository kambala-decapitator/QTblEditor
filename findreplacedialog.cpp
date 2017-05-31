#include "findreplacedialog.h"
#include "tablepanelwidget.h"

#include <QMessageBox>
#include <QDialog>
#include <QTableWidgetItem>
#include <QCloseEvent>

#include <QSettings>


FindReplaceDialog::FindReplaceDialog(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui.buttonFindNext->setToolTip(ui.buttonFindNext->shortcut().toString(QKeySequence::NativeText));
    ui.buttonFindPrevious->setToolTip(ui.buttonFindPrevious->shortcut().toString(QKeySequence::NativeText));
    ui.buttonReplaceAll->setToolTip(ui.buttonReplaceAll->shortcut().toString(QKeySequence::NativeText));
    ui.buttonReplaceNext->setToolTip(ui.buttonReplaceNext->shortcut().toString(QKeySequence::NativeText));


    connect(ui.lineEditReplace, SIGNAL(textChanged(QString)), SLOT(enableButtons()));
    connect(ui.lineEditFind, SIGNAL(textChanged(QString)), SLOT(enableButtons()));
    connect(ui.lineEditFind, SIGNAL(textChanged(QString)), SLOT(needsRefind()));

    connect(ui.checkBoxCaseSensitive, SIGNAL(toggled(bool)), SLOT(needsRefind()));
    connect(ui.checkBoxExactMatch, SIGNAL(toggled(bool)), SLOT(needsRefind()));
    connect(ui.checkBoxOnlyActiveTable, SIGNAL(toggled(bool)), SLOT(needsRefind()));

    connect(ui.buttonFindNext, SIGNAL(clicked()), SLOT(findNext()));
    connect(ui.buttonFindPrevious, SIGNAL(clicked()), SLOT(findPrevious()));
    connect(ui.buttonReplaceNext, SIGNAL(clicked()), SLOT(replaceNext()));
    connect(ui.buttonReplaceAll, SIGNAL(clicked()), SLOT(replaceAll()));


    resize(sizeHint());
    readSettings();
}

void FindReplaceDialog::readSettings()
{
    QSettings s;
    s.beginGroup("findReplaceDialog");
    ui.lineEditFind->setText(s.value("lastFindString").toString());
    ui.lineEditReplace->setText(s.value("lastReplaceString").toString());
    ui.checkBoxCaseSensitive->setChecked(s.value("isCaseSensitive").toBool());
    ui.checkBoxExactMatch->setChecked(s.value("isExactMatch").toBool());
    ui.checkBoxOnlyActiveTable->setChecked(s.value("isSearchOnlyActiveTable", true).toBool());
    s.endGroup();
}

void FindReplaceDialog::writeSettings()
{
    QSettings s;
    s.beginGroup("findReplaceDialog");
    if (!ui.lineEditFind->text().isEmpty())
        s.setValue("lastFindString", ui.lineEditFind->text());
    if (!ui.lineEditReplace->text().isEmpty())
        s.setValue("lastReplaceString", ui.lineEditReplace->text());
    s.setValue("isCaseSensitive", ui.checkBoxCaseSensitive->isChecked());
    s.setValue("isExactMatch", ui.checkBoxExactMatch->isChecked());
    s.setValue("isSearchOnlyActiveTable", ui.checkBoxOnlyActiveTable->isChecked());
    s.endGroup();
}

void FindReplaceDialog::closeEvent(QCloseEvent *e)
{
    writeSettings();
    e->accept();
}

void FindReplaceDialog::show(bool isTwoTablesOpened)
{
    ui.lineEditFind->setFocus();
    ui.lineEditFind->selectAll();
    ui.checkBoxOnlyActiveTable->setEnabled(isTwoTablesOpened);
    QDialog::show();
}

void FindReplaceDialog::enableButtons()
{
    bool isFindEqualToReplace = ui.lineEditFind->text() != ui.lineEditReplace->text(), isTextNotEmpty = !ui.lineEditFind->text().isEmpty();
    ui.buttonFindNext->setEnabled(isTextNotEmpty);
    ui.buttonFindPrevious->setEnabled(isTextNotEmpty);
    ui.buttonReplaceAll->setEnabled(isTextNotEmpty && isFindEqualToReplace);
    ui.buttonReplaceNext->setEnabled(isTextNotEmpty && isFindEqualToReplace);
}

bool FindReplaceDialog::areResultsObsolete()
{
    if (_query.isEmpty() || _query != ui.lineEditFind->text() || _findConditionChanged)
    {
        _query = ui.lineEditFind->text();
        emit getStrings(_query, ui.checkBoxCaseSensitive->isChecked(), ui.checkBoxExactMatch->isChecked(),
                        ui.checkBoxOnlyActiveTable->isEnabled() && !ui.checkBoxOnlyActiveTable->isChecked());
        return true;
    }
    return false;
}

void FindReplaceDialog::getNextString(bool isPrevious)
{
    if (areResultsObsolete())
        return;

    if (!_searchFailed)
    {
        if (isPrevious) // get previous
        {
            if (_currentStringIterator == _foundTableItems.begin())
            {
                qApp->beep();
                _currentStringIterator = _foundTableItems.end() - 1;
            }
            else
                _currentStringIterator--;
        }
        else // get next
        {
            if (_currentStringIterator + 1 == _foundTableItems.end())
            {
                qApp->beep();
                _currentStringIterator = _foundTableItems.begin();
            }
            else
                _currentStringIterator++;
        }

        emit currentItemChanged(*_currentStringIterator);
    }
    else
    {
        qApp->beep();
        QMessageBox::information(this, tr("Lrn2read!"), tr("You've already been told that there's no string \"%1\"").arg(_query));
    }
}

void FindReplaceDialog::findNext()
{
    getNextString(false);
}

void FindReplaceDialog::findPrevious()
{
    getNextString(true);
}

void FindReplaceDialog::getFoundStrings(const QList<QTableWidgetItem *> &foundItems)
{
    _findConditionChanged = false;

    if (!foundItems.size())
    {
        qApp->beep();
        QMessageBox::information(this, qApp->applicationName(), tr("String \"%1\" not found").arg(_query));
        _searchFailed = true;
        return;
    }

    _searchFailed = false;
    _foundTableItems = foundItems;
    _currentStringIterator = _foundTableItems.begin();

    emit currentItemChanged(*_currentStringIterator);
}

void FindReplaceDialog::replaceNext()
{
    bool notSearched = !_currentStringIterator.i;
    if (notSearched)
        findNext();
    if (!_searchFailed)
        replaceInCurrentString();
    if (!(notSearched && _searchFailed))
        findNext();
}

void FindReplaceDialog::replaceAll()
{
    areResultsObsolete();
    if (_searchFailed)
        return;

    int n = 0;
    for (_currentStringIterator = _foundTableItems.begin(); _currentStringIterator != _foundTableItems.end(); _currentStringIterator++, n++)
        replaceInCurrentString();
    QMessageBox::information(this, tr("Replace"), tr("%n occurrence(s) replaced", 0, n));

    _query.clear();
}

void FindReplaceDialog::replaceInCurrentString()
{
    QTableWidgetItem *itemToReplaceIn = *_currentStringIterator;
    QString replaceIn = itemToReplaceIn->text(), replaceWith = ui.lineEditReplace->text();
    Qt::CaseSensitivity cs = (Qt::CaseSensitivity)ui.checkBoxCaseSensitive->isChecked();
    int occurencesCount = replaceIn.count(_query, cs), position = 0;
    for (int i = 0; i < occurencesCount; i++)
    {
        position = replaceIn.indexOf(_query, position, cs);
        itemToReplaceIn->setText(replaceIn.replace(position, _query.length(), replaceWith));
    }
}
