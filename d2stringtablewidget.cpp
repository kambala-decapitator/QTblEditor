#include "d2stringtablewidget.h"

#include <QProgressDialog>
#include <QKeyEvent>
#include <QAction>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


D2StringTableWidget::D2StringTableWidget(QWidget *parent) : QTableView(parent), _model(new QStandardItemModel(0, 2, this)), _displayRowHex(false), _addToRowValue(true)
{
    setModel(_model);
    _model->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("String"));
    setStyleSheet("QTableView::item:!active { selection-background-color: #999999 }");
    horizontalHeader()->
#ifdef IS_QT5
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
#ifndef Q_OS_MAC
        case Qt::Key_Enter:  // Return (usual Enter) or Enter (on the numpad)
#endif
        case Qt::Key_Return: // starts editing of the current selected cell
            if (state() != QAbstractItemView::EditingState)
                ;//emit itemDoubleClicked(currentItem());
            break;
        // in-place edit
#ifdef Q_OS_MAC
        case Qt::Key_Enter:
#else
        case Qt::Key_F2:
#endif
            editInPlace();
            break;
        case Qt::Key_Home: // Home or Ctrl+Home goes to the first cell
            if (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::ControlModifier)
                setCurrentIndex(QModelIndex());
//                setCurrentCell(0, 0, QItemSelectionModel::ClearAndSelect);
            break;
        case Qt::Key_End: // End or Ctrl+End goes to the last cell
            if (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::ControlModifier)
                setCurrentIndex(_model->index(rowCount() - 1, 1));
//                setCurrentCell(rowCount() - 1, 1, QItemSelectionModel::ClearAndSelect);
            break;
        default:
            QTableView::keyPressEvent(keyEvent);
            break;
    }
}

void D2StringTableWidget::deleteItems(bool isClear)
{
//    QList<QTableWidgetSelectionRange> ranges(selectedRanges());
//    int elementsToDelete = 0;
//    for (int i = 0; i < ranges.size(); i++)
//        elementsToDelete += ranges.at(i).rowCount();

//    // deleting rows is a long process, so progress dialog is shown
//    QProgressDialog progress(tr("Deleting selected rows..."), tr("Cancel"), 0, elementsToDelete, this);
//    progress.setWindowModality(Qt::WindowModal);
//    for (int i = 0, rowShift = 0; i < ranges.size(); ++i)
//    {
//        const QTableWidgetSelectionRange &range = ranges.at(i);
//        if (isClear) // Only Delete pressed, clears selected items
//        {
//            for (int j = range.topRow(); j <= range.bottomRow(); ++j)
//                for (int k = range.leftColumn(); k <= range.rightColumn(); ++k)
//                    item(j, k)->setText(QString());
//        }
//        else // Shift+Delete pressed, removes selected rows
//        {
//            for (int k = 0; k < range.rowCount(); ++k)
//            {
//                progress.setValue((i + 1) * k);
//                if (progress.wasCanceled())
//                    return;

//                int row = range.topRow() - rowShift;
//                removeRow(row);
//                _editedItems.remove(qMakePair<int, int>(row, 0));
//                _editedItems.remove(qMakePair<int, int>(row, 1));
//            }
//            rowShift += range.rowCount();
//            emit currentCellChanged(currentRow(), 0, 0, 0);
//        }
//    }
//    progress.setValue(elementsToDelete);

//    if (!isClear)
//        changeRowHeaderDisplay();
}

void D2StringTableWidget::createRowAt(int row)
{
    _model->insertRow(row/*, QList<QStandardItem *>() << new QStandardItem << new QStandardItem*/);
//    _model->setItem(row, 0, new QStandardItem);
//    _model->setItem(row, 1, new QStandardItem);
    setCurrentIndex(_model->index(row, 0));
//    setCurrentCell(row, 0, QItemSelectionModel::ClearAndSelect);
//    emit currentCellChanged(row, 0, 0, 0);
}

void D2StringTableWidget::createNewEntry(int row, const QString &key, const QString &val)
{
    _model->setItem(row, 0, new QStandardItem(key.isEmpty() || key == "\"" ? QString() : key));
    _model->setItem(row, 1, new QStandardItem(val.isEmpty() || val == "\"" ? QString() : val));
}

int D2StringTableWidget::rowCount() const
{
    return _model->rowCount();
}

int D2StringTableWidget::currentRow() const
{
    return currentIndex().row();
}

int D2StringTableWidget::currentColumn() const
{
    return currentIndex().column();
}

void D2StringTableWidget::setRowCount(int rows)
{
    _model->setRowCount(rows);
}

QList<QStandardItem *> D2StringTableWidget::findItems(const QString &text, Qt::MatchFlags flags) const
{
    QList<QStandardItem *> keys = _model->findItems(text, flags, 0);
    QList<QStandardItem *> values = _model->findItems(text, flags, 1);
    return keys + values;
}

void D2StringTableWidget::mousePressEvent(QMouseEvent *mouseEvent)
{
    emit tableGotFocus(parentWidget());
    QTableView::mousePressEvent(mouseEvent);

    if (mouseEvent->button() == Qt::RightButton)
        editInPlace();
}

void D2StringTableWidget::clearBackground()
{
    // without blocking cells will become green again immediately
    // TODO: check if blocking still needed
    blockSignals(true);
    foreach (QStandardItem *item, _editedItems)
        item->setBackground(QBrush(Qt::white));
    blockSignals(false);

    _editedItems.clear();
}

void D2StringTableWidget::dropEvent(QDropEvent *event)
{
//    QTableWidgetItem *firstDroppedItem = itemAt(event->pos());
//    QList<QStandardItem *> droppedItems;
//    QStringList oldTexts;
//    QTableWidgetSelectionRange range = qobject_cast<QTableWidget *>(event->source())->selectedRanges().at(0);
//    for (int i = 0, rows = range.bottomRow() - range.topRow(), cols = range.rightColumn() - range.leftColumn(); i <= rows; ++i)
//    {
//        for (int j = 0; j <= cols; ++j)
//        {
//            QTableWidgetItem *anItem = item(firstDroppedItem->row() + i, firstDroppedItem->column() + j);
//            droppedItems += anItem;
//            oldTexts += anItem->text();
//            addEditedItem(anItem);
//        }
//    }

//    QTableWidget::dropEvent(event);
//    for (int i = 0; i < droppedItems.size(); ++i)
//    {
//        QTableWidgetItem *anItem = droppedItems.at(i);
//        if (anItem->text() != oldTexts.at(i))
//            emit itemWasDropped(anItem);
//    }
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
#ifdef Q_OS_MAC
        rowText += "  "; // fixes slight text truncation
#endif
        rowLabels += rowText;
    }
    _model->setVerticalHeaderLabels(rowLabels);
}

void D2StringTableWidget::changeCurrentCell(int row, int col) { if (row < _model->rowCount()) setCurrentIndex(_model->index(row, col)); }
