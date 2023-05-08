#include "modelview.h"

#include <QMouseEvent>
#include <QVector>
#include <tuple>
#include <QGraphicsRectItem>
#include <QJsonObject>

const QVector<float> ModelView::zoomScales = QVector({50.0f, 75.0f, 100.0f, 125.0f, 150.0f, 200.0f});

ModelView::ModelView(QWidget *parent)
    : QGraphicsView(parent)
    , modelScene(new ModelScene())
    , zoomScale(100)
    , currentMouseTool(Pan)
{
    setScene(modelScene);
}

ModelView::~ModelView(){
    delete modelScene;
}

void ModelView::zoomIn()
{
    if (zoomScale < zoomScales.constLast()){
        float newScale = zoomScales.at(zoomScales.indexOf(zoomScale) + 1);
        scale(newScale/zoomScale, newScale/zoomScale);
        zoomScale = newScale;
    }

}

void ModelView::zoomOut()
{
    if (zoomScale > zoomScales.constFirst()){
        float newScale = zoomScales.at(zoomScales.indexOf(zoomScale) - 1);
        scale(newScale/zoomScale, newScale/zoomScale);
        zoomScale = newScale;
    }
}

void ModelView::zoomToExtents()
{
    centerOn(0, 0);
    fitInView(modelScene->getModelRect(), Qt::KeepAspectRatio);
    zoomScale = 100;
}

void ModelView::undo()
{
    QGraphicsItem *shape = popFromHistory(Undo);
    if (shape != nullptr){
        pushToHistory(Redo, shape);
        if (scene()->items().contains(shape)){
            scene()->removeItem(shape);
        }
        else{
            scene()->addItem(shape);
        }
    }
}

void ModelView::redo()
{
    QGraphicsItem *shape = popFromHistory(Redo);
    if (shape != nullptr){
        pushToHistory(Undo, shape);
        if (scene()->items().contains(shape)){
            scene()->removeItem(shape);
        }
        else{
            scene()->addItem(shape);
        }
    }
}

void ModelView::setMouseTool(mouseTool mouseTool){
    if (currentMouseTool != Select && mouseTool == Select){
        for (int i = 0; i < scene()->items().size(); i++){
            scene()->items().at(i)->setFlag(QGraphicsItem::ItemIsSelectable, true);
            scene()->items().at(i)->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }
    else if (currentMouseTool == Select && mouseTool != Select){
        for (int i = 0; i < scene()->items().size(); i++){
            scene()->items().at(i)->setFlag(QGraphicsItem::ItemIsSelectable, false);
            scene()->items().at(i)->setFlag(QGraphicsItem::ItemIsMovable, false);
        }
    }

    currentMouseTool = mouseTool;

    if (mouseTool == Pan){
        setDragMode(QGraphicsView::ScrollHandDrag);
    }
    else{
        setDragMode(QGraphicsView::NoDrag);
    }

    if (currentMouseTool == DrawRectangle || currentMouseTool == DrawEllipse){
        setCursor(Qt::CrossCursor);
    }
    else{
        setCursor(Qt::PointingHandCursor);
    }
}

ModelScene *ModelView::getModelScene() const {
    return modelScene;
}

void ModelView::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        if (currentMouseTool == DrawRectangle || currentMouseTool == DrawEllipse){
            startCorner = mapToScene(event->pos());
            startShape();
        }
        else if (currentMouseTool == Delete){
            deleteShape();
        }
        else if (currentMouseTool == Pencil){
            drawnPath = scene()->addPath(QPainterPath(mapToScene(event->pos())), QPen(QBrush(Qt::SolidPattern), 20), QBrush(Qt::SolidPattern));
            pushToHistory(Undo, drawnPath);
            update();
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void ModelView::mouseMoveEvent(QMouseEvent *event){
    if (currentMouseTool == DrawRectangle || currentMouseTool == DrawEllipse){
        updateShape(mapToScene(event->pos()));
    }
    else if (currentMouseTool == Pencil){
        drawnPath->path().lineTo(mapToScene(event->pos()));
        update();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ModelView::mouseReleaseEvent(QMouseEvent *event){
    if (currentMouseTool == DrawRectangle || currentMouseTool == DrawEllipse){
        updateShape(mapToScene(event->pos()));
    }
    else if (currentMouseTool == Pencil){
        drawnPath->path().lineTo(mapToScene(event->pos()));
        update();
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void ModelView::paintEvent(QPaintEvent* event){
    QGraphicsView::paintEvent(event);

    //QPainter painter(this);
    //painter.drawPath(drawnPath->path());
}

void ModelView::keyPressEvent(QKeyEvent *event){
    if (currentMouseTool == Select && event->key() == Qt::Key_Backspace){
        for (int i = 0; i < scene()->items().size(); i++){
            QGraphicsItem *shape = scene()->items().at(i);
            if (shape->isSelected()){
                scene()->removeItem(shape);
                pushToHistory(Undo, shape);
            }
        }
    }
    QGraphicsView::keyPressEvent(event);
}

void ModelView::startShape(){
    if (currentMouseTool == DrawRectangle){
        drawnShape = scene()->addRect(QRectF(startCorner, startCorner));
        pushToHistory(Undo, drawnShape);
    }
    else{//currentMouseTool == DrawEllipse
        drawnShape = scene()->addEllipse(QRectF(startCorner, startCorner));
    }
    pushToHistory(Undo, drawnShape);
}

void ModelView::updateShape(QPointF endCorner){
    QGraphicsRectItem *rectangle = dynamic_cast<QGraphicsRectItem *>(drawnShape);
    if (rectangle){
        rectangle->setRect(QRectF(startCorner, endCorner));
        rectangle->update();
        return;
    }

    QGraphicsEllipseItem *ellipse = dynamic_cast<QGraphicsEllipseItem *>(drawnShape);
    if (ellipse){
        ellipse->setRect(QRectF(startCorner, endCorner));
        ellipse->update();
    }
}

void ModelView::deleteShape(){
    QGraphicsItem *shape;
    for (int i = 0; i < scene()->items().size(); i++){
        shape = scene()->items()[i];
        if (shape->isUnderMouse()){
            scene()->removeItem(shape);
            pushToHistory(Undo, shape);
            return;
        }
    }
}

void ModelView::pushToHistory(editType type, QGraphicsItem *shape){
    if (historyStack.size() >= maxHistorySize){
        historyStack.pop_front();
    }
    historyStack.push_back({type, shape});
}

QGraphicsItem *ModelView::popFromHistory(editType type){
    for (int i = historyStack.size() - 1; i > 0; i--){
        if (historyStack[i].first == type){
            QGraphicsItem *shape = historyStack[i].second;
            historyStack.remove(i);
            return shape;
        }
    }
    return nullptr;
}

QJsonArray ModelView::shapesToJson(){
    QJsonArray shapes;
    for (int i = 0; i < modelScene->items().size(); i++){
        QGraphicsItem *shape = scene()->items().at(i);
        QJsonObject entry;

        if (shape == nullptr){
            continue;
        }

        if (dynamic_cast<QGraphicsRectItem *>(shape)){
            entry["Shape"] = "Rectangle";
        }
        else if (dynamic_cast<QGraphicsEllipseItem *>(shape)){
            entry["Shape"] = "Ellipse";
        }
        entry["Left X"] = shape->boundingRect().topLeft().x();
        entry["Right X"] = shape->boundingRect().bottomRight().x();
        entry["Top Y"] = shape->boundingRect().topLeft().y();
        entry["Bottom Y"] = shape->boundingRect().bottomRight().y();

        shapes.push_back(entry);
    }
    return shapes;
}

void ModelView::jsonToShapes(QJsonArray shapes){
    for (const QJsonValue &entry : shapes){
        QGraphicsItem *shape = nullptr;
        QRectF boundingRect(QPointF(entry["Left X"].toDouble(), entry["Top Y"].toDouble()),
                            QPointF(entry["Right X"].toDouble(), entry["Bottom Y"].toDouble()));

        if (entry["Shape"] == "Rectangle"){
            shape = new QGraphicsRectItem(boundingRect);
        }
        else if (entry["Shape"] == "Ellipse"){
            shape = new QGraphicsEllipseItem(boundingRect);
        }

        scene()->addItem(shape);
        scene()->update(boundingRect);
    }
}

float ModelView::getZoomScale() const
{
    return zoomScale;
}
