#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "modelview.h"
#include "modelscene.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , toolButtons()
{
    ui->setupUi(this);

    ui->modelGraphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    //TODOs: clean up zooming indices in ModelView, introduce panning, drawing and deleting events + slots.

    connect(ui->spaceComboBox, &QComboBox::currentTextChanged, this, &MainWindow::spaceChanged);
    connect(ui->modelComboBox, &QComboBox::currentTextChanged, this, &MainWindow::modelChanged);

    connect(ui->savePushButton, &QPushButton::clicked, this, &MainWindow::save);
    connect(ui->loadPushButton, &QPushButton::clicked, this, &MainWindow::open);

    connect(ui->parameter1LineEdit, &QLineEdit::textEdited, this, &MainWindow::drawModel);
    connect(ui->parameter2LineEdit, &QLineEdit::textEdited, this, &MainWindow::drawModel);
    connect(ui->modelComboBox, &QComboBox::currentTextChanged, this, &MainWindow::drawModel);

    connect(ui->zoomInToolButton, &QToolButton::clicked, this, [this](){ui->modelGraphicsView->zoomIn();
                                                                        updateZoomButtons();});
    connect(ui->zoomOutToolButton, &QToolButton::clicked, this, [this](){ui->modelGraphicsView->zoomOut();
                                                                        updateZoomButtons();});
    connect(ui->zoomToExtentsToolButton, &QToolButton::clicked, this, [this](){ui->modelGraphicsView->zoomToExtents();
                                                                        updateZoomButtons();});

    ui->panToolButton->setDisabled(true);

    toolButtons = {{Select, ui->selectToolButton},
                   {Pan, ui->panToolButton},
                   {Pencil, ui->pencilToolButton},
                   {DrawRectangle, ui->drawRectangleToolButton},
                   {DrawEllipse, ui->drawEllipseToolButton},
                   {Delete, ui->deleteToolButton}};

    connect(ui->selectToolButton, &QToolButton::clicked, this, [this](){setMouseTool(Select);});
    connect(ui->panToolButton, &QToolButton::clicked, this, [this](){setMouseTool(Pan);});
    connect(ui->pencilToolButton, &QToolButton::clicked, this, [this](){setMouseTool(Pencil);});
    connect(ui->drawRectangleToolButton, &QToolButton::clicked, this, [this](){setMouseTool(DrawRectangle);});
    connect(ui->drawEllipseToolButton, &QToolButton::clicked, this, [this](){setMouseTool(DrawEllipse);});
    connect(ui->deleteToolButton, &QToolButton::clicked, this, [this](){setMouseTool(Delete);});

    connect(ui->actionUndo, &QAction::triggered, ui->modelGraphicsView, &ModelView::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->modelGraphicsView, &ModelView::redo);

    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
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

    if (space == "Select"){
        ui->modelComboBox->setDisabled(true);
    }
    else{
        ui->modelComboBox->setEnabled(true);
    }

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
    QJsonObject inputs;
    inputs["Space"] = ui->spaceComboBox->currentText();
    inputs["Units"] = ui->unitsComboBox->currentText();
    inputs["Model"] = ui->modelComboBox->currentText();
    inputs["Parameter 1"] = ui->parameter1LineEdit->text();
    inputs["Parameter 2"] = ui->parameter2LineEdit->text();



    inputs["Shapes"] = ui->modelGraphicsView->shapesToJson();

    QDir mdir;
    QString path = "D:/dialog_files/";
    if (!mdir.exists(path)){
        mdir.mkpath(path);
    }

    QFile input_save(QFileDialog::getSaveFileName(this, QString(), path));

    if (!input_save.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this, "Save failed", path + "inputs.json could not be opened for writing.");
        return;
    }
    //Saves inputs to the file input_save by converting to QJsonDocument and then QByteArray.
    input_save.write(QJsonDocument(inputs).toJson());
    input_save.close();
}

void MainWindow::open(){
    QDir mdir;
    QString path = "D:/dialog_files/";
    if (!mdir.exists(path)){
        mdir.mkpath(path);
    }

    QString filepath = QFileDialog::getOpenFileName(this, QString(), path);
    QFile inputs_file(filepath);

    if (!inputs_file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "Load failed", "File could not be opened for reading.");
        return;
    }

    QByteArray inputs_json = inputs_file.readAll();

    QJsonObject inputs = QJsonDocument::fromJson(inputs_json).object();

    for (const QString &key : {"Space", "Units", "Model", "Parameter 1", "Parameter 2", "Shapes"}){
        if (!inputs.contains(key)){
            QMessageBox::warning(this, "Invalid inputs file", "File did not have a value for parameter " + key + ".");
            return;
        }
    }

    ui->spaceComboBox->setCurrentText(inputs["Space"].toString());
    ui->unitsComboBox->setCurrentText(inputs["Units"].toString());
    ui->modelComboBox->setCurrentText(inputs["Model"].toString());
    ui->parameter1LineEdit->setText(inputs["Parameter 1"].toString());
    ui->parameter2LineEdit->setText(inputs["Parameter 2"].toString());

    drawModel();
    ui->modelGraphicsView->jsonToShapes(inputs["Shapes"].toArray());

    inputs_file.close();
}

void MainWindow::drawModel(){
    //clear the window
    ui->modelGraphicsView->getModelScene()->clear();

    bool success1, success2;

    float p1 = ui->parameter1LineEdit->text().toFloat(&success1);
    float p2 = ui->parameter2LineEdit->text().toFloat(&success2);

    if (!(success1 && success2)){
        ui->warningLabel->setEnabled(true);
        ui->warningLabel->setText("<p style=\"color: red\">All parameters must be numbers.</p>");
        return;
    }

    if (p1 < 0 || p2 < 0){
        ui->warningLabel->setEnabled(true);
        ui->warningLabel->setText("<p style=\"color: red\">Parameters must be positive numbers.</p>");
        return;
    }

    ui->warningLabel->setText("");
    ui->warningLabel->setDisabled(true);

    QString model = ui->modelComboBox->currentText();

    ui->modelGraphicsView->getModelScene()->drawModel(model, p1, p2);

    ui->modelGraphicsView->zoomToExtents();
    ui->modelGraphicsView->show();
}

void MainWindow::setMouseTool(mouseTool mouseTool){
    ui->modelGraphicsView->setMouseTool(mouseTool);
    for (QToolButton *button : qAsConst(toolButtons)){
        button->setEnabled(true);}
    toolButtons[mouseTool]->setDisabled(true);
}

void MainWindow::updateZoomButtons(){
    if (ui->modelGraphicsView->getZoomScale() == ui->modelGraphicsView->zoomScales.constLast()){
        //if the model view is fully zoomed in
        ui->zoomInToolButton->setDisabled(true);
    }
    else{
        ui->zoomInToolButton->setEnabled(true);
    }

    if (ui->modelGraphicsView->getZoomScale() == ui->modelGraphicsView->zoomScales.constFirst()){
        //if the model view is fully zoomed out
        ui->zoomOutToolButton->setDisabled(true);
    }
    else{
        ui->zoomOutToolButton->setEnabled(true);
    }
}
