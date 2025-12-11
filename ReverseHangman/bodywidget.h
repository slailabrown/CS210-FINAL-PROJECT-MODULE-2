#ifndef BODYWIDGET_H
#define BODYWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QRect>
#include <array>
#include <vector>

//draws sprite, creates hitboxes, creates blood overlays
class BodyWidget : public QWidget {
    Q_OBJECT
public:
    explicit BodyWidget(QWidget *parent = nullptr);
    void setLostLimbs(const std::vector<bool>& lost); // update lost limbs (for blood splatters)
    void setSelectableLimbs(const std::vector<int>& indices); // clickable limbs

signals:
    // flag for when selectable limb clicked
    void limbClicked(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public:
    static const int LIMB_COUNT = 12;

private:
    void updateHitRects(); // computes hitRects_ according to widget size

    QPixmap bodyImage_; //sprite for body
    QPixmap bloodOverlay_; //blood splatter sprite

    //limb statuses
    std::array<bool, LIMB_COUNT> lost_; //limb gone == true
    std::array<bool, LIMB_COUNT> selectable_; //limb clickable == true

    std::array<QRect, LIMB_COUNT> hitRects_; //hitbox rectangles for limbs
};

#endif // BODYWIDGET_H

