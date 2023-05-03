
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QMainWindow>



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
    void save();
    void drawModel();
    void open();
    void zoomIn();
    void zoomOut();
    void zoomToExtents();
private:
    Ui::MainWindow *ui;
    QGraphicsScene *modelScene;
    float zoomScale;
    QRectF modelRect;
    QString toolType;
};

#endif // MAINWINDOW_H
