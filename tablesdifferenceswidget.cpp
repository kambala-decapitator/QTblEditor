#include "tablesdifferenceswidget.h"


TablesDifferencesWidget::TablesDifferencesWidget(QWidget *parent, DiffType diffType) : QWidget(parent), _diffType(diffType)
{
    ui.setupUi(this);
	setWindowFlags(Qt::Tool);
	setAttribute(Qt::WA_DeleteOnClose);

	if (diffType == TablesDifferencesWidget::Keys)
		setWindowTitle(tr("Different keys"));
	else if (diffType == TablesDifferencesWidget::Strings)
		setWindowTitle(tr("Different strings"));
	else
		setWindowTitle(tr("Different keys & strings"));

	connect(ui.refreshButton, SIGNAL(clicked()), SLOT(refreshButtonClicked()));
	connect(ui.rowsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SIGNAL(rowDoubleClicked(QListWidgetItem *)));
}

void TablesDifferencesWidget::addRows(QStringList rowStrings)
{
	ui.rowsListWidget->addItems(rowStrings);
	ui.rowsListWidget->setToolTip(QString("%1 items").arg(rowStrings.size()));
}
