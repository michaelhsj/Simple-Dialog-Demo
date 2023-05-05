#include "modelscene.h"

ModelScene::ModelScene()
    : QGraphicsScene()
    , modelRect()
{
}

ModelScene::~ModelScene()
{
}

void ModelScene::drawModel(QString model, float p1, float p2){
    float scalingFactor = fmax(300 / fmax(p1, p2), 1);
    p1 *= scalingFactor;
    p2 *= scalingFactor;

    //rescale p1 and p2?

    QColor borderColor = QColor(106,151,201);
    QColor shadedColor = QColor(114,158,206);
    QColor litColor = QColor(164,194,225);

    QPoint TopLeft;

    float angleFactor = 0.25;
    if (model == "U"){
        // p1 = cylinder radius
        // p2 = cylinder height
        // draw upright cylinder
        addEllipse(-p1, p2/2 - p1 * angleFactor, 2*p1, 2*p1*angleFactor, QPen(borderColor), QBrush(shadedColor));
        addRect(-p1, -p2/2, 2 * p1, p2, QPen(borderColor), QBrush(shadedColor));
        addEllipse(-p1, -p2/2 - p1 * angleFactor, 2*p1, 2*p1*angleFactor, QPen(borderColor), QBrush(litColor));

        TopLeft.setX(-p1);
        TopLeft.setY(-p2/2 - p1);
    }
    else if (model == "B"){
        // p1 = cylinder radius
        // p2 = cylinder width
        //draw cylinder on its side
        addEllipse(p2/2 -p1*angleFactor, -p1, 2*p1*angleFactor, 2*p1, QPen(borderColor), QBrush(shadedColor));
        addRect(-p2/2, -p1, p2, 2*p1, QPen(borderColor), QBrush(shadedColor));
        addEllipse(-p2/2 -p1*angleFactor, -p1, 2*p1*angleFactor, 2*p1, QPen(borderColor), QBrush(litColor));

        TopLeft.setX(-p2/2 -p1*angleFactor);
        TopLeft.setY(-p1);
    }
    else if (model == "T"){
        // p1 = radius of cavity
        // p2 = width of square
        // draw box with cavity
        addRect(-p2/2, -p2/2, p2, p2, QPen(borderColor), QBrush(shadedColor));
        addEllipse(-p1/2, -p1/2, p1, p1, QPen(borderColor), QBrush(QColor("white")));

        TopLeft.setX(-fmax(p1, p2)/2);
        TopLeft.setY(-fmax(p1, p2)/2);
    }

    // We will always place the center of the object at (0, 0), so the bottom right of the shape will
    // be TopLeft reflected.
    modelRect.setTopLeft(TopLeft);
    modelRect.setBottomRight(-TopLeft);

}

QRectF &ModelScene::getModelRect(){
    return modelRect;
}
