#include "editcolorsdialog.h"

#include <QColorDialog>


extern int colorsNum;

EditColorsDialog::EditColorsDialog(QWidget *parent, const QStringList &colorStrings, const QList<QChar> &colorCodes, const QList<QColor> &colors)
	: QDialog(parent)
{
    ui.setupUi(this);
	ui.hexCodeLineEdit->setValidator(new QRegExpValidator(QRegExp("[\\da-fA-F]+"), ui.hexCodeLineEdit));

	for (int i = colorsNum; i < colors.size(); i++)
	{
		_colorsInfo << ColorInfo(colorStrings.at(i + 1), colorCodes.at(i), colors.at(i));
		ui.colorsListWidget->addItem(colorStrings.at(i + 1));
	}

	connect(ui.colorsListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
			SLOT(showColorInfo(QListWidgetItem *, QListWidgetItem *)));
	connect(ui.addColorButton, SIGNAL(clicked()), SLOT(addColor()));
	connect(ui.deleteColorButton, SIGNAL(clicked()), SLOT(deleteColor()));
	connect(ui.colorNameLineEdit, SIGNAL(textChanged(QString)), SLOT(setListItemText(QString)));
	connect(ui.colorNameLineEdit, SIGNAL(textChanged(QString)), SLOT(enableOkButton()));
	connect(ui.hexCodeLineEdit, SIGNAL(textChanged(QString)), SLOT(enableOkButton()));
	connect(ui.chooseColorButton, SIGNAL(clicked()), SLOT(chooseColor()));
	connect(ui.decRadioButton, SIGNAL(toggled(bool)), SLOT(changeRgbNumbersDisplay(bool)));

	if (_colorsInfo.size())
	{
		ui.colorsListWidget->setCurrentRow(0);
		ui.colorPropertiesGroupBox->setEnabled(true);
		ui.deleteColorButton->setEnabled(true);
	}

	QList<QLineEdit *> rgbLineEdits = QList<QLineEdit *>() << ui.redLineEdit << ui.greenLineEdit << ui.blueLineEdit;
	foreach (QLineEdit *rgbLineEdit, rgbLineEdits)
		connect(rgbLineEdit, SIGNAL(textChanged(QString)), SLOT(rgbValueChanged()));
}

void EditColorsDialog::enableOkButton()
{
	ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(ui.colorNameLineEdit->text().isEmpty() || ui.hexCodeLineEdit->text().isEmpty());
}

void EditColorsDialog::updateColorDisplay(const QColor &newColor)
{
	updateColorLabel(newColor);

	bool isDec = ui.decRadioButton->isChecked();
	ui.redLineEdit->setText(QString::number(newColor.red(), isDec ? 10 : 16));
	ui.greenLineEdit->setText(QString::number(newColor.green(), isDec ? 10 : 16));
	ui.blueLineEdit->setText(QString::number(newColor.blue(), isDec ? 10 : 16));
}

void EditColorsDialog::updateColorLabel(const QColor &newColor)
{
	QPixmap pixmap(ui.colorDisplayLabel->size());
	pixmap.fill(newColor);
	ui.colorDisplayLabel->setPixmap(pixmap);
}

QRgb EditColorsDialog::currentRgbColor() const
{
	bool isDec = ui.decRadioButton->isChecked();
	return qRgb(ui.redLineEdit->text().toInt(0, isDec ? 10 : 16),
				ui.greenLineEdit->text().toInt(0, isDec ? 10 : 16),
				ui.blueLineEdit->text().toInt(0, isDec ? 10 : 16));
}

void EditColorsDialog::saveCurrentColor(int row)
{
	_colorsInfo.replace(row, ColorInfo(ui.colorNameLineEdit->text(), QChar(ui.hexCodeLineEdit->text().toInt(0, 16)), currentRgbColor()));
}

void EditColorsDialog::showColorInfo(QListWidgetItem *newItem, QListWidgetItem *oldItem)
{
	if (oldItem)
		saveCurrentColor(ui.colorsListWidget->row(oldItem));

	if (newItem)
	{
		ColorInfo ci = _colorsInfo.at(ui.colorsListWidget->row(newItem));
		ui.colorNameLineEdit->setText(ci.Name);
		ui.hexCodeLineEdit->setText(QString::number(ci.Code.unicode(), 16));
		updateColorDisplay(ci.Color);

		enableOkButton();
	}
	else
	{
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		ui.colorPropertiesGroupBox->setEnabled(false);
	}
}

void EditColorsDialog::addColor()
{
	_colorsInfo.append(ColorInfo());

	QListWidgetItem *newItem = new QListWidgetItem;
	ui.colorsListWidget->addItem(newItem);
	ui.colorsListWidget->setCurrentItem(newItem);

	ui.colorPropertiesGroupBox->setEnabled(true);
	ui.deleteColorButton->setEnabled(true);

	ui.colorNameLineEdit->setFocus();
}

void EditColorsDialog::deleteColor()
{
	QList<QLineEdit *> lineEdits = QList<QLineEdit *>() << ui.colorNameLineEdit << ui.hexCodeLineEdit
								   << ui.redLineEdit << ui.greenLineEdit << ui.blueLineEdit;
	foreach (QLineEdit *lineEdit, lineEdits)
		lineEdit->clear();

	int row = ui.colorsListWidget->currentRow();
	delete ui.colorsListWidget->takeItem(row);
	_colorsInfo.removeAt(row);

	ui.deleteColorButton->setEnabled(ui.colorsListWidget->count() != 0);
}

void EditColorsDialog::chooseColor()
{
	QColor selectedColor = QColorDialog::getColor(currentRgbColor());
	if (selectedColor.isValid())
		updateColorDisplay(selectedColor);
}

void EditColorsDialog::changeRgbNumbersDisplay(bool isDec)
{
	QList<QLineEdit *> rgbLineEdits = QList<QLineEdit *>() << ui.redLineEdit << ui.greenLineEdit << ui.blueLineEdit;
	foreach (QLineEdit *rgbLineEdit, rgbLineEdits)
	{
		rgbLineEdit->setText(QString::number(rgbLineEdit->text().toInt(0, isDec ? 16 : 10), isDec ? 10 : 16));
		if (isDec)
			rgbLineEdit->setValidator(new QIntValidator(0, 255, rgbLineEdit));
		else
			rgbLineEdit->setValidator(new QRegExpValidator(QRegExp("[\\da-fA-F]{,2}"), rgbLineEdit));
	}
}

void EditColorsDialog::accept()
{
	int row = ui.colorsListWidget->currentRow();
	if (row != -1)
		saveCurrentColor(row);
	QDialog::accept();
}
