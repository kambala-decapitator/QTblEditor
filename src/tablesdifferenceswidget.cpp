#include "tablesdifferenceswidget.h"


TablesDifferencesWidget::TablesDifferencesWidget(QWidget *parent, DiffType diffType) : QWidget(parent), _diffType(diffType)
{
    ui.setupUi(this);
    setWindowFlags(Qt::Tool);
    setAttribute(Qt::WA_DeleteOnClose);

    QString title;
    switch (diffType)
    {
    case TablesDifferencesWidget::Keys:
        title = tr("Different keys");
        break;
    case TablesDifferencesWidget::Strings:
        title = tr("Different strings");
        break;
    case TablesDifferencesWidget::KeysOrStrings:
        title = tr("Different keys & strings");
        break;
    case TablesDifferencesWidget::SameStrings:
        title = tr("Same strings");
        break;
    default:
        break;
    }
    setWindowTitle(title);

    connect(ui.refreshButton, SIGNAL(clicked()), SLOT(refreshButtonClicked()));
    foreach (QPushButton *b, QList<QPushButton *>() << ui.replaceL2RButton << ui.replaceR2LButton << ui.appendL2RButton << ui.appendR2LButton)
        connect(b, SIGNAL(clicked()), SLOT(modifyTextClicked()));
}

void TablesDifferencesWidget::addRows(const QStringList &rowStrings)
{
    ui.rowsListWidget->addItems(rowStrings);
    ui.rowsListWidget->setToolTip(QString("%1 items").arg(rowStrings.size()));
}

void TablesDifferencesWidget::modifyTextClicked()
{
    bool fromLeftToRight, append;
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    if (b == ui.replaceL2RButton)
    {
        fromLeftToRight = true;
        append = false;
    }
    else if (b == ui.replaceR2LButton)
    {
        fromLeftToRight = false;
        append = false;
    }
    else if (b == ui.appendL2RButton)
    {
        fromLeftToRight = true;
        append = true;
    }
    else // appendR2LButton
    {
        fromLeftToRight = false;
        append = true;
    }
    emit modifyTextRequested(fromLeftToRight, append);
}
