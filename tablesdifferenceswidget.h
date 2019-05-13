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
    QListWidget *listWidget() const { return ui.rowsListWidget; }

    void clear() { ui.rowsListWidget->clear(); }
    void addRows(const QStringList &rowStrings);

signals:
    void refreshRequested(TablesDifferencesWidget *w);
    void modifyTextRequested(bool fromLeftToRight, bool append);

private slots:
    void refreshButtonClicked() { emit refreshRequested(this); }
    void modifyTextClicked();

private:
    Ui::TablesDifferencesWidget ui;
    DiffType _diffType;
};

#endif // TABLESDIFFERENCESWIDGET_H
