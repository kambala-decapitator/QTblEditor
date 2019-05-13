#include "qtbleditor.h"
#include "gotorowdialog.h"
#include "tablepanelwidget.h"
#include "findreplacedialog.h"

#include <QMainWindow>
#include <QCloseEvent>
#include <QLabel>
#include <QSplitter>
#include <QMessageBox>
#include <QFileDialog>
#include <QSpinBox>
#include <QTextEdit>
#include <QInputDialog>
#include <QScrollBar>
#include <QClipboard>
#include <QListWidget>

#include <QTextStream>
#include <QSettings>
#include <QFileInfo>
#include <QMimeData>
#include <QDateTime>

#include <QNetworkReply>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


static const QString kNewTblFileName("!newstring!.tbl"), kCustomColorsFileName("customcolors.ini");
const int kMaxRecentFiles = 10;


// global auxiliary functions

#define CONNECT_ACTION_TO_SLOT(action, slot) connect(action, SIGNAL(triggered()), slot)

QDataStream &operator <<(QDataStream &out, const TblHeader &th)
{
    return out << th.CRC << th.NodesNumber << th.HashTableSize << th.Version << th.DataStartOffset << th.HashMaxTries
        << th.FileSize;
}

QDataStream &operator <<(QDataStream &out, const TblHashNode &tn)
{
    return out << tn.Active << tn.Index << tn.HashValue << tn.StringKeyOffset << tn.StringValOffset << tn.StringValLength;
}

QString stripSurroundingQuotes(const QString &s)
{
    return s.startsWith('\"') && s.endsWith('\"') ? s.mid(1, s.length() - 2) : s;
}

// end of global auxiliary functions


extern QList<QChar> colorCodes;
extern QStringList colorStrings;
extern QList<QColor> colors;
extern QString colorHeader;
extern int colorsNum;

QTblEditor::QTblEditor(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags), _openedTables(0), _isTableLoaded(false)
{
    ui.setupUi(this);
    ui.mainToolBar->setWindowTitle(tr("Toolbar"));

#ifdef Q_OS_MAC
    ui.actionInsertAfterCurrent->setShortcut(QKeySequence("+"));
    ui.actionAppendEntry->setShortcut(QKeySequence("Ctrl++"));
    ui.actionGoTo->setShortcut(QKeySequence("Ctrl+L"));
#else
    ui.actionGoTo->setShortcut(QKeySequence("Ctrl+G"));
#endif
    ui.actionSwap->setShortcut(QKeySequence("Meta+Tab"));
    ui.actionChangeActive->setShortcut(QKeySequence("Tab"));

    QActionGroup *csvSeparators = new QActionGroup(this);
    csvSeparators->addAction(ui.actionCsvComma);
    csvSeparators->addAction(ui.actionCsvSemiColon);

    QList<QAction *> diffActions = QList<QAction *>() << ui.actionKeys << ui.actionStrings << ui.actionBoth << ui.actionSameStrings;
    for (int i = 0; i < diffActions.size(); ++i)
        diffActions[i]->setData(i);

    _locationLabel = new QLabel("00000 (0x00000) / 00000 (0x00000)", this);
    _locationLabel->setToolTip(tr("Current row / total rows"));
    _locationLabel->setAlignment(Qt::AlignHCenter);
    _locationLabel->setMinimumSize(_locationLabel->sizeHint());
    _locationLabel->clear();

    _keyHashLabel = new QLabel("0x00000 | 0x00000", this);
    _keyHashLabel->setToolTip(tr("Key hash value"));
    _keyHashLabel->setAlignment(Qt::AlignHCenter);
    _keyHashLabel->setMinimumSize(_keyHashLabel->sizeHint());
    _keyHashLabel->clear();

#ifndef Q_OS_MAC
    ui.statusBar->addWidget(new QLabel, 1);
#endif
    ui.statusBar->addPermanentWidget(_keyHashLabel);
    ui.statusBar->addPermanentWidget(_locationLabel);

    _leftTablePanelWidget = new TablePanelWidget(this);
    _rightTablePanelWidget = new TablePanelWidget(this);
    _leftTableWidget = _leftTablePanelWidget->tableWidget();
    _rightTableWidget = _rightTablePanelWidget->tableWidget();
    _rightTablePanelWidget->hide();

    _tableSplitter = new QSplitter(Qt::Horizontal, this);
    _tableSplitter->addWidget(_leftTablePanelWidget);
    _tableSplitter->addWidget(_rightTablePanelWidget);
    _tableSplitter->setChildrenCollapsible(false);
    setCentralWidget(_tableSplitter);

    _findReplaceDlg = new FindReplaceDialog(this);

    _startNumberingGroup = new QActionGroup(this);
    _startNumberingGroup->addAction(ui.actionStartNumberingFrom0);
    _startNumberingGroup->addAction(ui.actionStartNumberingFrom1);


    connectActions();

    connect(ui.menuView, SIGNAL(aboutToShow()), SLOT(updateToolbarStateInMenu()));

    connect(_leftTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), SLOT(editString(QTableWidgetItem *)));
    connect(_leftTableWidget, SIGNAL(currentCellChanged(int, int, int, int)), SLOT(updateLocationLabel(int)));
    connect(_leftTableWidget, SIGNAL(tableGotFocus(QWidget *)), SLOT(changeCurrentTable(QWidget *)));
    connect(_leftTableWidget, SIGNAL(itemChanged(QTableWidgetItem *)), SLOT(updateItem(QTableWidgetItem *)));

    connect(_rightTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), SLOT(editString(QTableWidgetItem *)));
    connect(_rightTableWidget, SIGNAL(currentCellChanged(int, int, int, int)), SLOT(updateLocationLabel(int)));
    connect(_rightTableWidget, SIGNAL(tableGotFocus(QWidget *)), SLOT(changeCurrentTable(QWidget *)));
    connect(_rightTableWidget, SIGNAL(itemChanged(QTableWidgetItem *)), SLOT(updateItem(QTableWidgetItem *)));

    connect(ui.actionShowHexInRow, SIGNAL(toggled(bool)), _leftTableWidget, SLOT(toggleDisplayHex(bool)));
    connect(ui.actionShowHexInRow, SIGNAL(toggled(bool)), _rightTableWidget, SLOT(toggleDisplayHex(bool)));
    connect(ui.actionStartNumberingFrom1, SIGNAL(toggled(bool)), _leftTableWidget, SLOT(changeRowNumberingTo1(bool)));
    connect(ui.actionStartNumberingFrom1, SIGNAL(toggled(bool)), _rightTableWidget, SLOT(changeRowNumberingTo1(bool)));

    connect(_findReplaceDlg, SIGNAL(getStrings(QString, bool, bool, bool)), SLOT(findNextString(QString, bool, bool, bool)));
    connect(_findReplaceDlg, SIGNAL(currentItemChanged(QTableWidgetItem *)), SLOT(changeCurrentTableItem(QTableWidgetItem *)));
    

    _currentTableWidget = _leftTableWidget;
    readSettings();
    updateRecentFilesActions();

    _leftTableWidget->changeRowHeaderDisplay();
    _rightTableWidget->changeRowHeaderDisplay();
}

void QTblEditor::connectActions()
{
    CONNECT_ACTION_TO_SLOT(ui.actionNew, SLOT(newTable()));
    CONNECT_ACTION_TO_SLOT(ui.actionOpen, SLOT(open()));
    CONNECT_ACTION_TO_SLOT(ui.actionReopen, SLOT(reopen()));
    CONNECT_ACTION_TO_SLOT(ui.actionSendToServer, SLOT(sendToServer()));
    CONNECT_ACTION_TO_SLOT(ui.actionSave, SLOT(save()));
    CONNECT_ACTION_TO_SLOT(ui.actionSaveAs, SLOT(saveAs()));
    CONNECT_ACTION_TO_SLOT(ui.actionSaveAll, SLOT(saveAll()));
    CONNECT_ACTION_TO_SLOT(ui.actionClose, SLOT(closeTable()));
    CONNECT_ACTION_TO_SLOT(ui.actionCloseAll, SLOT(closeAll()));

    CONNECT_ACTION_TO_SLOT(ui.actionChangeText, SLOT(changeText()));
    CONNECT_ACTION_TO_SLOT(ui.actionAppendEntry, SLOT(appendEntry()));
    CONNECT_ACTION_TO_SLOT(ui.actionInsertAfterCurrent, SLOT(insertAfterCurrent()));
    CONNECT_ACTION_TO_SLOT(ui.actionClearSelected, SLOT(deleteSelectedItems()));
    CONNECT_ACTION_TO_SLOT(ui.actionDeleteSelected, SLOT(deleteSelectedItems()));
    CONNECT_ACTION_TO_SLOT(ui.actionCopy, SLOT(copy()));
    CONNECT_ACTION_TO_SLOT(ui.actionPaste, SLOT(paste()));
    CONNECT_ACTION_TO_SLOT(ui.actionFindReplace, SLOT(showFindReplaceDialog()));
    CONNECT_ACTION_TO_SLOT(ui.actionGoTo, SLOT(goTo()));

    connect(ui.actionToolbar, SIGNAL(toggled(bool)), ui.mainToolBar, SLOT(setVisible(bool)));
    connect(ui.actionSmallRows, SIGNAL(toggled(bool)), SLOT(toggleRowsHeight(bool)));
	connect(ui.actionHideKeyColumn, SIGNAL(toggled(bool)), SLOT(changeKeyColumnVisibility(bool)));

    CONNECT_ACTION_TO_SLOT(ui.actionSupplement, SLOT(supplement()));
    CONNECT_ACTION_TO_SLOT(ui.actionSwap, SLOT(swapTables()));
    CONNECT_ACTION_TO_SLOT(ui.actionChangeActive, SLOT(activateAnotherTable()));
    CONNECT_ACTION_TO_SLOT(ui.actionKeys, SLOT(showDifferences()));
    CONNECT_ACTION_TO_SLOT(ui.actionStrings, SLOT(showDifferences()));
    CONNECT_ACTION_TO_SLOT(ui.actionBoth, SLOT(showDifferences()));
    CONNECT_ACTION_TO_SLOT(ui.actionSameStrings, SLOT(showDifferences()));
    connect(ui.actionSyncScrolling, SIGNAL(toggled(bool)), SLOT(syncScrollingChanged(bool)));

    CONNECT_ACTION_TO_SLOT(ui.actionAbout, SLOT(aboutApp()));
    connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void QTblEditor::updateRecentFilesActions()
{
    ui.menuRecentFiles->clear();
    if (!_recentFilesList.isEmpty())
    {
        ui.menuRecentFiles->setEnabled(true);
        QMutableStringListIterator i(_recentFilesList);
        while (i.hasNext())
        {
            QString filePath = i.next();
            if (QFile::exists(filePath))
                ui.menuRecentFiles->addAction(
#ifdef Q_OS_WIN32
                        QDir::toNativeSeparators(filePath),
#else
                        filePath,
#endif
                        this, SLOT(openRecentFile()));
            else
                i.remove();
        }
    }
    else
        ui.menuRecentFiles->setEnabled(false);
}

int QTblEditor::openTableMsgBoxResult()
{
    QMessageBox openOptsDlg(this);
    openOptsDlg.setIcon(QMessageBox::Question);
    openOptsDlg.setDefaultButton(openOptsDlg.addButton(tr("Open one more"), QMessageBox::AcceptRole));
    openOptsDlg.setEscapeButton(openOptsDlg.addButton(QMessageBox::Cancel));
    openOptsDlg.addButton(tr("Replace active"), QMessageBox::NoRole);
    openOptsDlg.setText(tr("How do you want to open the table?"));
#ifdef Q_OS_MAC
    openOptsDlg.setWindowModality(Qt::WindowModal);
#endif
    return openOptsDlg.exec();
}

void QTblEditor::tableMenuSetEnabled(bool isEnabled)
{
    ui.menuTables->setEnabled(isEnabled);
    foreach (QAction *action, ui.menuTables->actions())
        action->setEnabled(isEnabled);
}

void QTblEditor::newTable()
{
    bool okPressed;
    int rowsNum = QInputDialog::getInt(this, tr("New table"), tr("Enter desired number of rows (1-100000):"), 1, 1, 100000, 1, &okPressed);
    if (okPressed)
    {
        bool areTwoTablesOpened = _openedTables == 2;
        int result = -1;
        if (_openedTables == 1)
        {
            result = openTableMsgBoxResult(); // 0 - add one more, 1 - replace active, rest - cancel
            if (result != 0 && result != 1)
                return;
        }
        if (result && !currentTablePanelWidget()->absoluteFileName().isEmpty() && !closeTable())
            return;

        if (!result || (result == -1 && areTwoTablesOpened))
        {
            activateAnotherTable();
            currentTablePanelWidget()->show();
            inactiveNamedTableWidget(currentTablePanelWidget())->setActive(false);
            tableMenuSetEnabled(true);
            closeAllDialogs();
        }

        if (!_openedTables || (_openedTables == 1 && (!result || result == -1)))
            _openedTables++;

        _currentTableWidget->setRowCount(rowsNum);
        currentTablePanelWidget()->updateRowCountLabel();
        for (int i = 0; i < rowsNum; i++)
            _currentTableWidget->createNewEntry(i, QString(), QString());
        currentTablePanelWidget()->setFilePath(kNewTblFileName);
        currentTablePanelWidget()->setActive(true);
        _currentTableWidget->setCurrentCell(0, 0, QItemSelectionModel::Select);

        updateWindow();
        enableTableActions(true);
        ui.actionSaveAll->setEnabled(true);
    }
}

void QTblEditor::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
#ifdef Q_OS_WIN32
    QString fileName = QDir::fromNativeSeparators(action->text());
#else
    QString fileName = action->text();
#endif
    if (loadFile(fileName))
    {
        _recentFilesList.move(_recentFilesList.indexOf(fileName), 0);
        updateRecentFilesActions();
    }
}

void QTblEditor::addToRecentFiles(const QString &fileName)
{
    int index = _recentFilesList.indexOf(fileName);
    if (index != -1) // it's already in the list
        _recentFilesList.move(index, 0);
    else
    {
        if (_recentFilesList.length() == kMaxRecentFiles)
            _recentFilesList.removeLast();
        _recentFilesList.prepend(fileName);
    }
    updateRecentFilesActions();
}

void QTblEditor::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open table file"), _lastPath,
        tr("All supported formats (*.tbl *.txt *.csv);;Tbl files (*.tbl);;Tab-delimited txt files (*.txt);;CSV files (*.csv);;All files (*)"));
    if (!fileName.isEmpty() && loadFile(fileName))
    {
        addToRecentFiles(fileName);
        _findReplaceDlg->needsRefind();
    }
}

void QTblEditor::reopen()
{
    if (processTable(currentTablePanelWidget()->absoluteFileName()))
    {
        updateWindow(false);
        setWindowModified(_leftTableWidget->isWindowModified() && _rightTableWidget->isWindowModified());
        updateLocationLabel(_currentTableWidget->currentRow());
    }
}

void QTblEditor::sendToServer()
{
    QSettings settings;
    QString url = settings.value("serverUrl").toString();
    if (url.isEmpty())
    {
        url = QInputDialog::getText(this, tr("Server URL"), QString());
        if (url.isEmpty())
            return;
        settings.setValue("serverUrl", url);
    }

    // force wrapping in quotes
    bool wrapCurrent = ui.actionWrapStrings->isChecked();
    ui.actionWrapStrings->setChecked(true);
    QByteArray data;
    writeAsText(data, false);
    ui.actionWrapStrings->setChecked(wrapCurrent);

    QNetworkRequest request(QUrl(url + "?name=" + QFileInfo(currentTablePanelWidget()->absoluteFileName()).baseName()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkAccessManager qnam;
    QNetworkReply *reply = qnam.post(request, data);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    if (reply->error())
        QMessageBox::critical(this, tr("Error"), reply->errorString());
    else
        QMessageBox::information(this, tr("Success"), tr("Response:") + "\n" + reply->readAll());
    reply->deleteLater();
}

bool QTblEditor::loadFile(const QString &fileName, bool shouldShowOpenOptions)
{
    bool areTwoTablesOpened = _openedTables == 2;
    int result = shouldShowOpenOptions ? -1 : 0;
    if (_openedTables == 1 && shouldShowOpenOptions)
    {
        result = openTableMsgBoxResult(); // 0 - add one more, 1 - replace active, rest - cancel
        if (result != 0 && result != 1)
            return false;
    }

    if (result && !currentTablePanelWidget()->absoluteFileName().isEmpty() && !closeTable())
        return false;

    if (!result || (result == -1 && areTwoTablesOpened))
    {
        activateAnotherTable();
        currentTablePanelWidget()->show();
        closeAllDialogs();
    }

    TablePanelWidget *w = currentTablePanelWidget();
    if (processTable(fileName))
    {
        if (!_openedTables || (_openedTables == 1 && (!result || result == -1)))
            _openedTables++;

        w->setFilePath(fileName);
        w->setActive(true);
        w->setWindowModified(false);

        int row = _lastSelectedRowsHash[QDir::toNativeSeparators(fileName)].toInt();
        _currentTableWidget->setCurrentCell(row, 1);
        _currentTableWidget->scrollTo(_currentTableWidget->model()->index(row, 1));

        if (_openedTables == 2)
        {
            inactiveNamedTableWidget(w)->setActive(false);
            tableMenuSetEnabled(true);
        }

        _lastPath = w->fileDirPath();
        enableTableActions(true);

        return true;
    }
    else
    {
        if (!result || (result == -1 && areTwoTablesOpened))
        {
            w->hide();
            activateAnotherTable();
        }
        return false;
    }
}

bool QTblEditor::processTable(const QString &fileName)
{
    _isTableLoaded = false;

    QFile inputFile(fileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QString extension = fileName.right(4).toLower();
        if (extension == ".tbl")
            return _isTableLoaded = processTblFile(&inputFile);
        else if (extension == ".txt" || extension == ".csv")
            return _isTableLoaded = processTxtOrCsvFile(&inputFile);
        else // arbitrary file opened
        {
            QTextStream in(&inputFile);
            in.readLine();
            QString secondLine = in.readLine(); // it should equal "X" if it's tbl file
            inputFile.reset();
            if (secondLine.contains('\t') || secondLine.contains(';') || secondLine.contains(',')) // text format
                _isTableLoaded = processTxtOrCsvFile(&inputFile);
            else // tbl format
                _isTableLoaded = processTblFile(&inputFile);
        }
    }
    else
    {
        QMessageBox::critical(this, qApp->applicationName(), tr("Error opening file \"%1\"\nReason: %2")
                              .arg(fileName, inputFile.errorString()));
        return false;
    }
    return _isTableLoaded;
}

bool QTblEditor::processTblFile(QFile *inputFile)
{
    QDataStream in(inputFile);
    in.setByteOrder(QDataStream::LittleEndian);
    TblStructure tbl;
    tbl.fillHeader(in); // reading header
    DWORD numElem = tbl.header().FileSize - TblHeader::size; // number of bytes to read without header
    char *table = new char[numElem];
    if (in.readRawData(table, numElem) == numElem)
    {
        delete [] table;

        inputFile->reset(); // current offset for reading = 0x00
        in.skipRawData(TblHeader::size); // current offset for reading = 0x15 (we don't need header any more)
        tbl.getStringTable(in);
        inputFile->close();

        int rowCount = tbl.header().NodesNumber;
        _currentTableWidget->setRowCount(rowCount);
        currentTablePanelWidget()->updateRowCountLabel();

        int maxKeyWidth = 0;
        for (WORD i = 0; i < rowCount; i++)
        {
            QPair<QString, QString> currentDataStrings = tbl.dataStrings(i); // reading pair <key, value>
            _currentTableWidget->createNewEntry(i, currentDataStrings.first, currentDataStrings.second);

            int currentKeyWidth = QFontMetrics(_currentTableWidget->item(i, 0)->font()).width(currentDataStrings.first);
            if (maxKeyWidth < currentKeyWidth)
                maxKeyWidth = currentKeyWidth;
        }
        _currentTableWidget->setColumnWidth(0, maxKeyWidth + 1); // making "key" column width fit all the entries

        return true;
    }
    else
    {
        delete [] table;
        QMessageBox::critical(this, qApp->applicationName(), tr("Couldn't read entire file, read only %n byte(s) after header.\n"
                                                    "Probably file is corrupted or wrong file format.", 0, numElem));
        return false;
    }
}

bool QTblEditor::processTxtOrCsvFile(QFile *inputFile)
{
    QList<QByteArray> entries = inputFile->readAll().split('\n'); // TODO: maybe read line by line to improve performance
    inputFile->close();
    if (entries.isEmpty())
        return true;

    int rows = entries.size() - 1; // don't include last empty line
    _currentTableWidget->setRowCount(rows);
    currentTablePanelWidget()->updateRowCountLabel();

    QByteArray currentLine = entries.at(0).trimmed();
    char separator = '\t', wrappingCharKey = '\"', wrappingCharValue = '\"';
    if (inputFile->fileName().right(4).toLower() == ".csv" || currentLine.contains("\",\"") || currentLine.contains("\";\""))
    {
        int secondDoubleQuoteIndex = currentLine.indexOf('\"', 1);
        if (secondDoubleQuoteIndex == -1)
        {
            QMessageBox::critical(this, qApp->applicationName(), tr("Wrong file format - all strings in *.csv should be wrapped in double quotes"));
            closeTable();
            return false;
        }
        separator = currentLine.at(secondDoubleQuoteIndex + 1);
    }
    else
    {
        QList<QByteArray> s = currentLine.split(separator);
        QByteArray currentKey = s.at(0), currentValue = s.at(1);
        if (!currentKey.startsWith(wrappingCharKey) || !currentKey.endsWith(wrappingCharKey))
            wrappingCharKey = 0;
        if (!currentValue.startsWith(wrappingCharValue) || !currentValue.endsWith(wrappingCharValue))
            wrappingCharValue = 0;
    }

    QByteArray keyValueSeparator(1, separator);
    if (wrappingCharKey)
        keyValueSeparator.prepend(wrappingCharKey);
    if (wrappingCharValue)
        keyValueSeparator.append(wrappingCharValue);

    QString wrappingCharKeyString = wrappingCharKey ? QChar(wrappingCharKey) : QString(), wrappingCharValueString = wrappingCharValue ? QChar(wrappingCharValue) : QString();
    QString utf8TblEditHeaderString = QString("%1Key%1%2%3Value%3").arg(wrappingCharKeyString, QChar(separator), wrappingCharValueString);
    QString afj666HeaderString = QString("%1String Index%1%2%3Text%3").arg(wrappingCharKeyString, QChar(separator), wrappingCharValueString);
    int i = 0;
    if (currentLine == utf8TblEditHeaderString || currentLine == afj666HeaderString)
    {
        if (!rows)
            return true;
        currentLine = entries.at(++i);
    }

    int maxKeyWidth = 0;
    for (; i < rows; currentLine = entries.at(++i))
    {
        currentLine = currentLine.trimmed();
        if (wrappingCharKey)
            currentLine = currentLine.mid(1); // remove leading wrappingCharKeyString
        if (wrappingCharValue)
            currentLine.chop(1); // remove trailing wrappingCharValueString

        int separatorIndex = currentLine.indexOf(keyValueSeparator);
        if (separatorIndex == -1)
        {
            QMessageBox::critical(this, qApp->applicationName(), tr("Wrong file format - separator is absent at line %1").arg(i + 1));
            closeTable();
            return false;
        }

        QString key = restoreNewlines(TblStructure::decodeKey(currentLine.left(separatorIndex)));
        _currentTableWidget->createNewEntry(i, key, restoreNewlines(QString::fromUtf8(currentLine.mid(separatorIndex + keyValueSeparator.length()))));

        int currentKeyWidth = QFontMetrics(_currentTableWidget->item(i, 0)->font()).width(key);
        if (maxKeyWidth < currentKeyWidth)
            maxKeyWidth = currentKeyWidth;
    }
    _currentTableWidget->setColumnWidth(0, maxKeyWidth + 1);

    return true;
}

bool QTblEditor::closeTable(bool hideTable)
{
    if (!_openedTables)
        return true;

    if (wasSaved())
    {
        _locationLabel->clear();
        _keyHashLabel->clear();
        closeAllDialogs();

        QString filePath = currentTablePanelWidget()->absoluteFileName();
        if (filePath != kNewTblFileName)
            _lastSelectedRowsHash[QDir::toNativeSeparators(filePath)] = _currentTableWidget->currentRow();

        if (hideTable || filePath == kNewTblFileName)
        {
            currentTablePanelWidget()->clearContents();
            _openedTables--;
        }
        if (_openedTables)
        {
            if (hideTable || filePath == kNewTblFileName)
                currentTablePanelWidget()->hide();
            activateAnotherTable();
            currentTablePanelWidget()->setActive(true);

            tableMenuSetEnabled(false);
            updateWindow(_currentTableWidget->isWindowModified());
            ui.actionSaveAll->setEnabled(_currentTableWidget->isWindowModified());
        }
        else
        {
            enableTableActions(false);
            updateWindow(false);
            ui.actionSaveAll->setEnabled(false);
        }
        return true;
    }
    return false;
}

void QTblEditor::enableTableActions(bool state)
{
    QList<QAction *> actions = QList<QAction *>() << ui.actionSaveAs << ui.actionClose << ui.actionCloseAll << ui.menuEdit->actions() << ui.actionSendToServer;
    foreach (QAction *action, actions)
        action->setEnabled(state);
    ui.menuEdit->setEnabled(state);
}

void QTblEditor::closeEvent(QCloseEvent *event)
{
    if (closeAll(false))
    {
        writeSettings();
        event->accept();
    }
    else
        event->ignore();
}

bool QTblEditor::wasSaved()
{
    if (_currentTableWidget->isWindowModified())
    {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("The table \"%1\" has been modified.").arg(currentTablePanelWidget()->fileName()));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.setWindowFlags(msgBox.windowFlags() & ~Qt::WindowContextHelpButtonHint);

        int answer = msgBox.exec();
        if (answer == QMessageBox::Save)
            save();
        else if (answer == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void QTblEditor::save()
{
    QString fileName = currentTablePanelWidget()->absoluteFileName();
    if (fileName == kNewTblFileName)
        saveAs();
    else
        saveFile(fileName);
}

void QTblEditor::saveAs()
{
    QString fileName = currentTablePanelWidget()->absoluteFileName();
#ifdef Q_OS_WIN32
    if (QSysInfo::WindowsVersion <= QSysInfo::WV_XP) // workaround for XP and earlier Windows versions
    {
        QFileInfo info(fileName);
        fileName = info.canonicalPath() + '/' + info.baseName(); // removing extension
    }
#endif
    QString fileNameToSave = QFileDialog::getSaveFileName(this, tr("Save table"), fileName,
                                                          tr("Tbl files (*.tbl);;Tab-delimited text files (*.txt);;CSV files (*.csv);;All files (*)"));
    if (!fileNameToSave.isEmpty() && saveFile(fileNameToSave))
    {
        // change opened file path only if saving with the same extension
        if (fileName.right(4) == fileNameToSave.right(4))
            currentTablePanelWidget()->setFilePath(fileNameToSave);
        addToRecentFiles(fileNameToSave);
    }
}

void QTblEditor::saveAll()
{
    if (_currentTableWidget->isWindowModified())
        save();
    if (_openedTables == 2)
    {
        activateAnotherTable();
        if (_currentTableWidget->isWindowModified())
            save();
        activateAnotherTable();
    }
    ui.actionSaveAll->setDisabled(true);
}

bool QTblEditor::saveFile(const QString &fileName)
{
    QByteArray bytesToWrite; // first of all, write everything to buffer
	QFileInfo fi(fileName);
    QString extension = fi.suffix().toLower();
    bool isCsv = extension == "csv", savedAsTbl = false;

    DWORD fileSize;
	if (extension == "tbl")
	{
		fileSize = writeAsTbl(bytesToWrite);
		savedAsTbl = true;
	}
    else if (extension == "txt" || isCsv)
        fileSize = writeAsText(bytesToWrite, isCsv);
    else // any file
    {
        QMessageBox msgbox(this);
        msgbox.setWindowTitle(qApp->applicationName());
        msgbox.setText(tr("Choose file format"));
        msgbox.addButton("tbl", QMessageBox::AcceptRole);
        msgbox.addButton("txt", QMessageBox::AcceptRole);
        msgbox.addButton("csv", QMessageBox::AcceptRole);
        msgbox.setWindowModality(Qt::WindowModal);

        switch (msgbox.exec())
        {
        case 0: // tbl
            fileSize = writeAsTbl(bytesToWrite);
			savedAsTbl = true;
            break;
        case 1: // txt
            fileSize = writeAsText(bytesToWrite, false);
            break;
        case 2: // csv
            fileSize = writeAsText(bytesToWrite, true);
            break;
        }
    }

    QFile output(fileName);
    if (output.open(QIODevice::WriteOnly))
    {
        if (output.write(bytesToWrite) == fileSize)
        {
            _currentTableWidget->clearBackground();
            _lastPath = QFileInfo(fileName).canonicalPath();
            if (_openedTables == 1 || !inactiveNamedTableWidget(currentTablePanelWidget())->tableWidget()->isWindowModified())
                ui.actionSaveAll->setDisabled(true);

            updateWindow(false);
            ui.statusBar->showMessage(tr("File \"%1\" successfully saved").arg(QDir::toNativeSeparators(fileName)), 3000);

			if (savedAsTbl && ui.actionSaveTxtWithTbl->isChecked())
				saveFile(QFileInfo(QDir(fi.absolutePath()), fi.completeBaseName() + ".txt").filePath());

            return true;
        }
        else
            QMessageBox::critical(this, qApp->applicationName(), tr("Error writing file \"%1\"\nReason: %2").arg(QDir::toNativeSeparators(fileName), output.errorString()));
    }
    else
        QMessageBox::critical(this, qApp->applicationName(), tr("Error creating file \"%1\"\nReason: %2").arg(QDir::toNativeSeparators(fileName), output.errorString()));
    return false;
}

DWORD QTblEditor::writeAsTbl(QByteArray &bytesToWrite)
{
    int entriesNumber = _currentTableWidget->rowCount();
    QStringList stringValsWithModifiedColors; // replacing user-readable colors with their internal form
    for (WORD i = 0; i < entriesNumber; i++)
    {
        QString currentVal = _currentTableWidget->item(i, 1)->text();
        for (int j = 0; j < colorStrings.size(); j++)
        {
            if (currentVal.contains(colorStrings.at(j)) && j)
                currentVal.replace(colorStrings.at(j), colorHeader + colorCodes.at(j - 1));
            else
                currentVal.replace(colorStrings.at(j), colorHeader); // string "\color;" found
        }
        stringValsWithModifiedColors << currentVal;
    }

    DWORD dataStartOffset = TblHeader::size + entriesNumber*sizeof(WORD) + entriesNumber*TblHashNode::size;
    QVector<WORD> indices(entriesNumber);
    QVector<TblHashNode> nodes(entriesNumber);
    QVector<bool> collisionsDetected(entriesNumber, false);
    DWORD currentOffset = dataStartOffset, maxCollisionsNumber = 0;
    for (WORD i = 0; i < entriesNumber; i++)
    {
        QByteArray currentKey = TblStructure::encodeKey(_currentTableWidget->item(i, 0)->text()),
        currentVal = stringValsWithModifiedColors.at(i).toUtf8();
        DWORD hashValue = TblStructure::hashValue(currentKey.data(), entriesNumber), hashIndex = hashValue,
        currentCollisionsNumber = 0;
        while (collisionsDetected[hashIndex]) // counting collisions for current hash value
        {
            currentCollisionsNumber++;
            hashIndex++;
            hashIndex %= entriesNumber;
        }
        collisionsDetected[hashIndex] = true;
        if (currentCollisionsNumber > maxCollisionsNumber)
            maxCollisionsNumber = currentCollisionsNumber;
        indices[i] = hashIndex;

        // we need the size of UTF-8 data, not QString
        WORD currentKeyLength = qstrlen(currentKey.constData()) + 1, currentValLength = qstrlen(currentVal.constData()) + 1;
        // convenient constructor instead of nodes[hashIndex].Active = 1; ...
        nodes[hashIndex] = TblHashNode(1, i, hashValue, currentOffset, currentOffset + currentKeyLength, currentValLength);
        currentOffset += currentKeyLength + currentValLength;
    }

    bytesToWrite.fill(0, TblHeader::size); // header will be filled in the end, so we reserve bytes for it
    QDataStream out(&bytesToWrite, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);

    out.skipRawData(TblHeader::size); // current offset for writing = 0x15
    for (WORD i = 0; i < entriesNumber; i++)
        out << indices[i];

    for (WORD i = 0; i < entriesNumber; i++)
        out << nodes[i];

    for (WORD i = 0; i < entriesNumber; i++)
    {
        QByteArray keyLatin1 = TblStructure::encodeKey(_currentTableWidget->item(i, 0)->text()), valUtf8 = stringValsWithModifiedColors.at(i).toUtf8();
        out.writeRawData(keyLatin1.constData(), keyLatin1.size() + 1);
        out.writeRawData(valUtf8.constData(), qstrlen(valUtf8.constData()) + 1);
    }

    DWORD fileSize = bytesToWrite.size();
    out.device()->reset(); // current offset for writing = 0x00 (to write the header)
    out << TblHeader(TblStructure::getCRC(bytesToWrite.data() + dataStartOffset, fileSize - dataStartOffset), entriesNumber,
        entriesNumber, 1, dataStartOffset, maxCollisionsNumber + 1, fileSize); // convenient constructor

    return fileSize;
}

DWORD QTblEditor::writeAsText(QByteArray &bytesToWrite, bool isCsv)
{
    // format: [wrapper]key[wrapper]<separator>[wrapper]value[wrapper]<newline>,
    // where <separator> is '\t' (for *.txt) or ','/';' (for *.csv),
    // [wrapper] - double quotes or nothing (depends on current settings).
    // for *.csv [wrapper] is always '\"'
    quint8 separator = '\t', wrappingChar = '\"';
    if (isCsv)
        separator = ui.actionCsvComma->isChecked() ? ',' : ';';
    else if (!ui.actionWrapStrings->isChecked())
        wrappingChar = 0;

    QDataStream out(&bytesToWrite, QIODevice::WriteOnly);
    for (WORD i = 0, n = _currentTableWidget->rowCount(); i < n; ++i)
    {
        QByteArray keyLatin1 = TblStructure::encodeKey(foldNewlines(_currentTableWidget->item(i, 0)->text()));
        if (wrappingChar)
            out << wrappingChar;
        out.writeRawData(keyLatin1.constData(), keyLatin1.size());
        if (wrappingChar)
            out << wrappingChar;

        out << separator;

        QByteArray valUtf8 = foldNewlines(_currentTableWidget->item(i, 1)->text()).toUtf8();
        if (wrappingChar)
            out << wrappingChar;
        if (!valUtf8.isEmpty())
            out.writeRawData(valUtf8.constData(), qstrlen(valUtf8.constData()));
        if (wrappingChar)
            out << wrappingChar;

        out << static_cast<quint8>('\n');
    }
    return bytesToWrite.size();
}

void QTblEditor::showFindReplaceDialog()
{
    _findReplaceDlg->show(_openedTables == 2);
    _findReplaceDlg->raise();
    _findReplaceDlg->activateWindow();
}

void QTblEditor::changeCurrentTableItem(QTableWidgetItem *newItem)
{
    _currentTableWidget = qobject_cast<D2StringTableWidget *>(newItem->tableWidget());
    _currentTableWidget->setCurrentCell(newItem->row(), newItem->column());

    currentTablePanelWidget()->setActive(true);
    if (_openedTables == 2)
        inactiveNamedTableWidget(currentTablePanelWidget())->setActive(false);
}

void QTblEditor::findNextString(const QString &query, bool isCaseSensitive, bool isExactString, bool isSearchBothTables)
{
    Qt::MatchFlags searchOptions(Qt::MatchWrap | Qt::MatchContains);
    if (isCaseSensitive)
        searchOptions |= Qt::MatchCaseSensitive;
    if (isExactString)
        searchOptions &= ~Qt::MatchContains;
    QList<QTableWidgetItem *> foundItems = _currentTableWidget->findItems(query, searchOptions);
    if (isSearchBothTables && _openedTables == 2)
        foundItems.append(inactiveTableWidget(_currentTableWidget)->findItems(query, searchOptions));
    _findReplaceDlg->getFoundStrings(foundItems);
}

void QTblEditor::goTo()
{
    GoToRowDialog dlg(this, _currentTableWidget->rowCount());
    if (dlg.exec())
        _currentTableWidget->setCurrentCell(dlg.row() - 1, 1);
}

void QTblEditor::editString(QTableWidgetItem *itemToEdit)
{
    int row = itemToEdit->row();
    KeyValueItemsPair itemsPair;
    if (itemToEdit->column()) // value selected
        itemsPair = KeyValueItemsPair(itemToEdit->tableWidget()->item(row, 0), itemToEdit);
    else // key selected
        itemsPair = KeyValueItemsPair(itemToEdit, itemToEdit->tableWidget()->item(row, 1));

    EditStringCellDialog *editStringCellDlg = 0;
    if (_openedTables == 1 || row >= inactiveTableWidget(_currentTableWidget)->rowCount() || !ui.actionSyncScrolling->isChecked())
        editStringCellDlg = new EditStringCellDialog(this, itemsPair);
    else
    {
        QTableWidget *w = inactiveTableWidget(itemToEdit->tableWidget());
        KeyValueItemsPair otherItemsPair(w->item(row, 0), w->item(row, 1));
        if (_currentTableWidget == _leftTableWidget)
            editStringCellDlg = new EditStringCellDialog(this, itemsPair, otherItemsPair);
        else
            editStringCellDlg = new EditStringCellDialog(this, otherItemsPair, itemsPair);
    }

    connect(editStringCellDlg, SIGNAL(editorClosedAt(int)), _currentTableWidget, SLOT(changeCurrentCell(int)));
    if (_openedTables == 2 && ui.actionSyncScrolling->isChecked())
    {
        connect(editStringCellDlg, SIGNAL(editorClosedAt(int)), inactiveTableWidget(_currentTableWidget), SLOT(changeCurrentCell(int)));
        connect(this, SIGNAL(tablesWereSwapped()), editStringCellDlg, SLOT(swapEditors()));
    }

    editStringCellDlg->show();
}

void QTblEditor::updateItem(QTableWidgetItem *item)
{
    if (_isTableLoaded && item && item->background().color() != Qt::green)
    {
        item->setBackground(QBrush(Qt::green));

        TablePanelWidget *w = _leftTablePanelWidget->tableWidget() == item->tableWidget() ? _leftTablePanelWidget : _rightTablePanelWidget;
        w->tableWidget()->addEditedItem(item);
        w->setWindowModified(true);

        setWindowModified(true);
        ui.actionSave->setEnabled(true);
        ui.actionSaveAll->setEnabled(true);
        ui.actionReopen->setEnabled(w->absoluteFileName() != kNewTblFileName);
    }
}

bool QTblEditor::isDialogQuestionConfirmed(const QString &text)
{
    QMessageBox confirmationDialog(QMessageBox::Question, qApp->applicationName(), text, QMessageBox::Yes | QMessageBox::No, this);
    confirmationDialog.setDefaultButton(QMessageBox::Yes);
#ifdef Q_OS_MAC
    confirmationDialog.setWindowModality(Qt::WindowModal);
#endif
    return confirmationDialog.exec() == QMessageBox::Yes;
}

void QTblEditor::deleteSelectedItems()
{
    bool needsClear = sender() == ui.actionClearSelected; // clear if true, delete if false
    if (isDialogQuestionConfirmed(tr("Are you sure you want to %1 selected items?").arg(needsClear ? tr("clear") : tr("delete"))))
    {
        _currentTableWidget->deleteItems(needsClear);
        updateWindow();
        if (!needsClear)
            currentTablePanelWidget()->updateRowCountLabel();
    }
}

void QTblEditor::updateWindow(bool isModified)
{
    setWindowModified(isModified);
    currentTablePanelWidget()->setWindowModified(isModified);

    ui.actionSave->setEnabled(isModified);
    if (currentTablePanelWidget()->absoluteFileName() == kNewTblFileName)
        ui.actionReopen->setEnabled(false);
    else
        ui.actionReopen->setEnabled(isModified);
}

void QTblEditor::updateLocationLabel(int newRow)
{
    if (newRow < 0)
        return;

    int row = newRow + 1, rows = _currentTableWidget->rowCount();
    _locationLabel->setText(QString("%1 (0x%2) / %3 (0x%4)").arg(row).arg(row, 0, 16).arg(rows).arg(rows, 0, 16));

    QString keyHash = QString("0x%1").arg(TblStructure::hashValue(TblStructure::encodeKey(_currentTableWidget->item(newRow, 0)->text()).data(), rows), 0, 16);
    D2StringTableWidget *otherTableWidget = inactiveTableWidget(_currentTableWidget);
    int inactiveRows = otherTableWidget->rowCount();
    if (_openedTables == 2 && newRow < inactiveRows)
    {
        QString otherKeyHash = QString("0x%1").arg(TblStructure::hashValue(TblStructure::encodeKey(otherTableWidget->item(newRow, 0)->text()).data(), inactiveRows), 0, 16);
        if (keyHash != otherKeyHash)
        {
            if (_currentTableWidget == _leftTableWidget)
                keyHash += " | " + otherKeyHash;
            else
                keyHash = otherKeyHash + " | " + keyHash;
        }
    }
    _keyHashLabel->setText(keyHash);
}

void QTblEditor::writeSettings()
{
    QSettings settings;
    settings.setValue("syncScrolling", ui.actionSyncScrolling->isChecked());
    settings.setValue("lastSelectedRowsHash", _lastSelectedRowsHash);

    settings.beginGroup("geometry");
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("splitterState", _tableSplitter->saveState());
    settings.endGroup();

    settings.beginGroup("options");
    settings.setValue("isCsvSeparatorComma", ui.actionCsvComma->isChecked());
    settings.setValue("restoreOpenedFiles", ui.actionRestoreLastOpenedFiles->isChecked());
    settings.setValue("wrapTxtStrings", ui.actionWrapStrings->isChecked());
    settings.setValue("showHexInRows", ui.actionShowHexInRow->isChecked());
    settings.setValue("startNumberingFrom", _startNumberingGroup->checkedAction()->text());
	settings.setValue("saveTxtWithTbl", ui.actionSaveTxtWithTbl->isChecked());
    settings.endGroup();

    settings.beginGroup("recentItems");
    settings.setValue("lastPath", _lastPath);
    settings.setValue("recentFiles", _recentFilesList);
    settings.endGroup();

    if (ui.actionRestoreLastOpenedFiles->isChecked() && _openedTables)
    {
        settings.beginGroup("lastSession");
        settings.setValue("firstTablePath", currentTablePanelWidget()->absoluteFileName());
        settings.setValue("firstTableRow", _currentTableWidget->currentRow());
        if (_openedTables == 2)
        {
            activateAnotherTable();
            settings.setValue("secondTablePath", currentTablePanelWidget()->absoluteFileName());
            settings.setValue("secondTableRow", _currentTableWidget->currentRow());
        }
        else
        {
            settings.remove("secondTablePath");
            settings.remove("secondTableRow");
        }
        settings.endGroup();
    }
    else
        settings.remove("lastSession");


    // write custom colors to file
    QFile f(
#ifdef Q_OS_MAC
            qApp->applicationDirPath() + "/../Resources/" +
#endif
            kCustomColorsFileName);
    if (f.open(QIODevice::WriteOnly))
    {
        QTextStream out(&f);
        out << tr("# You can place comments anywhere in the file\n# Format: name[tab]hex code[tab]hex RGB\n");
        for (int i = colorsNum; i < colors.size(); i++)
            out << colorStrings.at(i + 1) << '\t' << QString("0x%1").arg(colorCodes.at(i).unicode(), 0, 16)
            << '\t' << colorHexString(colors.at(i)) << endl;
    }
    else
        QMessageBox::critical(this, qApp->applicationName(),
                              tr("Unable to create file \"customcolors.ini\"\nReason: %1").arg(f.errorString()));
}

void QTblEditor::readSettings()
{
    QSettings settings;
    ui.actionSyncScrolling->setChecked(settings.value("syncScrolling", true).toBool());
    _lastSelectedRowsHash = settings.value("lastSelectedRowsHash").toHash();

    settings.beginGroup("geometry");
    restoreGeometry(settings.value("windowGeometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    _tableSplitter->restoreState(settings.value("splitterState").toByteArray());
    settings.endGroup();

    settings.beginGroup("options");
    ui.actionRestoreLastOpenedFiles->setChecked(settings.value("restoreOpenedFiles", true).toBool());
    ui.actionCsvSemiColon->setChecked(!settings.value("isCsvSeparatorComma", true).toBool());
    ui.actionWrapStrings->setChecked(settings.value("wrapTxtStrings", true).toBool());
    ui.actionShowHexInRow->setChecked(settings.value("showHexInRows").toBool());
    (settings.value("startNumberingFrom").toString() == "0" ? ui.actionStartNumberingFrom0 : ui.actionStartNumberingFrom1)->setChecked(true);
	ui.actionSaveTxtWithTbl->setChecked(settings.value("saveTxtWithTbl").toBool());
    settings.endGroup();

    settings.beginGroup("recentItems");
    _lastPath = settings.value("lastPath").toString();
    _recentFilesList = settings.value("recentFiles").toStringList();
    updateRecentFilesActions();
    settings.endGroup();

    settings.beginGroup("lastSession");
    QStringList keys = settings.allKeys();
    if (keys.length() >= 2)
    {
        if (loadFile(settings.value(keys.at(0)).toString()))
            _currentTableWidget->setCurrentCell(settings.value(keys.at(1)).toInt(), 1);

        if (keys.length() == 4)
        {
            _isTableLoaded = false;
            if (loadFile(settings.value(keys.at(2)).toString(), false))
                _currentTableWidget->setCurrentCell(settings.value(keys.at(3)).toInt(), 1);
        }
    }
    settings.endGroup();


    // read custom colors from file
    QFile f(
#ifdef Q_OS_MAC
            qApp->applicationDirPath() + "/../Resources/" +
#endif
            kCustomColorsFileName);
    if (f.exists())
    {
        if (f.open(QIODevice::ReadOnly))
        {
            QTextStream in(&f);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                if (!line.startsWith('#') && !line.isEmpty())
                {
                    QStringList list = line.split('\t');
                    if (list.size() != 3)
                        continue;

                    colorStrings.append(list.at(0));
                    colorCodes.append(QChar(((QString)list.at(1)).toInt(0, 16)));

                    QString colorString = list.at(2);
                    int r = colorString.mid(1, 2).toInt(0, 16), g = colorString.mid(3, 2).toInt(0, 16), b = colorString.mid(5, 2).toInt(0, 16);
                    colors.append(QColor(r, g, b));
                }
            }
        }
        else
            QMessageBox::critical(this, qApp->applicationName(), tr("Error opening file \"%1\"\nReason: %2").arg(f.fileName(), f.errorString()));
    }
}

void QTblEditor::aboutApp()
{
    const QString appFullName = qApp->applicationName() + " v" + qApp->applicationVersion(), email("decapitator@ukr.net");

    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle(tr("About %1").arg(qApp->applicationName()));
    aboutBox.setIconPixmap(windowIcon().pixmap(64));
    aboutBox.setTextFormat(Qt::RichText);
    aboutBox.setText(QString("<b>%1</b><br />").arg(appFullName)
                     + tr("Compiled on: %1").arg(QLocale(QLocale::C).toDateTime(QString(__TIMESTAMP__).simplified(), "ddd MMM d hh:mm:ss yyyy").toString("dd.MM.yyyy hh:mm:ss")));
    aboutBox.setInformativeText(tr("<i>Author:</i> Filipenkov Andrey (<b>kambala</b>)")
                                + QString("<br /><i>Telegram:</i> <a href=\"https://telegram.me/kambala_decapitator\">@kambala_decapitator</a><br /><i>E-mail:</i> <a href=\"mailto:%2?subject=%3\">%2</a>").arg(email, appFullName));
    aboutBox.exec();
}

TablePanelWidget *QTblEditor::currentTablePanelWidget() const
{
    return _currentTableWidget == _leftTablePanelWidget->tableWidget() ? _leftTablePanelWidget : _rightTablePanelWidget;
}

TablePanelWidget *QTblEditor::inactiveNamedTableWidget(TablePanelWidget *namedTableToCheck) const
{
    return namedTableToCheck == _leftTablePanelWidget ? _rightTablePanelWidget : _leftTablePanelWidget;
}

D2StringTableWidget *QTblEditor::inactiveTableWidget(QTableWidget *tableToCheck) const
{
    return tableToCheck == _leftTableWidget ? _rightTableWidget : _leftTableWidget;
}

void QTblEditor::changeCurrentTable(QWidget *newActiveTable)
{
    TablePanelWidget *w = qobject_cast<TablePanelWidget *>(newActiveTable);
    if (_currentTableWidget != w->tableWidget())
    {
        _currentTableWidget = w->tableWidget();
        w->setActive(true);
        inactiveNamedTableWidget(w)->setActive(false);

        bool isTableModified = _currentTableWidget->isWindowModified();
        ui.actionSave->setEnabled(isTableModified);
        ui.actionReopen->setEnabled(isTableModified);
    }
}

void QTblEditor::activateAnotherTable()
{
    changeCurrentTable(inactiveNamedTableWidget(currentTablePanelWidget()));
    _currentTableWidget->setFocus();
}

void QTblEditor::supplement()
{
    D2StringTableWidget *smallerTable = _leftTableWidget->rowCount() < _rightTableWidget->rowCount() ? _leftTableWidget : _rightTableWidget;
    D2StringTableWidget *biggerTable = inactiveTableWidget(smallerTable);
    int i = smallerTable->rowCount(), maxRow = biggerTable->rowCount();
    if (i == maxRow)
        return;

    for (; i < maxRow; i++)
    {
        smallerTable->createRowAt(i);
        smallerTable->createNewEntry(i, biggerTable->item(i, 0)->text(), biggerTable->item(i, 1)->text());
    }

    _currentTableWidget = smallerTable;
    updateWindow();
    ui.actionSaveAll->setEnabled(true);
}

void QTblEditor::swapTables()
{
    _tableSplitter->addWidget(_leftTablePanelWidget);

    TablePanelWidget *temp = _leftTablePanelWidget;
    _leftTablePanelWidget = _rightTablePanelWidget;
    _rightTablePanelWidget = temp;

    _leftTableWidget = _leftTablePanelWidget->tableWidget();
    _rightTableWidget = _rightTablePanelWidget->tableWidget();

    updateLocationLabel(_currentTableWidget->currentRow());
    emit tablesWereSwapped();
}

void QTblEditor::increaseRowCount(int rowIndex)
{
    _currentTableWidget->createRowAt(rowIndex);
    updateWindow();
    currentTablePanelWidget()->updateRowCountLabel();
}

void QTblEditor::copy()
{
    QStringList selectedLines;
    foreach (const QTableWidgetSelectionRange &range, _currentTableWidget->selectedRanges())
    {
        for (int j = range.topRow(); j <= range.bottomRow(); j++)
        {
            QString s;
            if (range.columnCount() == 1)
                s = QString("\"%1\"").arg(_currentTableWidget->item(j, range.rightColumn())->text());
            else
                s = QString("\"%1\"\t\"%2\"").arg(_currentTableWidget->item(j, 0)->text(), _currentTableWidget->item(j, 1)->text());
            selectedLines += foldNewlines(s);
        }
    }

    qApp->clipboard()->setText(selectedLines.join("\n"));
}

void QTblEditor::paste()
{
    const QMimeData *data = qApp->clipboard()->mimeData();
    if (data->hasText())
    {
        QStringList records = data->text().split("\n");
        if (records.last().isEmpty()) // remove last empty line
            records.removeLast();
        int row = _currentTableWidget->currentRow() + 1, recordsNumber = records.size();
        if (records.at(0).contains('\t')) // format: "key"<tab>"value"
        {
            for (int i = 0; i < recordsNumber; i++)
            {
                int rowIndex = row + i;
                _currentTableWidget->createRowAt(rowIndex);
                QStringList keyValueString = records.at(i).split('\t');
                _currentTableWidget->createNewEntry(rowIndex, stripSurroundingQuotes(keyValueString.at(0)), stripSurroundingQuotes(restoreNewlines(keyValueString.at(1))));
            }
        }
        else // format: "text"
        {
            bool isKey = !_currentTableWidget->currentColumn();
            for (int i = 0; i < recordsNumber; i++)
            {
                int rowIndex = row + i;
                _currentTableWidget->createRowAt(rowIndex);

                QString s = stripSurroundingQuotes(restoreNewlines(records.at(i)));
                if (isKey)
                    _currentTableWidget->createNewEntry(rowIndex, s, QString());
                else
                    _currentTableWidget->createNewEntry(rowIndex, QString(), s);
            }
        }

        for (int i = row; i < row + recordsNumber; i++)
        {
            updateItem(_currentTableWidget->item(i, 0));
            updateItem(_currentTableWidget->item(i, 1));
        }

        _currentTableWidget->setCurrentCell(row + recordsNumber, 0);
        currentTablePanelWidget()->updateRowCountLabel();
    }
}

void QTblEditor::toggleRowsHeight(bool isSmall)
{
    // TODO: try to improve performance
    int height = isSmall ? 20 : 30;
    for (int i = 0; i < _leftTableWidget->rowCount(); i++)
        _leftTableWidget->setRowHeight(i, height);
    for (int i = 0; i < _rightTableWidget->rowCount(); i++)
        _rightTableWidget->setRowHeight(i, height);
}

void QTblEditor::changeKeyColumnVisibility(bool hide)
{
	_leftTableWidget->setColumnHidden(0, hide);
	_rightTableWidget->setColumnHidden(0, hide);
}

QStringList QTblEditor::differentStrings(TablesDifferencesWidget::DiffType diffType) const
{
    int minRows = qMin(_leftTableWidget->rowCount(), _rightTableWidget->rowCount());
    QStringList differenceRows;
    for (int i = 0; i < minRows; i++)
    {
        bool areDifferentKeys = _leftTableWidget->item(i, 0)->text() != _rightTableWidget->item(i, 0)->text();
        bool areDifferentStrings = !areDifferentKeys && _leftTableWidget->item(i, 1)->text() != _rightTableWidget->item(i, 1)->text();
        bool areDifferentEither = areDifferentKeys || areDifferentStrings;
        if ((diffType == TablesDifferencesWidget::Keys          && areDifferentKeys) ||
            (diffType == TablesDifferencesWidget::Strings       && areDifferentStrings) ||
            (diffType == TablesDifferencesWidget::KeysOrStrings && areDifferentEither) ||
            (diffType == TablesDifferencesWidget::SameStrings   && !areDifferentEither))
        {
            differenceRows.append(QString("%1 (0x%2)").arg(i + 1).arg(i + 1, 0, 16));
        }
    }
    return differenceRows;
}

QString QTblEditor::foldNewlines(const QString &s)
{
    return QString(s).replace(QLatin1String("\n"), QLatin1String("\\n"));
}

QString QTblEditor::restoreNewlines(const QString &s)
{
    return QString(s).replace(QLatin1String("\\n"), QLatin1String("\n"));
}

void QTblEditor::showDifferences()
{
    QAction *action = qobject_cast<QAction *>(sender());
    TablesDifferencesWidget::DiffType diffType = (TablesDifferencesWidget::DiffType)action->data().toInt();

    QStringList differenceRows = differentStrings(diffType);
    if (differenceRows.size())
    {
        TablesDifferencesWidget *diffWidget = 0;
        foreach (TablesDifferencesWidget *w, findChildren<TablesDifferencesWidget *>())
        {
            if (w->diffType() == diffType)
            {
                diffWidget = w;
                diffWidget->clear();
                break;
            }
        }

        if (!diffWidget)
        {
            diffWidget = new TablesDifferencesWidget(this, diffType);
            connect(diffWidget->listWidget(), SIGNAL(currentTextChanged(const QString &)), _leftTableWidget, SLOT(tableDifferencesItemChanged(const QString &)));
            if (!ui.actionSyncScrolling->isChecked())
                connect(diffWidget->listWidget(), SIGNAL(currentTextChanged(const QString &)), _rightTableWidget, SLOT(tableDifferencesItemChanged(const QString &)));
            connect(diffWidget, SIGNAL(refreshRequested(TablesDifferencesWidget *)), SLOT(refreshDifferences(TablesDifferencesWidget *)));
            connect(diffWidget, SIGNAL(modifyTextRequested(bool,bool)), SLOT(modifyText(bool,bool)));
        }
        diffWidget->addRows(differenceRows);
        diffWidget->resize(diffWidget->sizeHint());
        diffWidget->show();
    }
    else
        QMessageBox::information(this, qApp->applicationName(), tr("Tables are identical"));
}

void QTblEditor::refreshDifferences(TablesDifferencesWidget *w)
{
    w->clear();
    w->addRows(differentStrings(w->diffType()));
}

void QTblEditor::modifyText(bool fromLeftToRight, bool append)
{
    QTableWidget *acceptor = fromLeftToRight ? _rightTableWidget : _leftTableWidget, *donor = inactiveTableWidget(acceptor);
    QTableWidgetItem *acceptorItem = acceptor->item(acceptor->currentRow(), 1);
    QString prefix;
    if (append)
        prefix = acceptorItem->text();
    acceptorItem->setText(prefix + donor->item(donor->currentRow(), 1)->text());
}

void QTblEditor::syncScrollingChanged(bool isSyncing)
{
    QScrollBar *leftScrollbar = _leftTableWidget->verticalScrollBar(), *rightScrollbar = _rightTableWidget->verticalScrollBar();
    const char *changeCellSlot = SLOT(changeCurrentCell(int, int));
    if (isSyncing)
    {
        connect(leftScrollbar,  SIGNAL(valueChanged(int)), rightScrollbar, SLOT(setValue(int)));
        connect(rightScrollbar, SIGNAL(valueChanged(int)), leftScrollbar,  SLOT(setValue(int)));

        connect(_leftTableWidget,  SIGNAL(currentCellChanged(int, int, int, int)), _rightTableWidget, changeCellSlot);
        connect(_rightTableWidget, SIGNAL(currentCellChanged(int, int, int, int)), _leftTableWidget,  changeCellSlot);
    }
    else
    {
        leftScrollbar->disconnect(rightScrollbar);
        rightScrollbar->disconnect(leftScrollbar);

        _leftTableWidget->disconnect(_rightTableWidget, changeCellSlot);
        _rightTableWidget->disconnect(_leftTableWidget, changeCellSlot);
    }
}
