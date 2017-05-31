#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include "ui_findreplacedialog.h"


class QDialog;
class QTableWidgetItem;
class QCloseEvent;

class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    FindReplaceDialog(QWidget *parent = 0);

    void show(bool isTwoTablesOpened);

public slots:
    void getFoundStrings(const QList<QTableWidgetItem *> &foundItems);
    void needsRefind() { _findConditionChanged = true; _currentStringIterator = QList<QTableWidgetItem *>::iterator(); }

private slots:
    void enableButtons();
    void getNextString(bool isPrevious);
    void findNext();
    void findPrevious();
    void replaceNext();
    void replaceAll();

signals:
    void getStrings(const QString &query, bool isCaseSensitive, bool isExactString, bool isSearchBothTables);
    void currentItemChanged(QTableWidgetItem *newItem);

protected:
    void closeEvent(QCloseEvent *e);

private:
    Ui::FindReplaceDialogClass ui;
    QString _query;
    bool _findConditionChanged, _searchFailed;
    QList<QTableWidgetItem *> _foundTableItems;
    QList<QTableWidgetItem *>::iterator _currentStringIterator;

    void readSettings();
    void writeSettings();
    bool areResultsObsolete();
    void replaceInCurrentString();
    void changeCurrentTableCell();
};

#endif // FINDREPLACEDIALOG_H
