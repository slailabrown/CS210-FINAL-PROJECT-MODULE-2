#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gameengine.h"

class QLabel;
class QPushButton;
class QTextEdit;
class BodyWidget;

//asks for phrase, runs gameloop, displays UI, handles widgets
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    //space or enter for next turn
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onNextTurn(); //ai guesses letter
    void onViewBody(); //show limb statuses
    void onLimbClicked(int index); //clicks on limbs

private:
    void setupUi();
    void updateUiFromGame(const TurnInfo& info, bool afterLimbChoice);
    void enterLimbSelectionMode();
    void exitLimbSelectionMode();

    GameEngine engine_; //main game rules

    //ui controls
    QLabel* phraseLabel_;
    QLabel* limbLabel_;
    QLabel* guessLabel_;
    QTextEdit* logEdit_;
    QPushButton* viewBodyButton_;
    BodyWidget* bodyWidget_;

    bool selectingLimb_ = false; //true when between hit and click of limb
    TurnInfo lastTurn_; //turn waiting on sacrifice
};

#endif // MAINWINDOW_H
