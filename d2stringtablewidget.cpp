#include "d2stringtablewidget.h"

#include <QProgressDialog>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QAction>


D2StringTableWidget::D2StringTableWidget(QWidget *parent) : QTableWidget(parent), _displayRowHex(false), _addToRowValue(true)
{
	setStyleSheet("QTableWidget::item:!active { selection-background-color: #999999 }");
	horizontalHeader()->
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		setSectionsClickable
#else
		setClickable
#endif
		(false);
}

void D2StringTableWidget::keyPressEvent(QKeyEvent *keyEvent)
{
	switch (keyEvent->key())
	{
		case Qt::Key_Enter:  // Return (usual Enter), Enter (on the numpad) or F2
		case Qt::Key_Return: // starts editing of the
		case Qt::Key_F2:     // current selected cell
			emit itemDoubleClicked(currentItem());
			break;
		case Qt::Key_Home: // Home or Ctrl+Home goes to the first cell
			if (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::ControlModifier)
				setCurrentCell(0, 0, QItemSelectionModel::ClearAndSelect);
			break;
		case Qt::Key_End: // End or Ctrl+End goes to the last cell
			if (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::ControlModifier)
				setCurrentCell(rowCount() - 1, 1, QItemSelectionModel::ClearAndSelect);
			break;
		default:
			QTableWidget::keyPressEvent(keyEvent);
	}
}

void D2StringTableWidget::deleteItems(bool isClear)
{
	QList<QTableWidgetSelectionRange> ranges(selectedRanges());
	int elementsToDelete = 0;
	for (int i = 0; i < ranges.size(); i++)
		elementsToDelete += ranges.at(i).rowCount();

	// deleting rows is a long process, so progress dialog is shown
	QProgressDialog progress(tr("Deleting selected rows..."), tr("Cancel"), 0, elementsToDelete, this);
	progress.setWindowModality(Qt::WindowModal);
	int rowShift = 0;
	for (int i = 0; i < ranges.size(); i++)
	{
		QTableWidgetSelectionRange currentSelectedItems(ranges.at(i));
		if (isClear) // Only Delete pressed, clears selected items
		{
			for (int j = currentSelectedItems.topRow(); j <= currentSelectedItems.bottomRow(); j++)
				for (int k = currentSelectedItems.leftColumn(); k <= currentSelectedItems.rightColumn(); k++)
					item(j, k)->setText(QString::null);
		}
		else // Shift+Delete pressed, removes selected rows
		{
			for (int k = 0; k < currentSelectedItems.rowCount(); k++)
			{
				progress.setValue((i + 1) * k);
				if (progress.wasCanceled())
					return;
				removeRow(currentSelectedItems.topRow() - rowShift);
			}
			rowShift += currentSelectedItems.rowCount();
			emit currentCellChanged(currentRow(), 0, 0, 0);
		}
	}
	progress.setValue(elementsToDelete);
}

void D2StringTableWidget::createRowAt(int row)
{
	insertRow(row);
	setItem(row, 0, new QTableWidgetItem);
	setItem(row, 1, new QTableWidgetItem);
	setCurrentCell(row, 0, QItemSelectionModel::ClearAndSelect);
	emit currentCellChanged(row, 0, 0, 0);
}

void D2StringTableWidget::createNewEntry(int row, const QString &key, const QString &val)
{
	setItem(row, 0, new QTableWidgetItem(key.isEmpty() || key == "\"" ? "" : key.split('\"', QString::SkipEmptyParts).at(0)));
	setItem(row, 1, new QTableWidgetItem(val.isEmpty() || val == "\"" ? "" : val.split('\"', QString::SkipEmptyParts).at(0)));
}

void D2StringTableWidget::mousePressEvent(QMouseEvent *mouseEvent)
{
	emit tableGotFocus(parentWidget());
	QTableWidget::mousePressEvent(mouseEvent);
}

void D2StringTableWidget::clearBackground()
{
	foreach (QTableWidgetItem *item, _editedItems)
		item->setBackground(QBrush(Qt::white));
	_editedItems.clear();
}

void D2StringTableWidget::dropEvent(QDropEvent *event)
{
	QTableWidgetItem *item = itemAt(event->pos());
	QString oldString = item->text();

	QTableWidget::dropEvent(event);
	if (item->text() != oldString)
		emit itemWasDropped(item);
}

void D2StringTableWidget::listWidgetItemDoubleClicked(QListWidgetItem *item)
{
	QString text = item->text();
	changeCurrentCell(text.left(text.indexOf(' ')).toInt() - 1);
}

void D2StringTableWidget::toggleDisplayHex(bool toggled)
{
	_displayRowHex = toggled;
	changeRowHeaderDisplay();
}

void D2StringTableWidget::changeRowNumberingTo1(bool toggled)
{
	_addToRowValue = toggled;
	changeRowHeaderDisplay();
}

void D2StringTableWidget::changeRowHeaderDisplay()
{
	QStringList rowLabels;
	for (int i = 0; i < rowCount(); ++i)
	{
		int row = i + _addToRowValue;
		QString rowText = QString::number(row);
		if (_displayRowHex)
			rowText += QString(" (0x%2)").arg(row, 0, 16);
		rowLabels += rowText;
	}
	setVerticalHeaderLabels(rowLabels);
}
