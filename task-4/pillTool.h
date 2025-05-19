#ifndef PILLTOOL_H
#define PILLTOOL_H

#include "tool.h"
#include "pill.h"
#include <QPoint>

class PillTool : public Tool {
private:
    int step; 
    QPoint center1; 
    QPoint center2; 
    Pill* previewPill; 

public:
    PillTool();
    ~PillTool();

    Figure* onMousePress(QMouseEvent* event) override;
    void onMouseMove(QMouseEvent* event) override;
    void draw(QPainter& painter) override;
    std::string name() const override;
    void onMouseRelease(QMouseEvent* event) override {};
};

#endif // PILLTOOL_H