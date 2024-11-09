#ifndef QTCOMPAT_H
#define QTCOMPAT_H

#include <QLatin1String>
#include <QPoint>

#if IS_QT5
# ifdef Q_OS_MACOS
#  define OS_MACOS
# endif
#else
# ifdef Q_OS_MAC
#  define OS_MACOS
# endif
#endif // IS_QT5

class QDropEvent;
class QFont;
class QLineEdit;

namespace qtcompat
{
void setRegexValidator(QLatin1String pattern, QLineEdit *lineEdit);
int fontMetricsHorizontalAdvance(const QFont &font, QString text);
QString regexEscape(QString pattern);

QPoint dropEventPos(QDropEvent *e);
}

#endif // QTCOMPAT_H
