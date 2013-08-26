#ifndef EDITSTRINGCELL_H
#define EDITSTRINGCELL_H

#include "ui_editstringcell.h"

#include <QTableWidgetItem>


QString colorHexString(const QColor &c);


typedef QPair<QTableWidgetItem *, QTableWidgetItem *> KeyValueItemsPair; // <keyItem, valueItem>
static const KeyValueItemsPair kEmptyKeyValuePair(0, 0);

class EditStringCell : public QWidget
{
    Q_OBJECT

public:
    EditStringCell(QWidget *parent, const KeyValueItemsPair &keyValueItemsPairToEdit);

    void saveChanges();
    KeyValueItemsPair itemsPair() const { return _keyValueItemsPair; }
    void setItem(KeyValueItemsPair newKeyValueItemsPair);
    void changeItem(bool toNext);

    QPlainTextEdit *textEdit() const { return ui.stringEdit; }

public slots:
    void resetText();
    void updateColorsMenu();
    void setFocus() { QWidget::setFocus(); ui.stringEdit->setFocus(); }

signals:
    void colorMenuChanged();
    void maxLengthExceededFor110(const QString &warningText);

private slots:
    void wrapModeChanged(bool state);
    void insertText();
    void updateCharsEditCounter() { ui.charsEditCountLabel->setText(QString::number(ui.stringEdit->toPlainText().length())); }
    void calculateKeyHashValue();
    void setPreviewText();
    void updateCurrentEditColumn() { ui.currentEditColumnLabel->setText(QString::number(ui.stringEdit->textCursor().columnNumber())); }
    void showEditColorsDialog();

private:
    Ui::EditStringCellClass ui;
    QMenu *_colorMenu;
    KeyValueItemsPair _keyValueItemsPair;
};

#endif // EDITSTRINGCELL_H
