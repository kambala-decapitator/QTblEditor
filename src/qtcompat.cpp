#include "qtcompat.h"

#include <QFont>
#include <QFontMetrics>
#include <QLineEdit>

#if IS_QT5
#include <QRegularExpressionValidator>
#else
#include <QRegExpValidator>
#endif

namespace qtcompat
{
void setRegexValidator(QLatin1String pattern, QLineEdit *lineEdit)
{
#if IS_QT5
    QValidator *validator = new QRegularExpressionValidator(QRegularExpression(pattern), lineEdit);
#else
    QValidator *validator = new QRegExpValidator(QRegExp(pattern), lineEdit);
#endif
    lineEdit->setValidator(validator);
}

int fontMetricsHorizontalAdvance(const QFont &font, QString text)
{
    const QFontMetrics fontMetrics(font);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return fontMetrics.horizontalAdvance(text);
#else
    return fontMetrics.width(text);
#endif
}

QString regexEscape(QString pattern)
{
#if IS_QT5
    return QRegularExpression::escape(pattern);
#else
    return QRegExp::escape(pattern);
#endif
}
}
