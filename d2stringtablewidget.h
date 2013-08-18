#ifndef D2STRINGTABLEWIDGET_H
#define D2STRINGTABLEWIDGET_H

#include <QTableWidget>
#include <QHeaderView>


class QKeyEvent;
class QMouseEvent;
class QDropEvent;
class QListWidgetItem;
class QAction;

class D2StringTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    D2StringTableWidget(QWidget *parent = 0);

    void deleteItems(bool isClear);
    void createRowAt(int row);
    void addEditedItem(QTableWidgetItem *editedItem) { _editedItems[editedItem->row()] = editedItem; }
    void clearBackground();
    void createNewEntry(int row, const QString &key, const QString &val);
    void clearContents() { QTableWidget::clearContents(); _editedItems.clear(); }

    void changeRowHeaderDisplay();

public slots:
    void changeCurrentCell(int row, int col = 1) { if (row < rowCount()) setCurrentCell(row, col); }
    void tableDifferencesItemChanged(const QString &newText) { changeCurrentCell(newText.left(newText.indexOf(' ')).toInt() - 1); }

    void toggleDisplayHex(bool toggled);
    void changeRowNumberingTo1(bool toggled);

signals:
    void tableGotFocus(QWidget *);
    void itemWasDropped(QTableWidgetItem *);

protected:
    void keyPressEvent(QKeyEvent *keyEvent);
    void mousePressEvent(QMouseEvent *mouseEvent);
    void dropEvent(QDropEvent *event);

private:
    QMap<int, QTableWidgetItem *> _editedItems;
    bool _displayRowHex, _addToRowValue;

    void editInPlace() { editItem(currentItem()); };
};

#endif // D2STRINGTABLEWIDGET_H
