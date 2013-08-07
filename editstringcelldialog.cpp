#include "editstringcelldialog.h"
#include "editorssplitterhandle.h"

#include <QSplitter>
#include <QMessageBox>

#include <QSettings>


class EditorsSplitter : public QSplitter
{
public:
    EditorsSplitter(Qt::Orientation orientation, QWidget *parent = 0) : QSplitter(orientation, parent) {}

    EditorsSplitterHandle *customHandle() const { return static_cast<EditorsSplitterHandle *>(handle(1)); }

protected:
    virtual QSplitterHandle *createHandle() { return new EditorsSplitterHandle(orientation(), this); }
};


EditStringCellDialog::EditStringCellDialog(QWidget *parent, KeyValueItemsPair leftItemsPairToEdit, KeyValueItemsPair rightItemsPairToEdit) : QDialog(parent), _wasStringLengthWarningShown(false)
{
    ui.setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);

    QString prevAndNextButtonsText = tr("%1\n%2 or %3");
    ui.previousButton->setToolTip(prevAndNextButtonsText.arg(ui.previousButton->toolTip(),
                                                              QKeySequence("Ctrl+Alt+Left").toString(QKeySequence::NativeText),
                                                              QKeySequence("Ctrl+Alt+Up").toString(QKeySequence::NativeText)));
    ui.nextButton->setToolTip(prevAndNextButtonsText.arg(ui.nextButton->toolTip(),
                                                              QKeySequence("Ctrl+Alt+Right").toString(QKeySequence::NativeText),
                                                              QKeySequence("Ctrl+Alt+Down").toString(QKeySequence::NativeText)));
    ui.cancelButton->setToolTip(ui.cancelButton->toolTip() + '\n' + QKeySequence("Esc").toString(QKeySequence::NativeText));
    ui.resetButton->setToolTip(ui.resetButton->toolTip() + '\n' + ui.resetButton->shortcut().toString(QKeySequence::NativeText));
    ui.saveButton->setToolTip(ui.saveButton->toolTip() + '\n' + ui.saveButton->shortcut().toString(QKeySequence::NativeText));

    _editorsSplitter = new EditorsSplitter(Qt::Horizontal, this);
    _editorsSplitter->setChildrenCollapsible(false);
    _leftEditor = new EditStringCell(this, leftItemsPairToEdit);
    _editorsSplitter->addWidget(_leftEditor);
    if (rightItemsPairToEdit != emptyKeyValuePair)
    {
        _rightEditor = new EditStringCell(this, rightItemsPairToEdit);
        _editorsSplitter->addWidget(_rightEditor);

        EditorsSplitterHandle *splitterHandle = _editorsSplitter->customHandle();
        splitterHandle->createButtons();

        connect(ui.resetButton, SIGNAL(clicked()), _rightEditor, SLOT(resetText()));
        connect(_leftEditor,  SIGNAL(colorMenuChanged()), _rightEditor, SLOT(updateColorsMenu()));
        connect(_rightEditor, SIGNAL(colorMenuChanged()), _leftEditor,  SLOT(updateColorsMenu()));
        connect(_rightEditor, SIGNAL(maxLengthExceededFor110(const QString &)), SLOT(maybeShowStringLengthWarning(const QString &)));

        connect(splitterHandle->replaceRightButton(), SIGNAL(clicked()), SLOT(replaceText()));
        connect(splitterHandle->replaceLeftButton(),  SIGNAL(clicked()), SLOT(replaceText()));
        connect(splitterHandle->appendRightButton(),  SIGNAL(clicked()), SLOT(appendText()));
        connect(splitterHandle->appendLeftButton(),   SIGNAL(clicked()), SLOT(appendText()));
    }
    else
        _rightEditor = 0;

    updateLocation();

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(ui.previousButton);
    hLayout->addWidget(ui.nextButton);
    hLayout->addStretch();
    hLayout->addWidget(ui.cancelButton);
    hLayout->addWidget(ui.resetButton);
    hLayout->addWidget(ui.saveButton);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_editorsSplitter);
    layout->addLayout(hLayout);

    restoreGeometry(QSettings().value("geometry/EditStringCellDialogGeometry").toByteArray());

    connect(_leftEditor, SIGNAL(maxLengthExceededFor110(const QString &)), SLOT(maybeShowStringLengthWarning(const QString &)));
    connect(ui.resetButton,    SIGNAL(clicked()), _leftEditor, SLOT(resetText()));
    connect(ui.saveButton,     SIGNAL(clicked()), SLOT(saveText()));
    connect(ui.previousButton, SIGNAL(clicked()), SLOT(previous()));
    connect(ui.nextButton,     SIGNAL(clicked()), SLOT(next()));
}

void EditStringCellDialog::closeEvent(QCloseEvent *e)
{
    QSettings().setValue("geometry/EditStringCellDialogGeometry", saveGeometry());
    emit editorClosedAt(_leftEditor->itemsPair().first->row());
    e->accept();
}

void EditStringCellDialog::saveText()
{
    EditStringCell::SaveResult leftEditorSaveResult = _leftEditor->saveChanges(), rightEditorSaveResult = EditStringCell::NothingChanged;
    KeyValueItemsPair leftItems = _leftEditor->itemsPair(), rightItemsToSend = emptyKeyValuePair;
    KeyValueItemsPair leftItemsToSend(leftEditorSaveResult & EditStringCell::KeyChanged ? leftItems.first : 0,
                                 leftEditorSaveResult & EditStringCell::ValueChanged ? leftItems.second : 0);
    if (_rightEditor)
    {
        rightEditorSaveResult = _rightEditor->saveChanges();
        KeyValueItemsPair rightItems = _rightEditor->itemsPair();
        rightItemsToSend = KeyValueItemsPair(rightEditorSaveResult & EditStringCell::KeyChanged ? rightItems.first : 0,
                                        rightEditorSaveResult & EditStringCell::ValueChanged ? rightItems.second : 0);
    }
    emit sendingText(leftItemsToSend, rightItemsToSend);
}

void EditStringCellDialog::updateLocation()
{
    QTableWidgetItem *item = _leftEditor->itemsPair().first;
    int row = item->row(), displayRow = row + 1;
    setWindowTitle(tr("Edit record [row - %1 (0x%2)]").arg(displayRow).arg(displayRow, 0, 16));

    ui.previousButton->setEnabled(row != 0);

    int rowCount = item->tableWidget()->rowCount();
    if (_rightEditor)
        rowCount = qMin(rowCount, _rightEditor->itemsPair().first->tableWidget()->rowCount());
    ui.nextButton->setEnabled(row != rowCount - 1);
}

void EditStringCellDialog::changeRecord(bool isNext)
{
    saveText();

    _leftEditor->changeItem(isNext);
    if (_rightEditor)
        _rightEditor->changeItem(isNext);

    updateLocation();
}

void EditStringCellDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->modifiers() == (Qt::CTRL | Qt::ALT))
    {
        if (e->key() == Qt::Key_Left)
            previous();
        else if (e->key() == Qt::Key_Right)
            next();
    }
    QDialog::keyPressEvent(e);
}

void EditStringCellDialog::swapEditors()
{
    _editorsSplitter->addWidget(_leftEditor);

    EditStringCell *temp = _leftEditor;
    _leftEditor = _rightEditor;
    _rightEditor = temp;
}

void EditStringCellDialog::replaceText()
{
    QString text;
    QPlainTextEdit *textEdit = targetTextEditAndText(sender() == _editorsSplitter->customHandle()->replaceRightButton(), &text);
    textEdit->selectAll();
    textEdit->insertPlainText(text);
    
}

void EditStringCellDialog::appendText()
{
    QString text;
    QPlainTextEdit *textEdit = targetTextEditAndText(sender() == _editorsSplitter->customHandle()->appendRightButton(), &text);
    textEdit->moveCursor(QTextCursor::End);
    textEdit->insertPlainText(text);
}

void EditStringCellDialog::maybeShowStringLengthWarning(const QString &warningText)
{
    if (!_wasStringLengthWarningShown)
    {
        _wasStringLengthWarningShown = true;
        QMessageBox::warning(this, qApp->applicationName(), warningText);
    }
}

QPlainTextEdit *EditStringCellDialog::targetTextEditAndText(bool toRight, QString *outText)
{
    if (toRight)
    {
        *outText = _leftEditor->textEdit()->toPlainText();
        return _rightEditor->textEdit();
    }
    else
    {
        *outText = _rightEditor->textEdit()->toPlainText();
        return _leftEditor->textEdit();
    }
}
