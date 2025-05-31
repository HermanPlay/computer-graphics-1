#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cube.h"
#include <QSlider> // Add QSlider include

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
    Cube *cube; // Make sure this is accessible, or pass slider values to it

private slots: // Add private slots for slider value changes
    void on_xRotationSlider_valueChanged(int value);
    void on_yRotationSlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    // Cube *cubeWidget; // If you instantiate Cube in the .cpp and add to layout
};
#endif // MAINWINDOW_H
