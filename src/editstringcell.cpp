#include "editstringcell.h"
#include "tblstructure.h"
#include "editcolorsdialog.h"
#include "qtcompat.h"

#include <QMenu>

#include <QStack>
#include <QQueue>
#include <QTextCodec>

#include <utility>


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
extern int colorSize;

QString colorsRegexPattern()
{
    QStringList realColorStrings = colorStrings.mid(1);
    for (int i = 0; i < realColorStrings.size(); ++i)
        realColorStrings[i] = qtcompat::regexEscape(realColorStrings[i]);
    return QString("(?:%1)").arg(realColorStrings.join(QChar('|')));
}


static const QString kGenderNumberMenuName("GenderNumberMenu");

EditStringCell::EditStringCell(QWidget *parent, const KeyValueItemsPair &keyValueItemsPairToEdit, bool renderGreyAsWhite) : QWidget(parent), _renderGreyAsWhite(renderGreyAsWhite)
{
    ui.setupUi(this);

    static const QStringList kGenderAbbreviations = QStringList() << "[ms]" << "[fs]" << "[ns]" << "[nl]" << "[pl]" << "[mp]" << "[fp]";
    static const QStringList kGenders = QStringList() << tr("Masculine singular") << tr("Feminine singular") << tr("Neutral singular") << tr("Neutral") << tr("Plural") << tr("Masculine plural") << tr("Feminine plural");

    QMenu *genderNumberMenu = new QMenu(ui.genderNumberButton);
    genderNumberMenu->setObjectName(kGenderNumberMenuName);
    for (int i = 0; i < kGenderAbbreviations.size(); i++)
        genderNumberMenu->addAction(QString("%1 %2").arg(kGenderAbbreviations.at(i), kGenders.at(i)), this, SLOT(insertText()));
    ui.genderNumberButton->setMenu(genderNumberMenu);

    _colorMenu = new QMenu(ui.colorButton);
    _colorMenu->setObjectName("ColorMenu");
    updateColorsMenu();
    ui.colorButton->setMenu(_colorMenu);

    ui.stringPreview->viewport()->setStyleSheet("background-color: black;");

    connect(ui.wrapCheckBox, SIGNAL(toggled(bool)), SLOT(wrapModeChanged(bool)));
    connect(ui.keyLineEdit, SIGNAL(textChanged(QString)), SLOT(calculateKeyHashValue()));
    connect(ui.stringEdit, SIGNAL(textChanged()), SLOT(updateCharsEditCounter()));
    connect(ui.stringEdit, SIGNAL(textChanged()), SLOT(setPreviewText()));
    connect(ui.stringEdit, SIGNAL(cursorPositionChanged()), SLOT(updateCurrentEditColumn()));
    connect(ui.reversePreviewTextCheckBox, SIGNAL(toggled(bool)), SLOT(setPreviewText()));
    connect(ui.centerPreviewTextCheckBox, SIGNAL(toggled(bool)), SLOT(setPreviewText()));

    setItem(keyValueItemsPairToEdit);
}

void EditStringCell::wrapModeChanged(bool state)
{
    ui.stringEdit   ->setLineWrapMode(state ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    ui.stringPreview->setLineWrapMode(state ?      QTextEdit::WidgetWidth :      QTextEdit::NoWrap);
}

void EditStringCell::insertText()
{
    QAction *menuItem = qobject_cast<QAction *>(sender());
    if (qobject_cast<QWidget *>(menuItem->parent())->objectName() == kGenderNumberMenuName)
        ui.stringEdit->insertPlainText(menuItem->text().left(4));
    else
        ui.stringEdit->insertPlainText(menuItem->text());
}

void EditStringCell::saveChanges()
{
    _keyValueItemsPair.first->setText(ui.keyLineEdit->text());
    _keyValueItemsPair.second->setText(ui.stringEdit->toPlainText());
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
    ui.hashValueLabel->setText(QString("0x%1").arg(TblStructure::hashValue(TblStructure::encodeKey(ui.keyLineEdit->text()).data(), _keyValueItemsPair.first->tableWidget()->rowCount()), 0, 16));
}

void EditStringCell::setPreviewText()
{
    ui.stringPreview->clear();

    // stupid HTML
    QString text = ui.stringEdit->toPlainText().replace('<', "&lt;").replace('>', "&gt;");

    const QString nonBlankSpace("&nbsp;");
#if IS_QT5
    QRegularExpression emptyRe(" {2,}");
    do
    {
        QRegularExpressionMatch match = emptyRe.match(text);
        if (!match.hasMatch())
            break;
        int matchedLength = match.capturedLength();
        text.replace(match.capturedStart(), matchedLength, nonBlankSpace.repeated(matchedLength));
    } while (true);
#else
    int emptyMatchIndex;
    QRegExp emptyRe(" {2,}");
    while ((emptyMatchIndex = emptyRe.indexIn(text)) != -1)
    {
        int matchedLength = emptyRe.matchedLength();
        text.replace(emptyMatchIndex, matchedLength, nonBlankSpace.repeated(matchedLength));
    }
#endif

#if IS_QT5
    static QRegularExpression colorRegex(colorsRegexPattern()); // one of the colors
#else
    static QRegExp colorRegex(colorsRegexPattern()); // one of the colors
#endif
    const int textColorsCount = text.count(colorRegex);

    bool shouldCenterAlign = ui.centerPreviewTextCheckBox->isChecked();
    const QString defaultColor = colorStrings.at(1); // text is white by default
    if (ui.reversePreviewTextCheckBox->isChecked())
    {
        // text and colors flow top-to-bottom, but D2 renders text bottom-to-top
        // HTML renders top-to-bottom

        QString currentGlobalColor = defaultColor;
        QStringList lines = text.split('\n');

        typedef std::pair<QString, QString> ColoredText; // first - text, second - color
        typedef QQueue<ColoredText> LineColoredQueue;

        QStack<LineColoredQueue> lineStack;
        lineStack.reserve(lines.size());

        foreach (const QString &line, lines)
        {
            LineColoredQueue lineQueue;
            int nextColorStart = 0, previousColorEnd = 0;
            do
            {
#if IS_QT5
                QRegularExpressionMatch match = colorRegex.match(line, nextColorStart);
                nextColorStart = match.capturedStart();
#else
                nextColorStart = colorRegex.indexIn(line, nextColorStart);
#endif

                int textLength = nextColorStart;
                if (textLength != -1)
                    textLength -= previousColorEnd;
                lineQueue.enqueue(ColoredText(line.mid(previousColorEnd, textLength), currentGlobalColor));

                if (nextColorStart == -1)
                    break;

#if IS_QT5
                currentGlobalColor = match.captured();
                nextColorStart += match.capturedLength();
#else
                currentGlobalColor = colorRegex.cap();
                nextColorStart += colorRegex.matchedLength();
#endif
                previousColorEnd = nextColorStart;
            } while (true);

            lineStack.push(lineQueue);
        }

        QStringList htmlLines;
        while (!lineStack.isEmpty())
        {
            QString htmlLine;
            LineColoredQueue queue = lineStack.pop();
            while (!queue.isEmpty())
            {
                ColoredText pair = queue.dequeue();
                htmlLine += textWithHtmlColor(pair.first, pair.second);
            }
            htmlLines += htmlLine;
        }
        QString html = htmlLines.join("<br>");
        if (shouldCenterAlign)
            html = QString("<body align=\"center\">%1</body>").arg(html);
        ui.stringPreview->setHtml(html);
    }
    else
    {
        for (int i = 0; i < colors.size(); i++) // replace color codes with their hex values for HTML
            text.replace(colorStrings.at(i + 1), QString("</font><font color = \"%1\">").arg(colorHexString(colorAt(i))));
        ui.stringPreview->setHtml(QString("<body style='color: %1' %3>%2</body>").arg(colorHexFromColorString(defaultColor), text.replace('\n', "<br>"),
                                                                                      shouldCenterAlign ? QLatin1String("align=\"center\"") : QString()));
    }

    const int length = ui.stringPreview->toPlainText().length() + textColorsCount * colorSize;
    ui.charsPreviewCountLabel->setText(QString::number(length));

    const int kMaxLengthPatch110 = 255; // TODO: in Median XL Sigma it's 355
    const bool isTextOverLimit = length > kMaxLengthPatch110;
    ui.charsPreviewCountLabel->setStyleSheet(isTextOverLimit ? LATIN1_STRING_ARGS("color: %1; border: 1px solid %1;").arg(QLatin1String("red")) : QString());

    // show warning only once
    static bool showWarning = true;
    if (showWarning)
    {
        showWarning = false;

        if (isTextOverLimit)
            emit maxLengthExceededFor110(tr("Patch 1.10 has limitation of %1 characters per string").arg(kMaxLengthPatch110));
    }
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

        foreach (const EditColorsDialog::ColorInfo &ci, dlg.colorsInfo())
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
        pix.fill(colorAt(i));
        if (i == colorsNum)
            _colorMenu->addSeparator();
        _colorMenu->addAction(QIcon(pix), colorStrings.at(i + 1), this, SLOT(insertText()))->setIconVisibleInMenu(true);
    }
    _colorMenu->addSeparator();
    _colorMenu->addAction(tr("Edit..."), this, SLOT(showEditColorsDialog()));

    setPreviewText();
}

QString EditStringCell::colorHexFromColorString(const QString &colorString)
{
    return colorHexString(colorAt(colorStrings.indexOf(colorString) - 1));
}

QString EditStringCell::textWithHtmlColor(const QString &text, const QString &colorString)
{
    return QString("<font color = \"%1\">%2</font>").arg(colorHexFromColorString(colorString), text);
}

QColor EditStringCell::colorAt(int i)
{
    if (i == 1 && _renderGreyAsWhite)
        i = 0;
    return colors.at(i);
}
