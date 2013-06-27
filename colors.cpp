int colorHeaderSize = 2; // const
int colorsNum = 13; // const

#include <QColor>
// first bytes of color code: symbols in Unicode - U+00FF, U+0063
const QChar colorBytes[] = {0x00FF, 0x0063};
QString colorHeader(colorBytes, colorHeaderSize); // string representation of colorBytes; const

// last byte in the color code. these are default Blizzard's colors
QList<QChar> colorCodes = QList<QChar>()
						  << 0x002F
						  << 0x0030
						  << 0x0031
						  << 0x0032
						  << 0x0033
						  << 0x0034
						  << 0x0035
						  << 0x0036
						  << 0x0037
						  << 0x0038
						  << 0x0039
						  << 0x003A
						  << 0x003B;

// user-readable color codes: colorHeader + colorCodes.at(i) == colorStrings.at(i+1)
QStringList colorStrings = QStringList()
						   << "\\color;" // if there's only colorHeader
						   << "\\white;"
						   << "\\grey;"
						   << "\\red;"
						   << "\\green;"
						   << "\\blue;"
						   << "\\gold;"
						   << "\\dgrey;"
						   << "\\black;"
						   << "\\tan;"
						   << "\\orange;"
						   << "\\yellow;"
						   << "\\dgreen;"
						   << "\\purple;";

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
					   << QColor(150, 90, 250);
