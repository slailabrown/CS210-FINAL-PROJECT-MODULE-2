#ifndef BODYVIEWDIALOG_H
#define BODYVIEWDIALOG_H
#include <QDialog>
#include <vector>

class BodyViewDialog : public QDialog {
    Q_OBJECT
public:
    BodyViewDialog(const std::vector<bool>& lost, QWidget* parent=nullptr); //from game engine
};

#endif // BODYVIEWDIALOG_H
