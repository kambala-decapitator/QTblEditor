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
    explicit EditStringCellDialog(QWidget *parent, KeyValueItemsPair leftItemsPairToEdit, KeyValueItemsPair rightItemsPairToEdit = emptyKeyValuePair);

public slots:
    void swapEditors();

signals:
    void sendingText(KeyValueItemsPair leftItemsPair, KeyValueItemsPair rightItemsPair);
    void editorClosedAt(int row);

protected:
    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent *e);

private slots:
    void saveText();

    void previous() { changeRecord(false); }
    void next()     { changeRecord(true);  }

    void replaceText();
    void appendText();

private:
    Ui::EditStringCellDialog ui;
    EditorsSplitter *_editorsSplitter;
    EditStringCell *_leftEditor, *_rightEditor;

    void updateLocation();
    void changeRecord(bool isNext);
    QPlainTextEdit *targetTextEditAndText(bool toRight, QString *outText);
};

#endif // EDITSTRINGCELLDIALOG_H
