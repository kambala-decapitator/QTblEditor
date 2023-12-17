#ifndef QTCOMPAT_H
#define QTCOMPAT_H

#include <QLatin1String>

class QFont;
class QLineEdit;

namespace qtcompat
{
void setRegexValidator(QLatin1String pattern, QLineEdit *lineEdit);
int fontMetricsHorizontalAdvance(const QFont &font, QString text);
QString regexEscape(QString pattern);
}

#endif // QTCOMPAT_H
