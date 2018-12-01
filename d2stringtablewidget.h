#ifndef D2STRINGTABLEWIDGET_H
#define D2STRINGTABLEWIDGET_H

#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>


class QKeyEvent;
class QMouseEvent;
class QDropEvent;
class QAction;

class D2StringTableWidget : public QTableView
{
    Q_OBJECT

public:
    D2StringTableWidget(QWidget *parent = 0);

    void deleteItems(bool isClear);
    void createRowAt(int row);
    void addEditedItem(QStandardItem *editedItem) { _editedItems[qMakePair<int, int>(editedItem->row(), editedItem->column())] = editedItem; }
    void clearBackground();
    void createNewEntry(int row, const QString &key, const QString &val);
    void clearContents() { _editedItems.clear(); }

    int rowCount() const;
    int currentRow() const;
    int currentColumn() const;
    void setRowCount(int rows);
    QStandardItem *item(int row, int col) { return _model->item(row, col); }
    QList<QStandardItem *> findItems(const QString & text, Qt::MatchFlags flags) const;

    void changeRowHeaderDisplay();

public slots:
    void changeCurrentCell(int row, int col = 1);
    void tableDifferencesItemChanged(const QString &newText) { changeCurrentCell(newText.left(newText.indexOf(' ')).toInt() - 1); }

    void toggleDisplayHex(bool toggled);
    void changeRowNumberingTo1(bool toggled);

signals:
    void tableGotFocus(QWidget *);
    void itemWasDropped(QStandardItem *);

protected:
    void keyPressEvent(QKeyEvent *keyEvent);
    void mousePressEvent(QMouseEvent *mouseEvent);
    void dropEvent(QDropEvent *event);

private:
    QStandardItemModel *_model;
    QMap<QPair<int, int>, QStandardItem *> _editedItems;
    bool _displayRowHex, _addToRowValue;

    void editInPlace() { /*editItem(currentItem());*/ };
};

#endif // D2STRINGTABLEWIDGET_H
