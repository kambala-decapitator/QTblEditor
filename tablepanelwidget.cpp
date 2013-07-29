#include "tablepanelwidget.h"

#include <QDir>


QString TablePanelWidget::fileNameWithoutBold() const
{
    return ui.filePathLabel->text().mid(3, ui.filePathLabel->text().size() - 7);
}

QString TablePanelWidget::labelTextWithoutBold() const
{
    QString s = ui.filePathLabel->text();
    return s.contains("<b>") ? fileNameWithoutBold() : s;
}

QString TablePanelWidget::absoluteFileName() const
{
    QString s = labelTextWithoutBold();
    return s.endsWith('*') ? s.left(s.length() - 1) : s;
}

void TablePanelWidget::setFilePath(const QString &newFilePath)
{
    bool isLabelBold = ui.filePathLabel->text().contains("<b>");
    ui.filePathLabel->setText(QDir::toNativeSeparators(newFilePath));
    if (isLabelBold)
        setActive(true);
}

void TablePanelWidget::setActive(bool isActive)
{
    QString currentFileName = ui.filePathLabel->text();
    bool isLabelBold = currentFileName.contains("<b>");
    if (isActive ^ isLabelBold)
        ui.filePathLabel->setText(isActive ? QString("<b>%1</b>").arg(currentFileName) : fileNameWithoutBold());
}

void TablePanelWidget::clearContents()
{
    ui.tableWidget->clearContents();
    ui.tableWidget->setRowCount(0);
    ui.filePathLabel->setText(QString());
}

void TablePanelWidget::setWindowModified(bool isModified)
{
    QWidget::setWindowModified(isModified);
    ui.tableWidget->setWindowModified(isModified);

    QString filePath = labelTextWithoutBold();
    bool hasAsterisk = filePath.endsWith('*');
    if (isModified && !hasAsterisk)
        setFilePath(filePath + '*');
    else if (!isModified && hasAsterisk)
        setFilePath(filePath.left(filePath.length() - 1));
}

void TablePanelWidget::updateRowCountLabel()
{
    int rows = ui.tableWidget->rowCount();
    ui.rowCountLabel->setText(QString("%1 (0x%2)").arg(rows).arg(rows, 0, 16));
}
