#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "canvas.h"
#include "circleTool.h"
#include "pointer.h"
#include "polygonTool.h"
#include "tool.h"
#include "lineTool.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Canvas *canvas; // Add canvas as a member
    Tool* currentTool;

    Pointer* pointerTool;
    LineTool* lineTool;
    PolygonTool* polygonTool;
    CircleTool* circleTool;

private slots:
    void onPointerButtonClicked();
    void onLineButtonClicked();
    void onPolygonButtonClicked();
    void onCircleButtonClicked();
    void setCurrentTool(Tool* tool);
};
#endif // MAINWINDOW_H
