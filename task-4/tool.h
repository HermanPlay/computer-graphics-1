#ifndef TOOL_H
#define TOOL_H

#include <QMouseEvent>
#include <string>
#include "figure.h"
#include "qevent.h"

class Tool {
public:
    virtual ~Tool() = default;
    virtual Figure* onMousePress(QMouseEvent *event) = 0;
    virtual bool onMouseMove(QMouseEvent *event) = 0;
    virtual std::string name() const = 0;
    virtual void draw(QPainter &painter) = 0;
    virtual void onMouseRelease(QMouseEvent *evnet) = 0;

};

#endif // TOOL_H
