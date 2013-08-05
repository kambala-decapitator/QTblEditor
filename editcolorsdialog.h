#ifndef EDITCOLORSDIALOG_H
#define EDITCOLORSDIALOG_H

#include "ui_editcolorsdialog.h"


class EditColorsDialog : public QDialog
{
    Q_OBJECT

public:
    struct ColorInfo
    {
        QString Name;
        QChar Code;
        QColor Color;

        ColorInfo() {}
        ColorInfo(const QString &s, const QChar &ch, const QColor &c) : Name(s), Code(ch), Color(c) {}
    };

    explicit EditColorsDialog(QWidget *parent, const QStringList &colorStrings, const QList<QChar> &colorCodes, const QList<QColor> &colors);

    QList<ColorInfo> colorsInfo() const { return _colorsInfo; }

public slots:
    void accept();

private slots:
    void showColorInfo(QListWidgetItem *newItem, QListWidgetItem *oldItem);
    void setListItemText(const QString &text) { ui.colorsListWidget->currentItem()->setText(text); }
    void addColor();
    void deleteColor();
    void enableOkButton();
    void chooseColor();
    void changeRgbNumbersDisplay(bool isDec);
    void rgbValueChanged() { updateColorLabel(currentRgbColor()); }

private:
    Ui::EditColorsDialog ui;
    QList<ColorInfo> _colorsInfo;

    void updateColorDisplay(const QColor &newColor);
    void updateColorLabel(const QColor &newColor);
    QRgb currentRgbColor() const;
    void saveCurrentColor(int row);
};

#endif // EDITCOLORSDIALOG_H
