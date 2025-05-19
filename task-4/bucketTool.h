\
#ifndef BUCKETTOOL_H
#define BUCKETTOOL_H

#include "tool.h"
#include "canvas.h" 
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QColor>
#include <QPixmap>
#include <QImage>
#include <string>
#include <queue>
#include <vector>

class Canvas; 

class BucketTool : public Tool {
private:
    Canvas* canvas;
    QPoint startPoint;
    QColor targetColor;
    
    QColor currentFillColor;
    QPixmap* activeFillTexture; 
    QImage currentFillTextureImage; 
    bool useTextureFill;

    bool performFill;
    QImage initialCanvasImage; 

public:
    BucketTool(Canvas* canvasInstance);
    ~BucketTool() override;

    Figure* onMousePress(QMouseEvent *event) override;
    bool onMouseMove(QMouseEvent *event) override;
    std::string name() const override;
    void draw(QPainter &painter) override;
    void onMouseRelease(QMouseEvent *event) override;

    void setFillColor(const QColor& color);
    void setFillTexture(QPixmap* texture); 
    QColor getFillColor() const; 
};

#endif // BUCKETTOOL_H
