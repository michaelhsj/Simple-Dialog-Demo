#include "modelscene.h"
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsView>
#include <QStack>
#include <QQueue>
#include <QJsonArray>


#ifndef MODELVIEW_H
#define MODELVIEW_H

enum mouseTool {Pan, DrawRectangle, DrawEllipse, Delete};
enum editType{Undo, Redo};
const static size_t maxHistorySize = 20;

class ModelView : public QGraphicsView
{
    Q_OBJECT

public:
    ModelView(QWidget *parent);
    ~ModelView();
    ModelScene *getModelScene();
    void setMouseTool(mouseTool mouseTool);
    QJsonArray shapesToJson();


private:
    ModelScene *modelScene;
    float zoomScale;
    const static QVector<float> zoomScales;
    mouseTool currentMouseTool;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    QPointF startCorner;
    QPointF endCorner;
    QGraphicsEllipseItem *drawnEllipse;
    QGraphicsRectItem *drawnRect;
    void startShape();
    void updateShape();
    void deleteShape();
    QVector<QPair<editType, QGraphicsItem*>> historyStack;
    void pushToHistory(editType type, QGraphicsItem *shape);
    QGraphicsItem *popFromHistory(editType type);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomToExtents();
    void undo();
    void redo();
};

#endif // MODELVIEW_H

