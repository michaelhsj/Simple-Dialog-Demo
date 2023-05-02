
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    void modelChanged(const QString &arg1);
    void spaceChanged(const QString &space);
    void save();
    void drawModel();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
