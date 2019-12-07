#include "gotorowdialog.h"

#include <QPushButton>
#include <QRegExpValidator>

#include <QSettings>


GoToRowDialog::GoToRowDialog(QWidget *parent, int rowCount, bool rowsStartFromZero, bool showHex) : QDialog(parent), _minRow(1), _maxRow(rowCount)
{
    ui.setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(sizeHint());

    if (rowsStartFromZero)
    {
        --_minRow;
        --_maxRow;
    }
    setWindowTitle(tr("Go to row [%1-%2 (0x%3)]").arg(_minRow).arg(_maxRow).arg(_maxRow, 0, 16));

    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui.radioButtonHex->setChecked(showHex);
    changeMode(showHex);

    connect(ui.radioButtonHex, SIGNAL(toggled(bool)), SLOT(changeMode(bool)));
    connect(ui.lineEditValue, SIGNAL(textChanged(QString)), SLOT(enableOkButton()));
}

void GoToRowDialog::changeMode(bool isHex)
{
    ui.lineEditValue->setValidator(new QRegExpValidator(isHex ? QRegExp("[\\da-fA-F]+") : QRegExp("\\d+"), ui.lineEditValue));
	ui.lineEditValue->setFocus();
}

void GoToRowDialog::enableOkButton()
{
    int value = row();
    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(value >= _minRow && value <= _maxRow);
}

void GoToRowDialog::accept()
{
    QSettings().setValue("goToRowModeIsHex", ui.radioButtonHex->isChecked());
    QDialog::accept();
}

int GoToRowDialog::row()
{
    return ui.lineEditValue->text().toInt(0, ui.radioButtonDec->isChecked() ? 10 : 16);
}
