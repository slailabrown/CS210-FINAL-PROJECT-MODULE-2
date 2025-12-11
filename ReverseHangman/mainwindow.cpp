#include "mainwindow.h"
#include "bodywidget.h"
#include "bodyviewdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // asks for phrase
    bool ok = false;
    QString phrase = QInputDialog::getText(
        this,
        "Last words...",
        "Say your prayers:",
        QLineEdit::Normal,
        "",
        &ok
        );

    if (!ok || phrase.isEmpty()) { //user closed game
        close();
        return;
    }

    engine_.setSecret(phrase.toStdString());

    phraseLabel_->setText(QString::fromStdString(engine_.maskedPhrase()));
    limbLabel_->setText(
        QString("Limbs remaining: %1").arg(GameEngine::TOTAL_LIMBS)
        );
    guessLabel_->setText("Press Space / Enter or click Next turn.");

    bodyWidget_->setLostLimbs(engine_.lostLimbs()); // initial body state
}

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;

    phraseLabel_ = new QLabel("Phrase: ");
    QFont f = phraseLabel_->font();
    f.setPointSize(20);
    phraseLabel_->setFont(f);

    limbLabel_ = new QLabel("Limbs remaining:");
    guessLabel_ = new QLabel("Ready.");

    logEdit_ = new QTextEdit;
    logEdit_->setReadOnly(true);

    // next turn button
    QPushButton *nextButton = new QPushButton("Next turn (Space)");
    connect(nextButton, &QPushButton::clicked,
            this, &MainWindow::onNextTurn);

    // view body status button
    viewBodyButton_ = new QPushButton("Look at yourself.");
    connect(viewBodyButton_, &QPushButton::clicked,
            this, &MainWindow::onViewBody);

    QHBoxLayout *topButtons = new QHBoxLayout;
    topButtons->addWidget(nextButton);
    topButtons->addWidget(viewBodyButton_);
    topButtons->addStretch();

    // body sprite
    bodyWidget_ = new BodyWidget;
    connect(bodyWidget_, &BodyWidget::limbClicked,
            this, &MainWindow::onLimbClicked);

    mainLayout->addWidget(phraseLabel_);
    mainLayout->addWidget(limbLabel_);
    mainLayout->addWidget(guessLabel_);
    mainLayout->addLayout(topButtons);
    mainLayout->addWidget(new QLabel("The game continues...(click a limb when prompted):"));
    mainLayout->addWidget(bodyWidget_, 1);   // stretch image
    mainLayout->addWidget(new QLabel("Turn log:"));
    mainLayout->addWidget(logEdit_);

    central->setLayout(mainLayout);
    setCentralWidget(central);

    setWindowTitle("Reverse Hangman");
    resize(800, 600);

    selectingLimb_ = false;
}

// allow player keyboard control for next turn
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space ||
        event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        onNextTurn();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

// ai turn
void MainWindow::onNextTurn()
{
    // ai cannot play until player picks limb
    if (selectingLimb_) {
        QMessageBox::information(
            this,
            "Choose limb to sacrifice",
            "You must sacrifice before starting the next turn."
            );
        return;
    }

    if (engine_.isGameOver()) {
        QMessageBox::information(
            this,
            "Game over",
            engine_.playerWon()
                ? "You already won."
                : "Game already finished."
            );
        return;
    }

    TurnInfo info = engine_.nextTurn(); // game engine performs guess
    lastTurn_ = info;

    if (info.hit && !info.gameOver) { //show guess outcomes, choose sacrifice
        updateUiFromGame(info, false);
        enterLimbSelectionMode();
    }
    else {
        updateUiFromGame(info, false); //miss or game end
        if (info.gameOver) {
            QString msg = info.playerWon
                              ? "You won, but at what cost..."
                              : "I win. Ha ha ha.";
            QMessageBox::information(this, "GAME OVER", msg);
        }
    }
}

// enable clicking of limbs
void MainWindow::enterLimbSelectionMode()
{
    selectingLimb_ = true;
    guessLabel_->setText("Click a limb to sacrifice.");

    std::vector<int> selectable; //determine limbs to choose from
    for (const auto &p : engine_.availableLimbs()) {
        selectable.push_back(p.first); //index of limbs
    }
    bodyWidget_->setSelectableLimbs(selectable);
}

//disable clicking limbs
void MainWindow::exitLimbSelectionMode()
{
    selectingLimb_ = false;
    bodyWidget_->setSelectableLimbs({});   //clear selection set
}

//only calls when user clicks of limb
void MainWindow::onLimbClicked(int index)
{
    if (!selectingLimb_){
        return;
    }

    bool valid = false; //is limb selectable
    for (const auto &p : engine_.availableLimbs()) {
        if (p.first == index) {
            valid = true;
            break;
        }
    }
    if (!valid){
        return;
    }

    engine_.loseLimb(index); //tells engine to lose limb
    exitLimbSelectionMode();

    updateUiFromGame(lastTurn_, true); //finish turn (ui updates, looking at yourself reflects outcomes)

    auto lost = engine_.lostLimbs(); //check loss (no limbs)
    int remaining = GameEngine::TOTAL_LIMBS;
    for (bool b : lost){
        if (b){
            --remaining;
        }
    }

    if (remaining == 0 && !engine_.isGameOver()) {
        QMessageBox::information(
            this,
            "Game over",
            "Out of limbs."
            );
    }
}

// sync widgets with engine status
void MainWindow::updateUiFromGame(const TurnInfo &info, bool afterLimbChoice)
{
    phraseLabel_->setText(
        QString::fromStdString(engine_.maskedPhrase())
        );

    auto lost = engine_.lostLimbs();
    int remaining = GameEngine::TOTAL_LIMBS;
    for (bool b : lost){
        if (b){
            --remaining;
        }
    }

    limbLabel_->setText(
        QString("Limbs remaining: %1").arg(remaining)
        );

    bodyWidget_->setLostLimbs(lost); //update body widget with blood

    QString guessText =
        QString("AI guessed %1: %2")
            .arg(QChar(info.guess))
            .arg(info.hit ? "hit" : "miss");
    guessLabel_->setText(guessText);

    QString logLine = guessText + " | "
                      + QString::fromStdString(info.message);
    if (afterLimbChoice){
        logLine += " (you clicked a limb)";
    }
    logEdit_->append(logLine);
}

//opens dialof for all limbs statuses
void MainWindow::onViewBody()
{
    auto lost = engine_.lostLimbs();
    BodyViewDialog dlg(lost, this);
    dlg.exec();
}

