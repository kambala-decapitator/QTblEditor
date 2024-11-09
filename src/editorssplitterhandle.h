#ifndef EDITORSSPLITTERHANDLE_H
#define EDITORSSPLITTERHANDLE_H

#include <QSplitterHandle>


class QPushButton;

class EditorsSplitterHandle : public QSplitterHandle
{
public:
    EditorsSplitterHandle(Qt::Orientation orientation, QSplitter *parent);
    virtual ~EditorsSplitterHandle() {}

    virtual QSize sizeHint() const { return _w ? QSize(_w->width(), QSplitterHandle::sizeHint().height()) : QSplitterHandle::sizeHint(); }

    void createButtons() {}

    QPushButton *replaceRightButton() const { return _replaceRight; }
    QPushButton *replaceLeftButton()  const { return _replaceLeft;  }
    QPushButton *appendRightButton()  const { return _appendRight;  }
    QPushButton *appendLeftButton()   const { return _appendLeft;   }

protected:
    virtual void showEvent(QShowEvent *e);

private:
    QWidget *_w;
    QPushButton *_replaceRight, *_replaceLeft;
    QPushButton *_appendRight, *_appendLeft;
};

#endif
