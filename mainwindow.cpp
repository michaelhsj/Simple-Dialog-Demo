
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include<QJsonDocument>
#include <QJsonObject>
#include <QVector>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , modelScene(new QGraphicsScene)
    , zoomScale(100)
    , modelRect()
    , toolType(QString("Pan"))
{
    ui->setupUi(this);

    ui->modelGraphicsView->setScene(modelScene);

    connect(ui->spaceComboBox, &QComboBox::currentTextChanged, this, &MainWindow::spaceChanged);
    connect(ui->modelComboBox, &QComboBox::currentTextChanged, this, &MainWindow::modelChanged);

    connect(ui->savePushButton, &QPushButton::clicked, this, &MainWindow::save);
    connect(ui->loadPushButton, &QPushButton::clicked, this, &MainWindow::open);

    //refactor drawModel and zoom methods into graphicsview/graphicsscene subclasses?

    //refactor zoom into view (let it hold zoomscale), drawmodel into scene?
    //how will we call zoomtoextents in drawmodel then? call it in mainwindow, which calls smth in graphicsscene?

    connect(ui->parameter1LineEdit, &QLineEdit::textEdited, this, &MainWindow::drawModel);
    connect(ui->parameter2LineEdit, &QLineEdit::textEdited, this, &MainWindow::drawModel);
    connect(ui->modelComboBox, &QComboBox::currentTextChanged, this, &MainWindow::drawModel);

    connect(ui->zoomInToolButton, &QToolButton::clicked, this, &MainWindow::zoomIn);
    connect(ui->zoomOutToolButton, &QToolButton::clicked, this, &MainWindow::zoomOut);
    connect(ui->zoomToExtentsToolButton, &QToolButton::clicked, this, &MainWindow::zoomToExtents);

    //give each toolbutton a string for tool name, have them all do the same thing on click?
    //can add string as property via Design menu.
    //use enums instead of qstrings because qstrings are heavy?

    connect(ui->panToolButton, &QToolButton::clicked, this, &MainWindow::setTool);
    connect(ui->drawRectangleToolButton, &QToolButton::clicked, this, &MainWindow::setTool);
    connect(ui->drawEllipseToolButton, &QToolButton::clicked, this, &MainWindow::setTool);
    connect(ui->deleteToolButton, &QToolButton::clicked, this, &MainWindow::setTool);
}

MainWindow::~MainWindow()
{
    delete modelScene;
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

    for (const QString &key : {"Space", "Units", "Model", "Parameter 1", "Parameter 2"}){
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
    inputs_file.close();
}

void MainWindow::drawModel(){
    //clear the window

    modelScene->clear();

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

    float scalingFactor = 300 / fmax(p1, p2);
    p1 *= scalingFactor;
    p2 *= scalingFactor;

    //rescale p1 and p2?

    QColor borderColor = QColor(106,151,201);
    QColor shadedColor = QColor(114,158,206);
    QColor litColor = QColor(164,194,225);

    QPoint TopLeft;

    if (ui->modelComboBox->currentText() == "U"){
        float angleFactor = 0.25;
        // p1 = cylinder radius
        // p2 = cylinder height
        // draw upright cylinder
        modelScene->addEllipse(-p1, p2/2 - p1 * angleFactor, 2*p1, 2*p1*angleFactor, QPen(borderColor), QBrush(shadedColor));
        modelScene->addRect(-p1, -p2/2, 2 * p1, p2, QPen(borderColor), QBrush(shadedColor));
        modelScene->addEllipse(-p1, -p2/2 - p1 * angleFactor, 2*p1, 2*p1*angleFactor, QPen(borderColor), QBrush(litColor));

        TopLeft.setX(-p1);
        TopLeft.setY(-p2/2 - p1);
    }
    else if (ui->modelComboBox->currentText() == "B"){
        float angleFactor = 0.25;
        // p1 = cylinder radius
        // p2 = cylinder width
        //draw cylinder on its side
        modelScene->addEllipse(p2/2 -p1*angleFactor, -p1, 2*p1*angleFactor, 2*p1, QPen(borderColor), QBrush(shadedColor));
        modelScene->addRect(-p2/2, -p1, p2, 2*p1, QPen(borderColor), QBrush(shadedColor));
        modelScene->addEllipse(-p2/2 -p1*angleFactor, -p1, 2*p1*angleFactor, 2*p1, QPen(borderColor), QBrush(litColor));

        TopLeft.setX(-p2/2 -p1*angleFactor);
        TopLeft.setY(-p1);
    }
    else if (ui->modelComboBox->currentText() == "T"){
        // p1 = radius of cavity
        // p2 = width of square
        // draw box with cavity
        modelScene->addRect(-p2/2, -p2/2, p2, p2, QPen(borderColor), QBrush(shadedColor));
        modelScene->addEllipse(-p1/2, -p1/2, p1, p1, QPen(borderColor), QBrush(QColor("white")));

        TopLeft.setX(-fmax(p1, p2)/2);
        TopLeft.setY(-fmax(p1, p2)/2);
    }

    // We will always place the center of the object at (0, 0), so the bottom right of the shape will
    // be TopLeft reflected.
    modelRect.setTopLeft(TopLeft);
    modelRect.setBottomRight(-TopLeft);

    zoomToExtents();
    ui->modelGraphicsView->show();
}

void MainWindow::zoomIn(){
    QVector zoomScales({50, 75, 100, 125, 150, 200});
    if (zoomScale < zoomScales.constLast()){
        float newScale = zoomScales.at(zoomScales.indexOf(zoomScale) + 1);
        ui->modelGraphicsView->scale(newScale/zoomScale, newScale/zoomScale);
        zoomScale = newScale;
    }
}

void MainWindow::zoomOut(){
    QVector zoomScales({50, 75, 100, 125, 150, 200});
    if (zoomScale > zoomScales.constFirst()){
        float newScale = zoomScales.at(zoomScales.indexOf(zoomScale) - 1);
        ui->modelGraphicsView->scale(newScale/zoomScale, newScale/zoomScale);
        zoomScale = newScale;
    }
}

void MainWindow::zoomToExtents(){
    ui->modelGraphicsView->centerOn(0, 0);
    ui->modelGraphicsView->fitInView(modelRect, Qt::KeepAspectRatio);
    zoomScale = 100;
}

