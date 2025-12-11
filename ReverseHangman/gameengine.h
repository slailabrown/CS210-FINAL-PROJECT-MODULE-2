#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <string>
#include <vector>

//stack to record moves
class MoveStack {
    std::vector<int> data; //stores index of removed limb
public:
    void push(int x) { data.push_back(x); }
    bool empty() const { return data.empty(); }
    int top() const { return data.back(); }
    void pop() { if (!data.empty()) data.pop_back(); }
};

//queue of letters the ai will try (by order of frequency in english)
class LetterQueue {
    std::vector<char> data; //candidates
    size_t frontIndex = 0;  //current front
public:
    LetterQueue(); //fills queue
    bool empty() const { return frontIndex >= data.size(); }
    char front() const { return data[frontIndex]; }
    void pop() { if (!empty()) ++frontIndex; }
};

//singly linked list of limbs
struct LimbNode {
    std::string name; //name of limb
    int index; //index 0-11
    LimbNode* next;
    LimbNode(const std::string& n, int idx)
        : name(n), index(idx), next(nullptr) {}
};

class LimbList {
    LimbNode* head = nullptr;
public:
    LimbList(); //build list
    ~LimbList();  //free nodes
    bool empty() const { return head == nullptr; }
    std::vector<std::pair<int, std::string>> toVector() const; //returns vector of pairs for GUI (index, name)
    void removeByIndex(int idx); //remove limb by index after lost
};

//tree of body parts (body root with 12 children)
struct TreeNode {
    std::string name;
    int index;          //0-11 for limbs, -1 for root
    bool lost;          //if limb is gone
    std::vector<TreeNode*> children;
    TreeNode(const std::string& n, int idx)
        : name(n), index(idx), lost(false) {}
};

class BodyTree {
    TreeNode* root;
    TreeNode* findByIndex(TreeNode* node, int idx);
    void collectLost(TreeNode* node, std::vector<bool>& out) const;
    void destroy(TreeNode* node);
public:
    BodyTree();
    ~BodyTree();
    void markLost(int limbIndex);  //mark as lost
    void getLost(std::vector<bool>& lost) const; //fills with lost limbs
};

//graph for anatomical connections
class BodyGraph {
    std::vector<std::vector<int>> adj; //list of ajacent limbs
public:
    BodyGraph();
    std::string neighborsOf(int limbIndex) const; //returns neighbor indixes as string separated by comma (for debug)
};

//core game engine
struct TurnInfo {
    char guess = '?'; //letter guessed for turn
    bool hit = false; //hit or not
    bool gameOver = false; //game ending move?
    bool playerWon = false; //flag for win
    std::string message; //text for UI
};

class GameEngine {
public:
    static constexpr int TOTAL_LIMBS = 12;

    GameEngine();

    void setSecret(const std::string& phrase); //set/normalize phrase
    std::string maskedPhrase() const; //return phrase with "_" for hidden letters

    int limbsRemaining() const { return limbsRemaining_; }
    int maxGuesses() const { return MAX_GUESSES; }
    int guessesUsed() const { return guessesUsed_; }
    bool isGameOver() const { return gameOver_; }
    bool playerWon() const { return playerWon_; }

    TurnInfo nextTurn();          // one AI guess
    void loseLimb(int limbIndex); // user chooses which limb to sacrifice

    std::vector<bool> lostLimbs() const; //true if limb i is lost
    std::string limbName(int index) const; //limb index for log/UI
    std::vector<std::pair<int, std::string>> availableLimbs() const; //list of limbs for menu (readable)

private:
    std::string secret_; //secret phrase in uppercase (letter and spaces)
    std::vector<bool> revealed_; //reveal flag per letter
    bool gameOver_ = false;
    bool playerWon_ = false;

    static const int MAX_GUESSES = 18; //number of tries for AI
    int guessesUsed_ = 0;
    int limbsRemaining_ = TOTAL_LIMBS;

    LetterQueue guessQueue_; //order of letters to guess
    bool guessedHash_[26]; //hash set (letters used already)

    LimbList limbList_; //list limbs to lose
    MoveStack moveStack_; //stack of lost limbs
    BodyTree bodyTree_; //tree of body parts
    BodyGraph bodyGraph_; //graph of body anatomy (connection of limbs)

    bool allRevealed() const; //is phrase fully guessed?
    char pickNextLetter(); //pop next letter from queue
};

#endif // GAMEENGINE_H

