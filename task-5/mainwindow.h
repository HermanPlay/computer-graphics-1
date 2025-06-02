#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "rasterizer.h"
#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>

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
protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void updateMousePosition(int x, int y);

private:
    Ui::MainWindow *ui;
    QLabel *mousePositionLabel;
    Rasterizer *rasterizer; 
};
#endif // MAINWINDOW_H
