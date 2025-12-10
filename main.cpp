// main.cpp
#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QMessageBox>
#include <QListWidget>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;

// ============ DATA STRUCTURES ============

// 1. TREE - Skill/Ability Tree
struct SkillNode {
    QString name;
    int mpCost;
    int damage;
    QString type; // "attack", "heal", "buff"
    bool unlocked;
    SkillNode* left;
    SkillNode* right;
    
    SkillNode(QString n, int mp, int dmg, QString t) 
        : name(n), mpCost(mp), damage(dmg), type(t), unlocked(false), 
          left(nullptr), right(nullptr) {}
};

class AbilityTree {
public:
    SkillNode* root;
    
    AbilityTree() {
        root = new SkillNode("Attack", 0, 20, "attack");
        root->unlocked = true;
        root->left = new SkillNode("Fire", 10, 35, "attack");
        root->right = new SkillNode("Heal", 8, 30, "heal");
        root->left->left = new SkillNode("Firaga", 25, 60, "attack");
        root->left->right = new SkillNode("Thunder", 15, 45, "attack");
        root->right->left = new SkillNode("Cura", 20, 50, "heal");
        root->right->right = new SkillNode("Regen", 12, 20, "buff");
    }
    
    void getUnlockedSkills(SkillNode* node, vector<SkillNode*>& skills) {
        if (!node) return;
        if (node->unlocked) skills.push_back(node);
        getUnlockedSkills(node->left, skills);
        getUnlockedSkills(node->right, skills);
    }
};

// 2. Character Stats
struct Character {
    QString name;
    int hp;
    int maxHp;
    int mp;
    int maxMp;
    int level;
    int exp;
    int attack;
    int defense;
    bool isPlayer;
    vector<QString> statusEffects; // LIST
    map<QString, int> inventory; // MAP
    
    Character(QString n, int h, int m, int atk, int def, bool player = true)
        : name(n), hp(h), maxHp(h), mp(m), maxMp(m), level(1), exp(0),
          attack(atk), defense(def), isPlayer(player) {}
    
    void takeDamage(int dmg) {
        int actualDmg = max(1, dmg - defense);
        hp = max(0, hp - actualDmg);
    }
    
    void heal(int amount) {
        hp = min(maxHp, hp + amount);
    }
    
    void addExp(int amount) {
        exp += amount;
        if (exp >= level * 100) {
            levelUp();
        }
    }
    
    void levelUp() {
        level++;
        exp = 0;
        maxHp += 20;
        maxMp += 10;
        attack += 5;
        defense += 3;
        hp = maxHp;
        mp = maxMp;
    }
};

// 3. GRAPH - World Map connections
class WorldGraph {
public:
    unordered_map<QString, vector<QString>> connections; // HASHMAP
    unordered_map<QString, QString> locationDesc; // HASHMAP
    unordered_map<QString, int> enemyLevel; // HASHMAP
    
    WorldGraph() {
        // Define locations
        connections["Starting Village"] = {"Forest Path", "Old Mine"};
        connections["Forest Path"] = {"Starting Village", "Dark Woods", "Crystal Cave"};
        connections["Dark Woods"] = {"Forest Path", "Ancient Ruins"};
        connections["Crystal Cave"] = {"Forest Path", "Mountain Peak"};
        connections["Old Mine"] = {"Starting Village", "Ancient Ruins"};
        connections["Ancient Ruins"] = {"Dark Woods", "Old Mine", "Final Castle"};
        connections["Mountain Peak"] = {"Crystal Cave", "Final Castle"};
        connections["Final Castle"] = {"Ancient Ruins", "Mountain Peak"};
        
        locationDesc["Starting Village"] = "A peaceful village where your journey begins.";
        locationDesc["Forest Path"] = "A winding path through dense trees.";
        locationDesc["Dark Woods"] = "Dangerous woods filled with monsters.";
        locationDesc["Crystal Cave"] = "A mystical cave with glowing crystals.";
        locationDesc["Old Mine"] = "An abandoned mine with treasures.";
        locationDesc["Ancient Ruins"] = "Crumbling ruins of an ancient civilization.";
        locationDesc["Mountain Peak"] = "The highest point with a breathtaking view.";
        locationDesc["Final Castle"] = "The dark lord's fortress.";
        
        enemyLevel["Starting Village"] = 1;
        enemyLevel["Forest Path"] = 2;
        enemyLevel["Dark Woods"] = 4;
        enemyLevel["Crystal Cave"] = 3;
        enemyLevel["Old Mine"] = 3;
        enemyLevel["Ancient Ruins"] = 5;
        enemyLevel["Mountain Peak"] = 5;
        enemyLevel["Final Castle"] = 7;
    }
};

// 4. PRIORITY QUEUE - Turn-based battle system
struct BattleTurn {
    bool isPlayer;
    int speed;
    
    bool operator<(const BattleTurn& other) const {
        return speed < other.speed; // Higher speed goes first
    }
};

// 5. Battle Log using QUEUE
class BattleLog {
public:
    queue<QString> messages; // QUEUE
    
    void addMessage(QString msg) {
        messages.push(msg);
        if (messages.size() > 100) {
            messages.pop();
        }
    }
    
    QString getRecent() {
        QString result;
        queue<QString> temp = messages;
        int count = 0;
        while (!temp.empty() && count < 10) {
            result += temp.front() + "\n";
            temp.pop();
            count++;
        }
        return result;
    }
};

// ============ MAIN GAME WINDOW ============
class FantasyRPG : public QMainWindow {
    Q_OBJECT
    
private:
    // Data structures
    Character* player;
    Character* currentEnemy;
    AbilityTree abilityTree;
    WorldGraph worldMap;
    BattleLog battleLog;
    
    QString currentLocation;
    set<QString> visitedLocations; // SET
    stack<QString> locationHistory; // STACK
    
    // UI Elements
    QWidget* centralWidget;
    QLabel* locationLabel;
    QLabel* playerNameLabel;
    QLabel* enemyNameLabel;
    QProgressBar* playerHPBar;
    QProgressBar* playerMPBar;
    QProgressBar* enemyHPBar;
    QLabel* playerStatsLabel;
    QLabel* enemyStatsLabel;
    QTextEdit* battleLogText;
    QListWidget* abilityList;
    QListWidget* locationList;
    QPushButton* attackBtn;
    QPushButton* defendBtn;
    QPushButton* itemBtn;
    QPushButton* skillTreeBtn;
    QPushButton* backtrackBtn;
    QLabel* dataStructLabel;
    
    bool inBattle;
    QTimer* battleTimer;
    
public:
    FantasyRPG(QWidget *parent = nullptr) : QMainWindow(parent) {
        srand(time(0));
        
        setWindowTitle("Fantasy Quest - Final Fantasy Style RPG");
        setMinimumSize(1000, 700);
        
        // Initialize game data
        player = new Character("Hero", 100, 50, 20, 10, true);
        player->inventory["Potion"] = 3;
        player->inventory["Ether"] = 2;
        
        currentLocation = "Starting Village";
        visitedLocations.insert(currentLocation);
        locationHistory.push(currentLocation);
        
        currentEnemy = nullptr;
        inBattle = false;
        
        setupUI();
        updateUI();
        
        battleTimer = new QTimer(this);
        connect(battleTimer, &QTimer::timeout, this, &FantasyRPG::enemyTurn);
    }
    
    void setupUI() {
        centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
        
        // Left panel - Battle area
        QVBoxLayout* leftLayout = new QVBoxLayout();
        
        locationLabel = new QLabel("Location: Starting Village");
        locationLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
        leftLayout->addWidget(locationLabel);
        
        // Player status
        QGroupBox* playerGroup = new QGroupBox("Hero Status");
        QVBoxLayout* playerLayout = new QVBoxLayout();
        
        playerNameLabel = new QLabel("Hero - Level 1");
        playerNameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        playerLayout->addWidget(playerNameLabel);
        
        QLabel* hpLabel = new QLabel("HP:");
        playerLayout->addWidget(hpLabel);
        playerHPBar = new QProgressBar();
        playerHPBar->setStyleSheet("QProgressBar::chunk { background-color: #e74c3c; }");
        playerLayout->addWidget(playerHPBar);
        
        QLabel* mpLabel = new QLabel("MP:");
        playerLayout->addWidget(mpLabel);
        playerMPBar = new QProgressBar();
        playerMPBar->setStyleSheet("QProgressBar::chunk { background-color: #3498db; }");
        playerLayout->addWidget(playerMPBar);
        
        playerStatsLabel = new QLabel();
        playerLayout->addWidget(playerStatsLabel);
        
        playerGroup->setLayout(playerLayout);
        leftLayout->addWidget(playerGroup);
        
        // Enemy status
        QGroupBox* enemyGroup = new QGroupBox("Enemy Status");
        QVBoxLayout* enemyLayout = new QVBoxLayout();
        
        enemyNameLabel = new QLabel("No enemy");
        enemyNameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        enemyLayout->addWidget(enemyNameLabel);
        
        enemyHPBar = new QProgressBar();
        enemyHPBar->setStyleSheet("QProgressBar::chunk { background-color: #e67e22; }");
        enemyLayout->addWidget(enemyHPBar);
        
        enemyStatsLabel = new QLabel();
        enemyLayout->addWidget(enemyStatsLabel);
        
        enemyGroup->setLayout(enemyLayout);
        leftLayout->addWidget(enemyGroup);
        
        // Battle log
        QLabel* logLabel = new QLabel("Battle Log:");
        leftLayout->addWidget(logLabel);
        
        battleLogText = new QTextEdit();
        battleLogText->setReadOnly(true);
        battleLogText->setMaximumHeight(150);
        leftLayout->addWidget(battleLogText);
        
        mainLayout->addLayout(leftLayout, 2);
        
        // Right panel - Actions and abilities
        QVBoxLayout* rightLayout = new QVBoxLayout();
        
        QLabel* actionsLabel = new QLabel("Actions:");
        actionsLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        rightLayout->addWidget(actionsLabel);
        
        attackBtn = new QPushButton("⚔️ Attack");
        attackBtn->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; padding: 10px; font-size: 14px; } QPushButton:hover { background-color: #c0392b; }");
        connect(attackBtn, &QPushButton::clicked, this, &FantasyRPG::onAttack);
        rightLayout->addWidget(attackBtn);
        
        defendBtn = new QPushButton("🛡️ Defend");
        defendBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 10px; font-size: 14px; } QPushButton:hover { background-color: #2980b9; }");
        connect(defendBtn, &QPushButton::clicked, this, &FantasyRPG::onDefend);
        rightLayout->addWidget(defendBtn);
        
        itemBtn = new QPushButton("💊 Use Item");
        itemBtn->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; padding: 10px; font-size: 14px; } QPushButton:hover { background-color: #27ae60; }");
        connect(itemBtn, &QPushButton::clicked, this, &FantasyRPG::onUseItem);
        rightLayout->addWidget(itemBtn);
        
        QLabel* skillsLabel = new QLabel("Abilities:");
        skillsLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
        rightLayout->addWidget(skillsLabel);
        
        abilityList = new QListWidget();
        connect(abilityList, &QListWidget::itemDoubleClicked, this, &FantasyRPG::onUseAbility);
        rightLayout->addWidget(abilityList);
        
        skillTreeBtn = new QPushButton("🌳 View Skill Tree");
        skillTreeBtn->setStyleSheet("QPushButton { background-color: #9b59b6; color: white; padding: 8px; } QPushButton:hover { background-color: #8e44ad; }");
        connect(skillTreeBtn, &QPushButton::clicked, this, &FantasyRPG::onShowSkillTree);
        rightLayout->addWidget(skillTreeBtn);
        
        QLabel* travelLabel = new QLabel("Travel:");
        travelLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
        rightLayout->addWidget(travelLabel);
        
        locationList = new QListWidget();
        connect(locationList, &QListWidget::itemDoubleClicked, this, &FantasyRPG::onTravel);
        rightLayout->addWidget(locationList);
        
        backtrackBtn = new QPushButton("↶ Backtrack");
        backtrackBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; padding: 8px; } QPushButton:hover { background-color: #7f8c8d; }");
        connect(backtrackBtn, &QPushButton::clicked, this, &FantasyRPG::onBacktrack);
        rightLayout->addWidget(backtrackBtn);
        
        // Data structures label
        dataStructLabel = new QLabel();
        dataStructLabel->setStyleSheet("font-size: 10px; color: #7f8c8d; margin-top: 10px;");
        dataStructLabel->setWordWrap(true);
        rightLayout->addWidget(dataStructLabel);
        
        mainLayout->addLayout(rightLayout, 1);
        
        updateAbilityList();
        updateLocationList();
        updateDataStructuresInfo();
    }
    
    void updateUI() {
        // Update player info
        playerNameLabel->setText(QString("%1 - Level %2 (EXP: %3/%4)")
            .arg(player->name).arg(player->level).arg(player->exp).arg(player->level * 100));
        
        playerHPBar->setMaximum(player->maxHp);
        playerHPBar->setValue(player->hp);
        playerHPBar->setFormat(QString("%1/%2").arg(player->hp).arg(player->maxHp));
        
        playerMPBar->setMaximum(player->maxMp);
        playerMPBar->setValue(player->mp);
        playerMPBar->setFormat(QString("%1/%2").arg(player->mp).arg(player->maxMp));
        
        playerStatsLabel->setText(QString("ATK: %1 | DEF: %2")
            .arg(player->attack).arg(player->defense));
        
        // Update enemy info
        if (currentEnemy) {
            enemyNameLabel->setText(QString("%1 - Level %2")
                .arg(currentEnemy->name).arg(currentEnemy->level));
            enemyHPBar->setMaximum(currentEnemy->maxHp);
            enemyHPBar->setValue(currentEnemy->hp);
            enemyHPBar->setFormat(QString("%1/%2").arg(currentEnemy->hp).arg(currentEnemy->maxHp));
            enemyStatsLabel->setText(QString("ATK: %1 | DEF: %2")
                .arg(currentEnemy->attack).arg(currentEnemy->defense));
        } else {
            enemyNameLabel->setText("No enemy");
            enemyHPBar->setValue(0);
            enemyHPBar->setFormat("");
            enemyStatsLabel->setText("");
        }
        
        // Update location
        locationLabel->setText(QString("Location: %1").arg(currentLocation));
        
        // Update battle log
        battleLogText->setPlainText(battleLog.getRecent());
        battleLogText->verticalScrollBar()->setValue(
            battleLogText->verticalScrollBar()->maximum());
        
        // Enable/disable buttons
        bool canAct = inBattle && player->hp > 0 && currentEnemy && currentEnemy->hp > 0;
        attackBtn->setEnabled(canAct);
        defendBtn->setEnabled(canAct);
        itemBtn->setEnabled(canAct);
        abilityList->setEnabled(canAct);
        
        locationList->setEnabled(!inBattle);
        backtrackBtn->setEnabled(!inBattle && locationHistory.size() > 1);
        
        // Check win/lose
        if (player->hp <= 0) {
            QMessageBox::critical(this, "Game Over", "You have been defeated!");
            resetGame();
        }
        
        if (currentEnemy && currentEnemy->hp <= 0) {
            endBattle(true);
        }
    }
    
    void updateAbilityList() {
        abilityList->clear();
        vector<SkillNode*> skills;
        abilityTree.getUnlockedSkills(abilityTree.root, skills);
        
        for (auto skill : skills) {
            QString itemText = QString("%1 (MP: %2, DMG/Heal: %3)")
                .arg(skill->name).arg(skill->mpCost).arg(skill->damage);
            abilityList->addItem(itemText);
        }
    }
    
    void updateLocationList() {
        locationList->clear();
        auto connections = worldMap.connections[currentLocation];
        for (const QString& loc : connections) {
            QString marker = visitedLocations.count(loc) ? "✓ " : "? ";
            locationList->addItem(marker + loc);
        }
    }
    
    void updateDataStructuresInfo() {
        QString info = "Data Structures in Use:\n";
        info += "• Graph: World map\n";
        info += "• HashMap: Locations\n";
        info += "• Tree: Ability system\n";
        info += "• Stack: Travel history\n";
        info += "• Set: Visited places\n";
        info += "• Map: Inventory\n";
        info += "• Queue: Battle log\n";
        info += "• List: Status effects\n";
        info += "• Priority Queue: Turn order";
        dataStructLabel->setText(info);
    }
    
    void startBattle() {
        inBattle = true;
        
        int enemyLvl = worldMap.enemyLevel[currentLocation];
        QStringList enemyNames = {"Goblin", "Wolf", "Skeleton", "Orc", "Dragon"};
        QString enemyName = enemyNames[rand() % enemyNames.size()];
        
        int hp = 40 + enemyLvl * 15;
        int mp = 20 + enemyLvl * 5;
        int atk = 10 + enemyLvl * 5;
        int def = 5 + enemyLvl * 2;
        
        currentEnemy = new Character(enemyName, hp, mp, atk, def, false);
        currentEnemy->level = enemyLvl;
        
        battleLog.addMessage("=================================");
        battleLog.addMessage(QString("A wild %1 appears!").arg(enemyName));
        battleLog.addMessage("=================================");
        
        updateUI();
    }
    
    void endBattle(bool victory) {
        inBattle = false;
        battleTimer->stop();
        
        if (victory) {
            int expGain = currentEnemy->level * 30;
            int goldGain = currentEnemy->level * 20;
            
            battleLog.addMessage("=================================");
            battleLog.addMessage(QString("Victory! Gained %1 EXP!").arg(expGain));
            battleLog.addMessage("=================================");
            
            player->addExp(expGain);
            
            if (rand() % 3 == 0) {
                player->inventory["Potion"]++;
                battleLog.addMessage("Found a Potion!");
            }
        }
        
        delete currentEnemy;
        currentEnemy = nullptr;
        
        updateUI();
    }
    
    void enemyTurn() {
        if (!currentEnemy || currentEnemy->hp <= 0 || !inBattle) {
            battleTimer->stop();
            return;
        }
        
        int damage = currentEnemy->attack + rand() % 10;
        player->takeDamage(damage);
        
        battleLog.addMessage(QString("%1 attacks for %2 damage!")
            .arg(currentEnemy->name).arg(damage));
        
        updateUI();
        battleTimer->stop();
    }
    
    void resetGame() {
        player->hp = player->maxHp;
        player->mp = player->maxMp;
        currentLocation = "Starting Village";
        
        while (locationHistory.size() > 1) {
            locationHistory.pop();
        }
        
        inBattle = false;
        if (currentEnemy) {
            delete currentEnemy;
            currentEnemy = nullptr;
        }
        
        battleLog.addMessage("Game reset. Starting over...");
        updateUI();
        updateLocationList();
    }
    
private slots:
    void onAttack() {
        if (!currentEnemy || !inBattle) return;
        
        int damage = player->attack + rand() % 15;
        currentEnemy->takeDamage(damage);
        
        battleLog.addMessage(QString("You attack for %1 damage!").arg(damage));
        updateUI();
        
        if (currentEnemy->hp > 0) {
            battleTimer->start(1500); // Enemy attacks after 1.5 seconds
        }
    }
    
    void onDefend() {
        if (!currentEnemy || !inBattle) return;
        
        int tempDefense = player->defense;
        player->defense += 10;
        
        battleLog.addMessage("You brace for impact! Defense increased!");
        
        QTimer::singleShot(100, [this, tempDefense]() {
            player->defense = tempDefense;
            battleTimer->start(1500);
        });
        
        updateUI();
    }
    
    void onUseItem() {
        if (!inBattle) return;
        
        if (player->inventory["Potion"] > 0) {
            player->inventory["Potion"]--;
            player->heal(40);
            battleLog.addMessage("Used Potion! Restored 40 HP!");
            updateUI();
            battleTimer->start(1500);
        } else {
            QMessageBox::information(this, "No Items", "You don't have any potions!");
        }
    }
    
    void onUseAbility(QListWidgetItem* item) {
        if (!currentEnemy || !inBattle) return;
        
        QString abilityText = item->text();
        QString abilityName = abilityText.split(" (")[0];
        
        vector<SkillNode*> skills;
        abilityTree.getUnlockedSkills(abilityTree.root, skills);
        
        for (auto skill : skills) {
            if (skill->name == abilityName) {
                if (player->mp >= skill->mpCost) {
                    player->mp -= skill->mpCost;
                    
                    if (skill->type == "attack") {
                        currentEnemy->takeDamage(skill->damage);
                        battleLog.addMessage(QString("Cast %1 for %2 damage!")
                            .arg(skill->name).arg(skill->damage));
                    } else if (skill->type == "heal") {
                        player->heal(skill->damage);
                        battleLog.addMessage(QString("Cast %1! Restored %2 HP!")
                            .arg(skill->name).arg(skill->damage));
                    }
                    
                    updateUI();
                    
                    if (currentEnemy->hp > 0) {
                        battleTimer->start(1500);
                    }
                } else {
                    QMessageBox::information(this, "Not Enough MP", 
                        QString("Need %1 MP to cast %2!").arg(skill->mpCost).arg(skill->name));
                }
                break;
            }
        }
    }
    
    void onTravel(QListWidgetItem* item) {
        if (inBattle) return;
        
        QString locationText = item->text();
        QString newLocation = locationText.replace("✓ ", "").replace("? ", "");
        
        locationHistory.push(newLocation);
        currentLocation = newLocation;
        visitedLocations.insert(newLocation);
        
        battleLog.addMessage(QString("Traveled to %1").arg(newLocation));
        battleLog.addMessage(worldMap.locationDesc[newLocation]);
        
        updateLocationList();
        updateUI();
        
        // Random encounter
        if (rand() % 100 < 60) { // 60% chance
            startBattle();
        }
    }
    
    void onBacktrack() {
        if (locationHistory.size() <= 1 || inBattle) return;
        
        locationHistory.pop();
        currentLocation = locationHistory.top();
        
        battleLog.addMessage(QString("Backtracked to %1").arg(currentLocation));
        
        updateLocationList();
        updateUI();
    }
    
    void onShowSkillTree() {
        QString treeInfo = "Skill Tree (Unlocked abilities marked with ✓):\n\n";
        treeInfo += buildTreeString(abilityTree.root, 0);
        
        QMessageBox::information(this, "Ability Tree", treeInfo);
    }
    
    QString buildTreeString(SkillNode* node, int depth) {
        if (!node) return "";
        
        QString indent = QString("  ").repeated(depth);
        QString marker = node->unlocked ? "✓" : "✗";
        QString result = QString("%1%2 %3 (MP:%4, DMG:%5)\n")
            .arg(indent).arg(marker).arg(node->name).arg(node->mpCost).arg(node->damage);
        
        result += buildTreeString(node->left, depth + 1);
        result += buildTreeString(node->right, depth + 1);
        
        return result;
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    FantasyRPG game;
    game.show();
    
    return app.exec();
}

// Save as: main.cpp
// Compile with: qmake -project "QT += widgets" && qmake && make
// Or use Qt Creator IDE