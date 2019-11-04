#ifndef GOTOROWDIALOG_H
#define GOTOROWDIALOG_H

#include "ui_gotorowdialog.h"


class GoToRowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GoToRowDialog(QWidget *parent, int rowCount, bool rowsStartFromZero, bool showHex);

    int row();

public slots:
    void accept();

private slots:
    void changeMode(bool isHex);
    void enableOkButton();

private:
    Ui::GoToRowDialog ui;
    int _minRow, _maxRow;
};

#endif // GOTOROWDIALOG_H
