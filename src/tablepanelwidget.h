#ifndef TABLEPANELWIDGET_H
#define TABLEPANELWIDGET_H

#include "ui_tablepanelwidget.h"
#include "d2stringtablewidget.h"

#include <QFileInfo>


class TablePanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TablePanelWidget(QWidget *parent = 0) : QWidget(parent) { ui.setupUi(this); }

    D2StringTableWidget *tableWidget() { return ui.tableWidget; }

    QString absoluteFileName() const;
    QString fileName() const { return QFileInfo(absoluteFileName()).fileName(); }
    QString fileDirPath() const { return QFileInfo(absoluteFileName()).canonicalPath(); }

    void setFilePath(const QString &newFilePath);
    void setActive(bool isActive);
    void clearContents();
    void setWindowModified(bool isModified);
    void updateRowCountLabel();

private:
    Ui::TablePanelWidgetClass ui;

    QString fileNameWithoutBold() const;
    QString labelTextWithoutBold() const;
};

#endif // TABLEPANELWIDGET_H
