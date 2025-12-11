#include "bodywidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QDebug>
#include <array>

// size of original PNG
static const int BODY_IMG_WIDTH  = 1280;
static const int BODY_IMG_HEIGHT = 720;

//rectangle bases for each limb
static const std::array<QRect, BodyWidget::LIMB_COUNT> BASE_RECTS = {
    QRect(457,  32,  83,  99),  // 0 Head
    QRect(414, 181, 169, 221),  // 1 Torso
    QRect(377,  74,  45, 117),  // 2 Left bicep
    QRect(352, 212,  32,  99),  // 3 Left forearm
    QRect(385, 279,  41,  74),  // 4 Left hand
    QRect(438, 418,  49, 147),  // 5 Left thigh
    QRect(373, 464,  40, 221),  // 6 Left calf
    QRect(576,  74,  45, 118),  // 7 Right bicep
    QRect(615, 219,  33,  99),  // 8 Right forearm
    QRect(571, 279,  41,  74),  // 9 Right hand
    QRect(509, 418,  50, 147),  // 10 Right thigh
    QRect(584, 464,  40, 221)   // 11 Right calf
};

BodyWidget::BodyWidget(QWidget *parent)
    : QWidget(parent)
{
    bodyImage_.load(":/dumb/dumb/fullbody.png"); // Load body and blood images from resources
    if (bodyImage_.isNull()) {
        qWarning() << "BodyWidget: can't fucking load :/dumb/dumb/fullbody.png";
    }

    bloodOverlay_.load(":/dumb/dumb/blood.png");
    if (bloodOverlay_.isNull()) {
        qWarning() << "BodyWidget: can't fucking load :/dumb/dumb/blood.png";
    }
    lost_.fill(false);
    selectable_.fill(false);
    setMinimumSize(400, 225); // minimum aspect
    updateHitRects();
}

void BodyWidget::setLostLimbs(const std::vector<bool>& lost)
{
    for (int i = 0; i < LIMB_COUNT; ++i) {  //copy lost flags into array (fixed)
        if (i < (int)lost.size()){
            lost_[i] = lost[i];
        }
        else{
            lost_[i] = false;
        }
    }
    update();
}

void BodyWidget::setSelectableLimbs(const std::vector<int>& indices)
{
    selectable_.fill(false);
    for (int idx : indices) {
        if (idx >= 0 && idx < LIMB_COUNT){
            selectable_[idx] = true;
        }
    }
    update();
}

//for resizing, recomputes hitRects_
void BodyWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateHitRects();
}

//convert base rectangles into hiboxes, preserves aspect ratio
void BodyWidget::updateHitRects()
{
    if (BODY_IMG_WIDTH == 0 || BODY_IMG_HEIGHT == 0){
        return;
    }

    double sx = double(width())  / BODY_IMG_WIDTH;
    double sy = double(height()) / BODY_IMG_HEIGHT;
    for (int i = 0; i < LIMB_COUNT; ++i) {
        const QRect &src = BASE_RECTS[i];
        int x = int(src.x()      * sx);
        int y = int(src.y()      * sy);
        int w = int(src.width()  * sx);
        int h = int(src.height() * sy);
        hitRects_[i] = QRect(x, y, w, h);
    }
}

void BodyWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // fill background
    p.fillRect(rect(), Qt::black);

    // draw sprite
    if (!bodyImage_.isNull()) {
        p.drawPixmap(rect(), bodyImage_);
    }

    // draw overlay depending on state
    for (int i = 0; i < LIMB_COUNT; ++i) {
        const QRect &r = hitRects_[i];
        if (!r.isValid()){
            continue;
        }

        if (lost_[i]) {
            // draw splatter over lost limbs
            if (!bloodOverlay_.isNull()) {
                QRect bleedRect = r.adjusted(-5, -5, 5, 5); //slghty bigger rectangle so blood "spills over"

                p.save();
                p.setOpacity(0.85);  // slight transparency
                p.drawPixmap(bleedRect, bloodOverlay_);
                p.restore();
            }
        } else if (selectable_[i]) {
            p.setPen(QPen(QColor(0, 200, 255), 3)); //selectable == blue outline
            p.setBrush(Qt::NoBrush);
            p.drawRect(r.adjusted(1, 1, -1, -1));
        }
    }
}

void BodyWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    for (int i = 0; i < LIMB_COUNT; ++i) { //find clicked limb
        if (hitRects_[i].contains(pos) && selectable_[i] && !lost_[i]) {
            emit limbClicked(i);
            break;
        }
    }
    QWidget::mousePressEvent(event);
}

