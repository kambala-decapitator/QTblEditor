#ifndef QTBLEDITOR_H
#define QTBLEDITOR_H

#include "ui_qtbleditor.h"

#include "editstringcelldialog.h"
#include "d2stringtablewidget.h"
#include "tablesdifferenceswidget.h"
#include "tblstructure.h"


class TablePanelWidget;
class FindReplaceDialog;
class QFile;

class QTblEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit QTblEditor(QWidget *parent = 0, Qt::WindowFlags flags = 0);

public slots:

signals:
    void tablesWereSwapped();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newTable();
    void open();
    void openRecentFile();
    void reopen();
    void sendToServer();
    bool closeTable(bool hideTable = true);
    bool closeAll(bool hideTable = true) { return closeTable(hideTable) && (!_openedTables || closeTable(hideTable)); }
    void save();
    void saveAs();
    void saveAll();
    void aboutApp();

    void changeText() { editString(_currentTableWidget->currentItem()); }
    void appendEntry() { increaseRowCount(_currentTableWidget->rowCount()); }
    void insertAfterCurrent() { increaseRowCount(_currentTableWidget->currentRow() + 1); }
    void deleteSelectedItems();
    void copy();
    void paste();
    void showFindReplaceDialog();
    void findNextString(const QString &query, bool isCaseSensitive, bool isExactString, bool isSearchBothTables);
    void changeCurrentTableItem(QTableWidgetItem *newItem);
    void goTo();

    void updateToolbarStateInMenu() { ui.actionToolbar->setChecked(ui.mainToolBar->isVisible()); }
    void toggleRowsHeight(bool isSmall);
	void changeKeyColumnVisibility(bool hide);

    void supplement();
    void swapTables();
    void activateAnotherTable();
    void showDifferences();
    void syncScrollingChanged(bool isSyncing);

    void editString(QTableWidgetItem *itemToEdit);
    void updateLocationLabel(int newRow);
    void changeCurrentTable(QWidget *newActiveTable);
    void updateWindow(bool isModified = true);
    void updateItem(QTableWidgetItem *item);
    void refreshDifferences(TablesDifferencesWidget *w);
    void modifyText(bool fromLeftToRight, bool append);

private:
    Ui::QTblEditorClass ui;
    QSplitter *_tableSplitter;
    TablePanelWidget *_leftTablePanelWidget, *_rightTablePanelWidget;
    D2StringTableWidget *_currentTableWidget, *_leftTableWidget, *_rightTableWidget;
    FindReplaceDialog *_findReplaceDlg;
    QLabel *_locationLabel, *_keyHashLabel;
    QActionGroup *_startNumberingGroup;

    quint8 _openedTables; // 0, 1 or 2
    QString _lastPath;
    QStringList _recentFilesList;
    bool _isTableLoaded;
    QHash<QString, QVariant> _lastSelectedRowsHash;


    void connectActions();
    int openTableMsgBoxResult();

    bool loadFile(const QString &fileName, bool shouldShowOpenOptions = true);
    bool processTable(const QString &fileName);
    bool processTblFile(QFile *inputFile);
    bool processTxtOrCsvFile(QFile *inputFile);

    bool wasSaved();
    void enableTableActions(bool state);
    bool isDialogQuestionConfirmed(const QString &text);

    bool saveFile(const QString &fileName);
    DWORD writeAsTbl(QByteArray &bytesToWrite);
    DWORD writeAsText(QByteArray &bytesToWrite, bool isCsv);

    void writeSettings();
    void readSettings();

    TablePanelWidget *currentTablePanelWidget() const;
    TablePanelWidget *inactiveNamedTableWidget(TablePanelWidget *namedTableToCheck) const;
    D2StringTableWidget *inactiveTableWidget(QTableWidget *tableToCheck) const;

    void tableMenuSetEnabled(bool isEnabled);
    void addToRecentFiles(const QString &fileName);
    void updateRecentFilesActions();
    void closeAllDialogs() { foreach (QDialog *d, findChildren<QDialog *>()) d->close(); }
    void increaseRowCount(int rowIndex);
    QStringList differentStrings(TablesDifferencesWidget::DiffType diffType) const;

    QString foldNewlines(const QString &s);
    QString restoreNewlines(const QString &s);
};

#endif // QTBLEDITOR_H
