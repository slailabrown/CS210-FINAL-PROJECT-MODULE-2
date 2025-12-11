#include "limbchooserdialog.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>

LimbChooserDialog::LimbChooserDialog(
    const std::vector<std::pair<int,std::string>>& limbs,
    QWidget* parent)
    : QDialog(parent), limbs_(limbs)
{
    setWindowTitle("Choose a limb to lose");

    QVBoxLayout* layout = new QVBoxLayout(this);
    list_ = new QListWidget;

    for (const auto& p : limbs_) {
        list_->addItem(QString::fromStdString(p.second));
    }

    layout->addWidget(list_);

    QHBoxLayout* buttons = new QHBoxLayout;
    QPushButton* ok = new QPushButton("OK");
    QPushButton* cancel = new QPushButton("Cancel");
    connect(ok, &QPushButton::clicked,
            this, &LimbChooserDialog::onAccept);
    connect(cancel, &QPushButton::clicked,
            this, &LimbChooserDialog::reject);
    buttons->addWidget(ok);
    buttons->addWidget(cancel);
    layout->addLayout(buttons);
}

void LimbChooserDialog::onAccept() {
    int row = list_->currentRow();
    if (row >= 0 && row < (int)limbs_.size()) {
        selectedIndex_ = limbs_[row].first;
        accept();
    }
}
