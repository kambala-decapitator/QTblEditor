#include "gotorowdialog.h"

#include <QPushButton>

#include <QSettings>


GoToRowDialog::GoToRowDialog(QWidget *parent, int rowCount) : QDialog(parent), _maxRow(rowCount)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setFixedSize(sizeHint());
	setWindowTitle(tr("Go to row [1-%1 (0x%2)]").arg(rowCount).arg(rowCount, 0, 16));

	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	bool isHex = QSettings().value("goToRowModeIsHex").toBool();
	ui.radioButtonHex->setChecked(isHex);
	changeMode(isHex);

	ui.lineEditValue->setFocus();

	connect(ui.radioButtonHex, SIGNAL(toggled(bool)), SLOT(changeMode(bool)));
	connect(ui.lineEditValue, SIGNAL(textChanged(QString)), SLOT(enableOkButton()));
}

void GoToRowDialog::changeMode(bool isHex)
{
	ui.lineEditValue->setValidator(new QRegExpValidator(isHex ? QRegExp("[\\da-fA-F]+") : QRegExp("\\d+"), ui.lineEditValue));
}

void GoToRowDialog::enableOkButton()
{
	int value = row();
	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(value >= 1 && value <= _maxRow);
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
