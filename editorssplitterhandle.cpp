#include "editorssplitterhandle.h"

#include <QPushButton>
#include <QVBoxLayout>


EditorsSplitterHandle::EditorsSplitterHandle(Qt::Orientation orientation, QSplitter *parent) : QSplitterHandle(orientation, parent), _w(0)
{
    static const QString kReplaceText = tr("Replace %1", "param is arrow"), kAppendText = tr("Append %1", "param is arrow");

    _replaceRight = new QPushButton(kReplaceText.arg(QChar(0x2192)), this);
    _replaceLeft  = new QPushButton(kReplaceText.arg(QChar(0x2190)), this);
    _appendRight  = new QPushButton(kAppendText.arg( QChar(0x2192)), this);
    _appendLeft   = new QPushButton(kAppendText.arg( QChar(0x2190)), this);
    foreach (QPushButton *b, QList<QPushButton *>() << _replaceRight << _replaceLeft << _appendRight << _appendLeft)
        b->setCursor(Qt::ArrowCursor);

    _w = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(_w);
    layout->setContentsMargins(QMargins());
    layout->addStretch();
    layout->addWidget(_replaceRight);
    layout->addWidget(_replaceLeft);
    layout->addStretch();
    layout->addWidget(_appendRight);
    layout->addWidget(_appendLeft);
    layout->addStretch();
}
