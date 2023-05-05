#ifndef MODELSCENE_H
#define MODELSCENE_H

#include <QGraphicsScene>

class ModelScene : public QGraphicsScene
{
    Q_OBJECT

public:
    ModelScene();
    ~ModelScene();
    void drawModel(QString model, float p1, float p2);
    QRectF &getModelRect();

private:
    QRectF modelRect;
};

#endif // MODELSCENE_H
