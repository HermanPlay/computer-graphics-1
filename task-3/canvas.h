#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include "figure.h"
#include "qevent.h"
#include <vector>
#include "tool.h"

class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);
    void setCurrentTool(Tool *tool);
    void drawLine(const QPoint& start, const QPoint& end); // Custom line drawing method
    const std::vector<Figure*>& getFigures() const { return figures; } // Provide access to figures
    void clearFigures(); // Method to clear all figures
    void saveFigures(const QString& filePath); // Method to save all figures
    void loadFigures(const QString& filePath); // Method to load figures from a file
    bool isAntiAliasingEnabled() const;
    void setAntiAliasingEnabled(bool enabled);
    void selectFigure(Figure* figure) {
        selectedFigure = figure; 
    };
    ~Canvas();
    Figure* selectedFigure;
signals:
    void antiAliasingToggled(bool enabled); // Signal for anti-aliasing toggle
    void figureSelected(const QColor& color, int thickness); // Signal for figure selection
    void figureDeselected(); // Signal for figure deselection

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override; // Ensure this is declared

private:
    Tool *currentTool;
    std::vector<Figure*> figures;
    bool antiAliasingEnabled = false; // Tracks anti-aliasing state

    Figure* getFigureAt(const QPoint& pos); // Helper to find a figure at a given position
};


#endif // CANVAS_H
