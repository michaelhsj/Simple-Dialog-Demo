#include "modelscene.h"
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsView>
#include <QStack>
#include <QQueue>
#include <QJsonArray>


#ifndef MODELVIEW_H
#define MODELVIEW_H

enum mouseTool {Select, Pan, Pencil, DrawRectangle, DrawEllipse, Delete};
enum editType{Undo, Redo};
const static size_t maxHistorySize = 20;

class ModelView : public QGraphicsView
{
    Q_OBJECT

public:
    ModelView(QWidget *parent);
    ~ModelView();
    ModelScene *getModelScene() const;
    void setMouseTool(mouseTool mouseTool);
    QJsonArray shapesToJson();
    void jsonToShapes(QJsonArray shapes);
    float getZoomScale() const;
    const static QVector<float> zoomScales;

private:
    ModelScene *modelScene;
    float zoomScale;
    mouseTool currentMouseTool;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    QGraphicsItem *drawnShape;
    QPointF startCorner;
    void startShape();
    void updateShape(QPointF endCorner);
    void deleteShape();
    QVector<QPair<editType, QGraphicsItem*>> historyStack;
    void pushToHistory(editType type, QGraphicsItem *shape);
    QGraphicsItem *popFromHistory(editType type);
    void keyPressEvent(QKeyEvent *event);
    QGraphicsPathItem *drawnPath;
    void paintEvent(QPaintEvent *event);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomToExtents();
    void undo();
    void redo();
};

#endif // MODELVIEW_H

