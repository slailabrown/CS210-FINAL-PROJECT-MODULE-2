#include "bodyviewdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include "gameengine.h"

BodyViewDialog::BodyViewDialog(const std::vector<bool>& lost, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Current body status");
    QVBoxLayout* layout = new QVBoxLayout(this);

    static const char* names[GameEngine::TOTAL_LIMBS] = {
        "Head","Torso",
        "Left bicep","Left forearm","Left hand",
        "Left thigh","Left calf",
        "Right bicep","Right forearm","Right hand",
        "Right thigh","Right calf"
    };

    for (int i = 0; i < GameEngine::TOTAL_LIMBS; ++i) {
        bool isLost = (i < (int)lost.size()) ? lost[i] : false;
        QString text = QString("%1: %2")
                           .arg(names[i])
                           .arg(isLost ? "CHOPPED" : "OK");
        layout->addWidget(new QLabel(text));
    }
}
