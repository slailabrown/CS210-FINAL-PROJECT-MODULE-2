#ifndef LIMBCHOOSERDIALOG_H
#define LIMBCHOOSERDIALOG_H
#include <QDialog>
#include <vector>
#include <utility>
#include <string>

class QListWidget;

class LimbChooserDialog : public QDialog {
    Q_OBJECT
public:
    LimbChooserDialog(const std::vector<std::pair<int,std::string>>& limbs,
                      QWidget* parent = nullptr);
    int selectedLimbIndex() const { return selectedIndex_; }

private slots:
    void onAccept();

private:
    QListWidget* list_;
    std::vector<std::pair<int,std::string>> limbs_;
    int selectedIndex_ = -1;
};

#endif // LIMBCHOOSERDIALOG_H
