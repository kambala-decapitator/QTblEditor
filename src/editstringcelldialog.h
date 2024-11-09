#ifndef EDITSTRINGCELLDIALOG_H
#define EDITSTRINGCELLDIALOG_H

#include "ui_editstringcelldialog.h"

#include "editstringcell.h"


class QCloseEvent;
class QKeyEvent;
class EditorsSplitter;

class EditStringCellDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditStringCellDialog(QWidget *parent, KeyValueItemsPair leftItemsPairToEdit, bool renderGreyAsWhite, KeyValueItemsPair rightItemsPairToEdit = kEmptyKeyValuePair);

public slots:
    void done(int r);

    void swapEditors();

signals:
    void editorClosedAt(int row);

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void saveText();

    void previous() { changeRecord(false); }
    void next()     { changeRecord(true);  }

    void replaceText();
    void appendText();

    void maybeShowStringLengthWarning(const QString &warningText);

private:
    Ui::EditStringCellDialog ui;
    EditorsSplitter *_editorsSplitter;
    EditStringCell *_leftEditor, *_rightEditor;
    bool _wasStringLengthWarningShown;

    void updateLocation();
    void changeRecord(bool isNext);
    QPlainTextEdit *targetTextEditAndText(bool toRight, QString *outText);
};

#endif // EDITSTRINGCELLDIALOG_H
