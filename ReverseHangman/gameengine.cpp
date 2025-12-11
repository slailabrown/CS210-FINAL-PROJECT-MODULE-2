#include "gameengine.h"
#include <cctype>
#include <sstream>

//fill queue with letters (by frequency in english)
LetterQueue::LetterQueue() {
    std::string order = "ETAOINSHRDLUCMWFGYPBVKJXQZ";
    for (char c : order){
        data.push_back(c);
    }
}

//name and index of each limb
LimbList::LimbList() {
    // indices for limbs
    const char* names[GameEngine::TOTAL_LIMBS] = {
        "Head",          // 0
        "Torso",         // 1
        "Left bicep",    // 2
        "Left forearm",  // 3
        "Left hand",     // 4
        "Left thigh",    // 5
        "Left calf",     // 6
        "Right bicep",   // 7
        "Right forearm", // 8
        "Right hand",    // 9
        "Right thigh",   // 10
        "Right calf"     // 11
    };
    //node creation for limb list
    head = nullptr;
    LimbNode* tail = nullptr;
    for (int i = 0; i < GameEngine::TOTAL_LIMBS; ++i) {
        LimbNode* node = new LimbNode(names[i], i);
        if (!head) {
            head = node;
        } else {
            tail->next = node;
        }
        tail = node;
    }
}

//free nodes
LimbList::~LimbList() {
    LimbNode* cur = head;
    while (cur) {
        LimbNode* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
}

std::vector<std::pair<int, std::string>> LimbList::toVector() const {
    std::vector<std::pair<int, std::string>> out;
    LimbNode* cur = head;
    while (cur) {
        out.push_back({cur->index, cur->name});
        cur = cur->next;
    }
    return out;
}

void LimbList::removeByIndex(int idx) {
    LimbNode* cur = head;
    LimbNode* prev = nullptr;
    while (cur) {
        if (cur->index == idx) {
            if (prev) prev->next = cur->next; //unlink node
            else head = cur->next;
            delete cur;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

BodyTree::BodyTree() {
    root = new TreeNode("Body", -1); //root node for full body
    const char* names[GameEngine::TOTAL_LIMBS] = {
        "Head","Torso",
        "Left bicep","Left forearm","Left hand",
        "Left thigh","Left calf",
        "Right bicep","Right forearm","Right hand",
        "Right thigh","Right calf"
    };
    for (int i = 0; i < GameEngine::TOTAL_LIMBS; ++i) {
        root->children.push_back(new TreeNode(names[i], i));
    }
}

BodyTree::~BodyTree() {
    destroy(root);
}

//delete tree recursively
void BodyTree::destroy(TreeNode* node) {
    if (!node){
        return;
    }
    for (TreeNode* ch : node->children){
        destroy(ch);
    }
    delete node;
}

//search tree for limb by index
TreeNode* BodyTree::findByIndex(TreeNode* node, int idx) {
    if (!node){
        return nullptr;
    }
    if (node->index == idx){
        return node;
    }
    for (TreeNode* ch : node->children) {
        TreeNode* res = findByIndex(ch, idx);
        if (res){
            return res;
        }
    }
    return nullptr;
}

//denoting loss of limbs
void BodyTree::markLost(int limbIndex) {
    TreeNode* node = findByIndex(root, limbIndex);
    if (node){
        node->lost = true;
    }
}

void BodyTree::collectLost(TreeNode* node, std::vector<bool>& out) const {
    if (!node){
        return;
    }
    if (node->index >= 0 && node->index < (int)out.size()){
        out[node->index] = node->lost;
    }
    for (TreeNode* ch : node->children){
        collectLost(ch, out);
    }
}

void BodyTree::getLost(std::vector<bool>& lost) const {
    lost.assign(GameEngine::TOTAL_LIMBS, false);
    collectLost(root, lost);
}

BodyGraph::BodyGraph() {
    int n = GameEngine::TOTAL_LIMBS;
    adj.assign(n, {});

    auto addEdge = [&](int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    };

    //anatomical connection of limbs
    addEdge(0, 1);  // head <-> torso
    addEdge(1, 2);  // torso <-> left bicep
    addEdge(2, 3);  // left bicep <-> left forearm
    addEdge(3, 4);  // left forearm <-> left hand

    addEdge(1, 7);  // torso <-> right bicep
    addEdge(7, 8);  // right bicep <-> right forearm
    addEdge(8, 9);  // right forearm <-> right hand

    addEdge(1, 5);  // torso <-> left thigh
    addEdge(5, 6);  // left thigh <-> left calf

    addEdge(1, 10); // torso <-> right thigh
    addEdge(10, 11); // right thigh <-> right calf
}

//
std::string BodyGraph::neighborsOf(int limbIndex) const {
    if (limbIndex < 0 || limbIndex >= (int)adj.size()){
        return {};
    }
    std::ostringstream oss;
    bool first = true;
    for (int v : adj[limbIndex]) {
        if (!first){
            oss << ", ";
        }
        oss << v; // index for debug
        first = false;
    }
    return oss.str();
}

// normalize to letters and spaces only + convert to uppercase
static std::string normalize(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (std::isalpha((unsigned char)c) || c==' '){
            r.push_back(std::toupper((unsigned char)c));
        }
    }
    return r;
}

// game engine core
GameEngine::GameEngine() {
    for (bool &b : guessedHash_){
        b = false;
    }
}

void GameEngine::setSecret(const std::string& phrase) {
    secret_ = normalize(phrase);
    revealed_.assign(secret_.size(), false);
    guessesUsed_ = 0;
    gameOver_ = false;
    playerWon_ = false;
    limbsRemaining_ = TOTAL_LIMBS;

    guessQueue_ = LetterQueue(); // reset ai guess queue and guessed hash set
    for (bool &b : guessedHash_){
        b = false;
    }
}

// true when all letters are guessed correctly
bool GameEngine::allRevealed() const {
    for (size_t i = 0; i < secret_.size(); ++i) {
        if (secret_[i] != ' ' && !revealed_[i]){
            return false;
        }
    }
    return true;
}

std::string GameEngine::maskedPhrase() const {
    std::string out;
    for (size_t i = 0; i < secret_.size(); ++i) {
        if (secret_[i] == ' '){
            out.push_back(' ');
        }
        else if (revealed_[i]){
            out.push_back(secret_[i]);
        }
        else{
            out.push_back('_');
        }
    }
    return out;
}

// dequeue next unused letter from queue
char GameEngine::pickNextLetter() {
    while (!guessQueue_.empty()) {
        char c = guessQueue_.front();
        guessQueue_.pop();
        int idx = c - 'A';
        if (!guessedHash_[idx]) {
            guessedHash_[idx] = true;
            return c;
        }
    }
    return 0; // all letters used
}

// one ai guess and game state update, mainwindow can then force sacrifice if hit
TurnInfo GameEngine::nextTurn() {
    TurnInfo info;
    if (gameOver_) {
        info.gameOver = true;
        return info;
    }

    char g = pickNextLetter();
    if (g == 0) { // ai out of guesses (win)
        gameOver_ = true;
        playerWon_ = true;
        info.gameOver = true;
        info.playerWon = true;
        info.message = "I'll get you next time.";
        return info;
    }

    ++guessesUsed_;
    info.guess = g;

    bool hit = false;
    for (size_t i = 0; i < secret_.size(); ++i) { //reveal all occurences of correctly guessed letter
        if (secret_[i] == g) {
            revealed_[i] = true;
            hit = true;
        }
    }
    info.hit = hit;

    if (hit){
        info.message = "Hit, choose a limb to sacrifice.";
    }
    else{
        info.message = "You got lucky this time, no hit.";
    }

    //check for win after guess
    if (allRevealed()) { //if fully guessed
        gameOver_ = true;
        playerWon_ = false;
        info.gameOver = true;
        info.playerWon = false;
        info.message += "I guessed it.";
    } else if (guessesUsed_ >= MAX_GUESSES || limbsRemaining_ <= 0) { //no limbs left or ai out of guesses
        gameOver_ = true;
        playerWon_ = true;
        info.gameOver = true;
        info.playerWon = true;
        info.message += "You survived...barely.";
    }

    return info;
}

//after limb sacrifice
void GameEngine::loseLimb(int limbIndex) {
    if (limbIndex < 0 || limbIndex >= TOTAL_LIMBS){
        return;
    }
    bodyTree_.markLost(limbIndex); //tell tree to mark limb as lost
    limbList_.removeByIndex(limbIndex); //remove from list of availiable sacrifices
    moveStack_.push(limbIndex); //record lost limb in stack
    --limbsRemaining_;
}

std::vector<bool> GameEngine::lostLimbs() const {
    std::vector<bool> lost;
    bodyTree_.getLost(lost);
    return lost;
}

std::string GameEngine::limbName(int index) const {
    static const char* names[TOTAL_LIMBS] = {
        "Head","Torso",
        "Left bicep","Left forearm","Left hand",
        "Left thigh","Left calf",
        "Right bicep","Right forearm","Right hand",
        "Right thigh","Right calf"
    };
    if (index < 0 || index >= TOTAL_LIMBS) return "";
    return names[index];
}

std::vector<std::pair<int,std::string>> GameEngine::availableLimbs() const {
    return limbList_.toVector();
}
