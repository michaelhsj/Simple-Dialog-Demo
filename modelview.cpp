#include "modelview.h"

#include <QMouseEvent>
#include <QVector>
#include <tuple>
#include <QGraphicsRectItem>



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
    else{//currentMouseTool == Delete
        setCursor(Qt::PointingHandCursor);
    }
}

ModelScene *ModelView::getModelScene(){
    return modelScene;
}

void ModelView::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        if (currentMouseTool == DrawRectangle || currentMouseTool == DrawEllipse){
            startCorner = mapToScene(event->pos());
            startShape();
        }
        if (currentMouseTool == Delete){
            deleteShape();
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void ModelView::mouseMoveEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        if (currentMouseTool == DrawRectangle || currentMouseTool == DrawEllipse){
            endCorner = mapToScene(event->pos());
            updateShape();
        }
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ModelView::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        if (currentMouseTool == DrawRectangle || currentMouseTool == DrawEllipse){
            endCorner = mapToScene(event->pos());
            updateShape();
        }
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void ModelView::startShape(){
    if (currentMouseTool == DrawRectangle){
        drawnRect = scene()->addRect(QRectF(startCorner, startCorner));
        pushToHistory(Undo, drawnRect);
    }
    else{//currentMouseTool == DrawEllipse
        drawnEllipse = scene()->addEllipse(QRectF(startCorner, startCorner));
        pushToHistory(Undo, drawnEllipse);
    }
}

void ModelView::updateShape(){
    if (currentMouseTool == DrawRectangle){
        drawnRect->setRect(QRectF(startCorner, endCorner));
        scene()->update(drawnRect->rect());
        drawnRect->update();
    }
    else{//currentMouseTool == DrawEllipse
        drawnEllipse->setRect(QRectF(startCorner, endCorner));
        scene()->update(drawnEllipse->rect());
        drawnEllipse->update();
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
    for (QGraphicsItem *shape : scene()->items()){

        if (dynamic_cast<QGraphicsRectItem *>(shape)){

        }
        else{

        }
    }
}
