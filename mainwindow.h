
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "modelview.h"

#include <QGraphicsScene>
#include <QMainWindow>
#include <QMap>
#include <QToolButton>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void modelChanged(const QString &model);
    void spaceChanged(const QString &space);
    void drawModel();
    void save();
    void open();
    void updateZoomButtons();
private:
    Ui::MainWindow *ui;
    QMap<mouseTool, QToolButton*> toolButtons;
    void setMouseTool(mouseTool mouseTool);
};

#endif // MAINWINDOW_H
