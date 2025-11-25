int colorHeaderSize = 2; // const
int colorSize = colorHeaderSize + 1; // const
int colorsNum = 13; // const

#include <QColor>
#include <QLatin1String>

// first bytes of color code: symbols in Unicode - U+00FF, U+0063
const QChar colorBytes[] = {ushort(0x00FF), ushort(0x0063)};
QString colorHeader(colorBytes, colorHeaderSize); // string representation of colorBytes; const

// last byte in the color code. these are default Blizzard's colors
QList<QChar> colorCodes = QList<QChar>()
                          << ushort(0x002F)
                          << ushort(0x0030)
                          << ushort(0x0031)
                          << ushort(0x0032)
                          << ushort(0x0033)
                          << ushort(0x0034)
                          << ushort(0x0035)
                          << ushort(0x0036)
                          << ushort(0x0037)
                          << ushort(0x0038)
                          << ushort(0x0039)
                          << ushort(0x003A)
                          << ushort(0x003B)
    ;

// user-readable color codes: colorHeader + colorCodes.at(i) == colorStrings.at(i+1)
QStringList colorStrings = QStringList()
                           << QLatin1String("\\color;") // if there's only colorHeader
                           << QLatin1String("\\white;")
                           << QLatin1String("\\grey;")
                           << QLatin1String("\\red;")
                           << QLatin1String("\\green;")
                           << QLatin1String("\\blue;")
                           << QLatin1String("\\gold;")
                           << QLatin1String("\\dgrey;")
                           << QLatin1String("\\black;")
                           << QLatin1String("\\tan;")
                           << QLatin1String("\\orange;")
                           << QLatin1String("\\yellow;")
                           << QLatin1String("\\dgreen;")
                           << QLatin1String("\\purple;")
    ;

// real colors
QList<QColor> colors = QList<QColor>()
                       << QColor(Qt::white)
                       << QColor(Qt::lightGray)
                       << QColor(Qt::red)
                       << QColor(Qt::green)
                       << QColor(80, 80, 200)
                       << QColor(160, 145, 105)
                       << QColor(Qt::darkGray)
                       << QColor(Qt::black)
                       << QColor(170, 160, 120)
                       << QColor(250, 170, 35)
                       << QColor(Qt::yellow)
                       << QColor(Qt::darkGreen)
                       << QColor(150, 90, 250)
    ;
