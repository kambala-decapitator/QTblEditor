#ifndef TABLESDIFFERENCESWIDGET_H
#define TABLESDIFFERENCESWIDGET_H

#include "ui_tablesdifferenceswidget.h"


class TablesDifferencesWidget : public QWidget
{
    Q_OBJECT

public:
	enum DiffType {Keys, Strings, KeysOrStrings, SameStrings};

	explicit TablesDifferencesWidget(QWidget *parent, DiffType diffType);

	DiffType diffType() const { return _diffType; }
	void clear() { ui.rowsListWidget->clear(); }
	void addRows(QStringList rowStrings);

signals:
	void refreshRequested(TablesDifferencesWidget *w);
	void rowDoubleClicked(QListWidgetItem *rowItem);

private slots:
	void refreshButtonClicked() { emit refreshRequested(this); }

private:
    Ui::TablesDifferencesWidget ui;
	DiffType _diffType;
};

#endif // TABLESDIFFERENCESWIDGET_H
