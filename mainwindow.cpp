
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include<QJsonDocument>
#include <QJsonObject>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->spaceComboBox, &QComboBox::currentTextChanged, this, &MainWindow::spaceChanged);
    connect(ui->modelComboBox, &QComboBox::currentTextChanged, this, &MainWindow::modelChanged);
    connect(ui->savePushButton, &QPushButton::clicked, this, &MainWindow::save);

    connect(ui->parameter1LineEdit, &QLineEdit::textEdited, this, &MainWindow::drawModel);
    connect(ui->parameter2LineEdit, &QLineEdit::textEdited, this, &MainWindow::drawModel);
    connect(ui->modelComboBox, &QComboBox::currentTextChanged, this, &MainWindow::drawModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::spaceChanged(const QString &space)
{
    ui->modelComboBox->clear();
    if (space == "2D"){
        ui->modelComboBox->addItem("T");
    }
    else if (space == "3D"){
        ui->modelComboBox->addItem("U");
        ui->modelComboBox->addItem("B");
    }
    ui->modelComboBox->setEnabled(true);

}


void MainWindow::modelChanged(const QString &model)
{
    ui->parameter_1_label->setText("Radius");

    if (model == "U"){
        ui->parameter_2_label->setText("Height");
    }
    else if (model == "B" || model == "T"){
        ui->parameter_2_label->setText("Width");
    }
}

void MainWindow::save(){

    QJsonObject inputs = *(new QJsonObject());

    inputs["Space"] = ui->spaceComboBox->currentText();
    inputs["Units"] = ui->unitsComboBox->currentText();
    inputs["Model"] = ui->modelComboBox->currentText();
    inputs["Parameter 1"] = ui->parameter1LineEdit->text();
    inputs["Parameter 2"] = ui->parameter2LineEdit->text();

    QDir mdir;
    QString path = "D:/dialog_files/";
    if (mdir.exists(path)){
        mdir.mkpath(path);
    }

    QFile input_save("D:/test/inputs.json");

    if (!input_save.open(QIODevice::WriteOnly)){
        return;
    }
    //Saves inputs to the file input_save by converting to QJsonDocument and then QByteArray.
    input_save.write(QJsonDocument(inputs).toJson());
    input_save.close();
}

void MainWindow::drawModel(){
    //clear the window

    ui->modelGraphicsView->scene()->clear();

    float p1 = ui->parameter1LineEdit->text().toFloat();
    float p2 = ui->parameter2LineEdit->text().toFloat();

    //rescale p1 and p2
    p1 /= std::max(p1, p2);
    p2 /= std::max(p1, p2);

    if (ui->modelComboBox->currentText() == "U"){
        //QGraphicsScene::addEllipse();
        //QGraphicsScene::addEllipse();

        //QGraphicsScene::addRect();

        //draw upright cylinder
    }
    else if (ui->modelComboBox->currentText() == "B"){
        //QGraphicsScene::addEllipse();
        //QGraphicsScene::addEllipse();

        //QGraphicsScene::addRect();
        //draw cylinder on its side
    }
    else if (ui->modelComboBox->currentText() == "T"){
        //QGraphicsScene::addEllipse();
        //QGraphicsScene::addRect();
        //draw circular cavity in a square

    }
}
