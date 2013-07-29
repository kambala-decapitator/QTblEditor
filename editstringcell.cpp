#include "editstringcell.h"
#include "tblstructure.h"
#include "editcolorsdialog.h"

#include <QMenu>

#include <QStack>
#include <QTextCodec>


// convenience function
QString colorHexString(const QColor &c)
{
    QChar zeroChar = QChar('0');
    return QString("#%1%2%3").arg(c.red(), 2, 16, zeroChar).arg(c.green(), 2, 16, zeroChar).arg(c.blue(), 2, 16, zeroChar);
}


extern QList<QChar> colorCodes;
extern QStringList colorStrings;
extern QList<QColor> colors;
extern int colorsNum;

EditStringCell::EditStringCell(QWidget *parent, KeyValueItemsPair keyValueItemsPairToEdit) : QWidget(parent)
{
    ui.setupUi(this);

    QMenu *genderNumberMenu = new QMenu(ui.genderNumberButton);
    genderNumberMenu->setObjectName("GenderNumberMenu");
    QString genderAbbreviations[] = {
        "[ms]",
        "[fs]",
        "[ns]",
        "[nl]",
        "[pl]",
        "[mp]",
        "[fp]"
    };
    QString genders[] = {
        tr("Masculine singular"),
        tr("Feminine singular"),
        tr("Neutral singular"),
        tr("Neutral"),
        tr("Plural"),
        tr("Masculine plural"),
        tr("Feminine plural")
    };
    for (int i = 0; i < 7; i++)
        genderNumberMenu->addAction(QString("%1 %2").arg(genderAbbreviations[i], genders[i]), this, SLOT(insertText()));
    ui.genderNumberButton->setMenu(genderNumberMenu);

    _colorMenu = new QMenu(ui.colorButton);
    _colorMenu->setObjectName("ColorMenu");
    updateColorsMenu();
    ui.colorButton->setMenu(_colorMenu);

    connect(ui.wrapCheckBox, SIGNAL(toggled(bool)), SLOT(wrapModeChanged(bool)));
    connect(ui.keyLineEdit, SIGNAL(textChanged(QString)), SLOT(calculateKeyHashValue()));
    connect(ui.stringEdit, SIGNAL(textChanged()), SLOT(updateCharsEditCounter()));
    connect(ui.stringEdit, SIGNAL(textChanged()), SLOT(setPreviewText()));
    connect(ui.reversePreviewTextCheckBox, SIGNAL(toggled(bool)), SLOT(setPreviewText()));

    setItem(keyValueItemsPairToEdit);
}

void EditStringCell::wrapModeChanged(bool state)
{
    ui.stringEdit->setLineWrapMode((QPlainTextEdit::LineWrapMode)state);
    ui.stringPreview->setLineWrapMode((QTextEdit::LineWrapMode)state);
}

void EditStringCell::insertText()
{
    QAction *menuItem = qobject_cast<QAction *>(sender());
    if (menuItem->parentWidget()->objectName() == "GenderNumberMenu")
        ui.stringEdit->insertPlainText(menuItem->text().left(4));
    else
        ui.stringEdit->insertPlainText(menuItem->text());
}

EditStringCell::SaveResult EditStringCell::saveChanges()
{
    QString keyText = ui.keyLineEdit->text(), valueText = ui.stringEdit->toPlainText();
    int result = EditStringCell::NothingChanged;
    if (_keyValueItemsPair.first->text() != keyText)
    {
        _keyValueItemsPair.first->setText(keyText);
        result |= KeyChanged;
    }
    if (_keyValueItemsPair.second->text() != valueText)
    {
        _keyValueItemsPair.second->setText(valueText);
        result |= ValueChanged;
    }
    return (EditStringCell::SaveResult)result;
}

void EditStringCell::changeItem(bool toNext)
{
    int newItemIndexOffset = toNext ? 1 : -1, row = _keyValueItemsPair.first->row();
    QTableWidget *w = _keyValueItemsPair.first->tableWidget();
    setItem(KeyValueItemsPair(w->item(row + newItemIndexOffset, 0), w->item(row + newItemIndexOffset, 1)));
}

void EditStringCell::setItem(KeyValueItemsPair newKeyValueItemsPair)
{
    _keyValueItemsPair = newKeyValueItemsPair;
    resetText();
}

void EditStringCell::resetText()
{
    ui.keyLineEdit->setText(_keyValueItemsPair.first->text());
    ui.stringEdit->setPlainText(_keyValueItemsPair.second->text());
    updateCharsEditCounter();
}

void EditStringCell::calculateKeyHashValue()
{
    ui.hashValueLineEdit->setText(QString("0x%1").arg(TblStructure::hashValue(TblStructure::encodeKey(ui.keyLineEdit->text()).data(),
                                                                              _keyValueItemsPair.first->tableWidget()->rowCount()), 0, 16));
}

void EditStringCell::setPreviewText()
{
    ui.stringPreview->clear();

    // text is white by default + stupid HTML with its newlines
    QString text = "\\white;" + ui.stringEdit->toPlainText().replace('\n', "<br>");
    if (ui.reversePreviewTextCheckBox->isChecked())
    {
        QList<QPair<int, int> > colorStringsIndeces; // <index_of_color_string_in_array, position_in_string>
        for (int i = 0; i < colors.size(); i++)
        {
            QString colorString = colorStrings.at(i + 1);
            int occurencesCount = text.count(colorString), position = 0, length = colorString.length();
            for (int j = 0; j < occurencesCount; j++, position += length)
            {
                position = text.indexOf(colorString, position);
                colorStringsIndeces.append(qMakePair(i + 1, position));
            }
        }

        // sort colorStringsIndeces by position in ascending order
        int colorsNumberInString = colorStringsIndeces.size();
        for (int i = 0; i < colorsNumberInString - 1; i++)
        {
            QPair<int, int> a = colorStringsIndeces.at(i);
            for (int j = i + 1; j < colorsNumberInString; j++)
            {
                QPair<int, int> b = colorStringsIndeces.at(j);
                if (a.second > b.second)
                {
                    colorStringsIndeces[i] = b;
                    colorStringsIndeces[j] = a;
                    a = b;
                }
            }
        }

        QStack<QString> colorStringsStack;
        for (int i = 0; i < colorsNumberInString; i++)
        {
            int index = colorStringsIndeces.at(i).first;
            int position = colorStringsIndeces.at(i).second + colorStrings.at(index).length(); // skip colorString
            QString coloredText = text.mid(position, i != colorsNumberInString - 1 ? colorStringsIndeces.at(i + 1).second - position : -1);

            QStringList lines = coloredText.split("<br>");
            QString reversedLines;
            for (QStringList::const_iterator i = lines.end() - 1; i != lines.begin() - 1; i--)
            {
                reversedLines.append(*i);
                if (i != lines.begin())
                    reversedLines.append("<br>");
            }
            if (!reversedLines.isEmpty())
                colorStringsStack.push(QString("<font color = \"%1\">%2</font>").arg(colorHexString(colors.at(index - 1)), reversedLines));
        }

        // empty stack
        while (!colorStringsStack.isEmpty())
            ui.stringPreview->insertHtml(colorStringsStack.pop());
    }
    else
    {
        for (int i = 0; i < colors.size(); i++) // replace color codes with their hex values for HTML
            text.replace(colorStrings.at(i + 1), QString("</font><font color = \"%1\">").arg(colorHexString(colors.at(i))));
        text.remove(QRegExp("<font color = \"#[\\da-fA-F]{6}\"></font>")); // remove empty tags
        ui.stringPreview->setHtml(text);
    }

    ui.charsPreviewCountLabel->setText(QString::number(ui.stringPreview->toPlainText().length()));
}

void EditStringCell::showEditColorsDialog()
{
    EditColorsDialog dlg(this, colorStrings, colorCodes, colors);
    if (dlg.exec())
    {
        // remove all custom colors that were before
        for (int i = colorsNum, n = colors.size(); i < n; i++)
        {
            colorCodes.removeLast();
            colorStrings.removeLast();
            colors.removeLast();
        }

        QList<EditColorsDialog::ColorInfo> newColors = dlg.colorsInfo();
        foreach (EditColorsDialog::ColorInfo ci, newColors)
        {
            colorCodes.append(ci.Code);
            colorStrings.append(ci.Name);
            colors.append(ci.Color);
        }

        updateColorsMenu();
        emit colorMenuChanged();
    }
}

void EditStringCell::updateColorsMenu()
{
    _colorMenu->clear();

    QPixmap pix(24, 24);
    for (int i = 0; i < colors.size(); i++)
    {
        pix.fill(colors.at(i));
        if (i == colorsNum)
            _colorMenu->addSeparator();
        _colorMenu->addAction(QIcon(pix), colorStrings.at(i + 1), this, SLOT(insertText()));
    }
    _colorMenu->addSeparator();
    _colorMenu->addAction(tr("Edit..."), this, SLOT(showEditColorsDialog()));

    setPreviewText();
}
