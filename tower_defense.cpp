#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <map>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

// ============ DATA STRUCTURES ============

// 1. GRAPH - Dungeon room connections
struct Room {
    int id;
    string name;
    bool hasMonster;
    bool hasTreasure;
    bool visited;
    vector<int> connections; // LIST of connected rooms
};

class DungeonGraph {
public:
    unordered_map<int, Room> rooms; // HASHMAP of rooms
    unordered_map<int, pair<int, int>> positions; // HASHMAP for visual positions
    
    void addRoom(int id, string name, int x, int y) {
        rooms[id] = {id, name, false, false, false, {}};
        positions[id] = {x, y};
    }
    
    void connectRooms(int r1, int r2) {
        rooms[r1].connections.push_back(r2);
        rooms[r2].connections.push_back(r1);
    }
    
    void setMonster(int id) { rooms[id].hasMonster = true; }
    void setTreasure(int id) { rooms[id].hasTreasure = true; }
};

// 2. TREE - Skill tree for player upgrades
struct SkillNode {
    string name;
    int cost;
    bool unlocked;
    SkillNode* left;
    SkillNode* right;
    
    SkillNode(string n, int c) : name(n), cost(c), unlocked(false), left(nullptr), right(nullptr) {}
};

class SkillTree {
public:
    SkillNode* root;
    
    SkillTree() {
        // Create skill tree
        root = new SkillNode("Warrior", 0);
        root->unlocked = true;
        root->left = new SkillNode("Shield", 5);
        root->right = new SkillNode("Sword", 5);
        root->left->left = new SkillNode("Iron Shield", 10);
        root->left->right = new SkillNode("Magic Shield", 10);
        root->right->left = new SkillNode("Fire Sword", 10);
        root->right->right = new SkillNode("Ice Sword", 10);
    }
    
    bool unlockSkill(SkillNode* node, int& gold) {
        if (!node || node->unlocked) return false;
        if (gold >= node->cost) {
            gold -= node->cost;
            node->unlocked = true;
            return true;
        }
        return false;
    }
};

// 3. QUEUE - Event queue for game actions
struct GameEvent {
    string message;
    int timestamp;
};

// 4. STACK - Movement history for backtracking
class MovementHistory {
public:
    stack<int> history; // STACK
    
    void push(int roomId) {
        history.push(roomId);
    }
    
    int backtrack() {
        if (history.size() > 1) {
            history.pop(); // Remove current
            int prev = history.top();
            return prev;
        }
        return -1;
    }
};

// Player class
class Player {
public:
    int currentRoom;
    int health;
    int maxHealth;
    int gold;
    int attack;
    vector<string> inventory; // LIST
    MovementHistory moveHistory;
    
    Player() : currentRoom(0), health(100), maxHealth(100), gold(0), attack(10) {}
    
    void addItem(string item) {
        inventory.push_back(item);
    }
    
    void takeDamage(int dmg) {
        health = max(0, health - dmg);
    }
    
    void heal(int amount) {
        health = min(maxHealth, health + amount);
    }
};

// ============ GAME ENGINE ============
class DungeonGame {
private:
    sf::RenderWindow window;
    DungeonGraph dungeon;
    Player player;
    SkillTree skillTree;
    queue<GameEvent> eventQueue; // QUEUE
    vector<GameEvent> eventLog; // LIST
    map<int, int> monsterHealth; // ORDERED MAP
    
    sf::Font font;
    int eventCounter;
    bool showSkillTree;
    
public:
    DungeonGame() : window(sf::VideoMode(1200, 800), "Dungeon Explorer - Data Structures Game"),
                    eventCounter(0), showSkillTree(false) {
        srand(time(0));
        font.loadFromFile("arial.ttf");
        initializeDungeon();
        addEvent("Welcome to the Dungeon!");
        addEvent("Find treasure and defeat monsters!");
    }
    
    void initializeDungeon() {
        // Create dungeon layout (10 rooms)
        dungeon.addRoom(0, "Entrance", 200, 400);
        dungeon.addRoom(1, "Armory", 350, 250);
        dungeon.addRoom(2, "Library", 350, 550);
        dungeon.addRoom(3, "Treasury", 500, 150);
        dungeon.addRoom(4, "Kitchen", 500, 400);
        dungeon.addRoom(5, "Crypt", 500, 650);
        dungeon.addRoom(6, "Throne Room", 650, 300);
        dungeon.addRoom(7, "Garden", 650, 500);
        dungeon.addRoom(8, "Tower", 800, 200);
        dungeon.addRoom(9, "Dragon Lair", 800, 600);
        
        // Connect rooms
        dungeon.connectRooms(0, 1);
        dungeon.connectRooms(0, 2);
        dungeon.connectRooms(1, 3);
        dungeon.connectRooms(1, 4);
        dungeon.connectRooms(2, 4);
        dungeon.connectRooms(2, 5);
        dungeon.connectRooms(3, 6);
        dungeon.connectRooms(4, 6);
        dungeon.connectRooms(4, 7);
        dungeon.connectRooms(5, 7);
        dungeon.connectRooms(6, 8);
        dungeon.connectRooms(7, 9);
        dungeon.connectRooms(8, 9);
        
        // Add monsters and treasure
        dungeon.setMonster(3);
        dungeon.setMonster(5);
        dungeon.setMonster(8);
        dungeon.setMonster(9);
        
        dungeon.setTreasure(3);
        dungeon.setTreasure(6);
        dungeon.setTreasure(9);
        
        // Initialize monster health
        monsterHealth[3] = 30;
        monsterHealth[5] = 40;
        monsterHealth[8] = 50;
        monsterHealth[9] = 80; // Dragon!
        
        player.moveHistory.push(0);
        dungeon.rooms[0].visited = true;
    }
    
    void addEvent(string msg) {
        eventQueue.push({msg, eventCounter++});
        if (eventLog.size() >= 10) {
            eventLog.erase(eventLog.begin());
        }
        eventLog.push_back({msg, eventCounter});
    }
    
    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }
    
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::T) {
                    showSkillTree = !showSkillTree;
                }
                if (event.key.code == sf::Keyboard::B && !showSkillTree) {
                    backtrack();
                }
                if (event.key.code == sf::Keyboard::H && !showSkillTree) {
                    useHealthPotion();
                }
            }
            
            if (event.type == sf::Event::MouseButtonPressed && !showSkillTree) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                handleRoomClick(mousePos.x, mousePos.y);
            }
            
            if (event.type == sf::Event::MouseButtonPressed && showSkillTree) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                handleSkillClick(mousePos.x, mousePos.y);
            }
        }
    }
    
    void handleRoomClick(int x, int y) {
        for (auto& roomPair : dungeon.rooms) {
            auto pos = dungeon.positions[roomPair.first];
            int dx = x - pos.first;
            int dy = y - pos.second;
            
            if (sqrt(dx*dx + dy*dy) < 30) {
                // Check if connected to current room
                auto& currentRoom = dungeon.rooms[player.currentRoom];
                for (int connectedId : currentRoom.connections) {
                    if (connectedId == roomPair.first) {
                        moveToRoom(roomPair.first);
                        return;
                    }
                }
            }
        }
    }
    
    void moveToRoom(int roomId) {
        player.currentRoom = roomId;
        player.moveHistory.push(roomId);
        auto& room = dungeon.rooms[roomId];
        
        if (!room.visited) {
            room.visited = true;
            addEvent("Entered " + room.name);
            
            if (room.hasMonster && monsterHealth[roomId] > 0) {
                battleMonster(roomId);
            } else if (room.hasTreasure) {
                findTreasure(roomId);
                room.hasTreasure = false;
            }
        } else {
            addEvent("Returned to " + room.name);
        }
    }
    
    void battleMonster(int roomId) {
        int& hp = monsterHealth[roomId];
        addEvent("Monster appeared! HP: " + to_string(hp));
        
        int damage = player.attack + rand() % 10;
        hp -= damage;
        addEvent("You dealt " + to_string(damage) + " damage!");
        
        if (hp <= 0) {
            addEvent("Monster defeated!");
            int goldReward = 10 + rand() % 15;
            player.gold += goldReward;
            addEvent("Found " + to_string(goldReward) + " gold!");
            
            if (rand() % 3 == 0) {
                player.addItem("Health Potion");
                addEvent("Found a Health Potion!");
            }
        } else {
            int monsterDamage = 5 + rand() % 10;
            player.takeDamage(monsterDamage);
            addEvent("Took " + to_string(monsterDamage) + " damage!");
            
            if (player.health <= 0) {
                addEvent("You died! Game Over!");
            }
        }
    }
    
    void findTreasure(int roomId) {
        int gold = 20 + rand() % 30;
        player.gold += gold;
        addEvent("Found treasure: " + to_string(gold) + " gold!");
        
        if (roomId == 9) {
            addEvent("LEGENDARY TREASURE! You WIN!");
        }
    }
    
    void backtrack() {
        int prevRoom = player.moveHistory.backtrack();
        if (prevRoom != -1) {
            player.currentRoom = prevRoom;
            addEvent("Backtracked to " + dungeon.rooms[prevRoom].name);
        }
    }
    
    void useHealthPotion() {
        for (size_t i = 0; i < player.inventory.size(); i++) {
            if (player.inventory[i] == "Health Potion") {
                player.heal(30);
                player.inventory.erase(player.inventory.begin() + i);
                addEvent("Used Health Potion! +30 HP");
                return;
            }
        }
        addEvent("No potions available!");
    }
    
    void handleSkillClick(int x, int y) {
        // Simple skill tree layout
        if (tryUnlockSkill(skillTree.root->left, 400, 300, x, y)) return;
        if (tryUnlockSkill(skillTree.root->right, 700, 300, x, y)) return;
        if (tryUnlockSkill(skillTree.root->left->left, 300, 450, x, y)) return;
        if (tryUnlockSkill(skillTree.root->left->right, 500, 450, x, y)) return;
        if (tryUnlockSkill(skillTree.root->right->left, 600, 450, x, y)) return;
        if (tryUnlockSkill(skillTree.root->right->right, 800, 450, x, y)) return;
    }
    
    bool tryUnlockSkill(SkillNode* node, int nx, int ny, int x, int y) {
        if (!node) return false;
        int dx = x - nx;
        int dy = y - ny;
        if (sqrt(dx*dx + dy*dy) < 40) {
            if (skillTree.unlockSkill(node, player.gold)) {
                addEvent("Unlocked: " + node->name);
                player.attack += 5;
                player.maxHealth += 20;
                player.health += 20;
                return true;
            } else {
                addEvent("Need " + to_string(node->cost) + " gold!");
            }
            return true;
        }
        return false;
    }
    
    void update() {
        // Process event queue
        while (!eventQueue.empty()) {
            eventQueue.pop();
        }
    }
    
    void render() {
        window.clear(sf::Color(20, 20, 40));
        
        if (showSkillTree) {
            renderSkillTree();
        } else {
            renderDungeon();
        }
        
        window.display();
    }
    
    void renderDungeon() {
        // Draw connections
        for (auto& roomPair : dungeon.rooms) {
            auto pos1 = dungeon.positions[roomPair.first];
            for (int connId : roomPair.second.connections) {
                auto pos2 = dungeon.positions[connId];
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(pos1.first, pos1.second), sf::Color(100, 100, 100)),
                    sf::Vertex(sf::Vector2f(pos2.first, pos2.second), sf::Color(100, 100, 100))
                };
                window.draw(line, 2, sf::Lines);
            }
        }
        
        // Draw rooms
        for (auto& roomPair : dungeon.rooms) {
            auto pos = dungeon.positions[roomPair.first];
            sf::CircleShape circle(25);
            circle.setPosition(pos.first - 25, pos.second - 25);
            
            if (roomPair.first == player.currentRoom) {
                circle.setFillColor(sf::Color::Green);
            } else if (!roomPair.second.visited) {
                circle.setFillColor(sf::Color(100, 100, 100));
            } else {
                circle.setFillColor(sf::Color(50, 50, 150));
            }
            
            circle.setOutlineThickness(2);
            circle.setOutlineColor(sf::Color::White);
            window.draw(circle);
            
            // Draw indicators
            if (roomPair.second.hasMonster && monsterHealth[roomPair.first] > 0) {
                sf::CircleShape monster(8);
                monster.setPosition(pos.first - 8, pos.second - 40);
                monster.setFillColor(sf::Color::Red);
                window.draw(monster);
            }
            
            if (roomPair.second.hasTreasure) {
                sf::CircleShape treasure(8);
                treasure.setPosition(pos.first + 20, pos.second - 40);
                treasure.setFillColor(sf::Color::Yellow);
                window.draw(treasure);
            }
            
            // Room name
            sf::Text text;
            text.setFont(font);
            text.setString(roomPair.second.name);
            text.setCharacterSize(12);
            text.setFillColor(sf::Color::White);
            text.setPosition(pos.first - 30, pos.second + 30);
            window.draw(text);
        }
        
        // Draw UI Panel
        drawUIPanel();
    }
    
    void drawUIPanel() {
        // Right panel
        sf::RectangleShape panel(sf::Vector2f(250, 780));
        panel.setPosition(940, 10);
        panel.setFillColor(sf::Color(40, 40, 60, 230));
        panel.setOutlineThickness(2);
        panel.setOutlineColor(sf::Color::White);
        window.draw(panel);
        
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(14);
        text.setFillColor(sf::Color::White);
        
        int y = 25;
        
        text.setString("=== PLAYER STATUS ===");
        text.setPosition(960, y);
        window.draw(text);
        y += 30;
        
        text.setString("HP: " + to_string(player.health) + "/" + to_string(player.maxHealth));
        text.setPosition(960, y);
        window.draw(text);
        y += 25;
        
        text.setString("Gold: " + to_string(player.gold));
        text.setPosition(960, y);
        window.draw(text);
        y += 25;
        
        text.setString("Attack: " + to_string(player.attack));
        text.setPosition(960, y);
        window.draw(text);
        y += 35;
        
        text.setString("=== INVENTORY ===");
        text.setPosition(960, y);
        window.draw(text);
        y += 25;
        
        for (auto& item : player.inventory) {
            text.setString("- " + item);
            text.setPosition(970, y);
            window.draw(text);
            y += 20;
        }
        y += 20;
        
        text.setString("=== EVENT LOG ===");
        text.setPosition(960, y);
        window.draw(text);
        y += 25;
        
        for (auto& evt : eventLog) {
            text.setString(evt.message);
            text.setCharacterSize(11);
            text.setPosition(960, y);
            window.draw(text);
            y += 18;
        }
        
        // Data structures indicator
        y = 680;
        text.setCharacterSize(12);
        text.setString("DATA STRUCTURES:");
        text.setPosition(960, y);
        window.draw(text);
        y += 20;
        
        text.setString("Graph-Rooms, HashMap-Data");
        text.setPosition(960, y);
        window.draw(text);
        y += 15;
        
        text.setString("Stack-History, Queue-Events");
        text.setPosition(960, y);
        window.draw(text);
        y += 15;
        
        text.setString("List-Inventory, Map-Monsters");
        text.setPosition(960, y);
        window.draw(text);
        y += 15;
        
        text.setString("Tree-Skills (Press T)");
        text.setPosition(960, y);
        window.draw(text);
        y += 30;
        
        // Controls
        text.setCharacterSize(11);
        text.setString("B-Backtrack H-Heal T-Skills");
        text.setPosition(960, y);
        window.draw(text);
    }
    
    void renderSkillTree() {
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::White);
        
        text.setString("SKILL TREE (Press T to close)");
        text.setPosition(450, 50);
        window.draw(text);
        
        text.setString("Gold: " + to_string(player.gold));
        text.setPosition(520, 90);
        window.draw(text);
        
        // Draw skill tree
        drawSkillNode(skillTree.root, 550, 150);
        drawSkillNode(skillTree.root->left, 400, 300);
        drawSkillNode(skillTree.root->right, 700, 300);
        drawSkillNode(skillTree.root->left->left, 300, 450);
        drawSkillNode(skillTree.root->left->right, 500, 450);
        drawSkillNode(skillTree.root->right->left, 600, 450);
        drawSkillNode(skillTree.root->right->right, 800, 450);
        
        // Draw connections
        drawLine(550, 150, 400, 300);
        drawLine(550, 150, 700, 300);
        drawLine(400, 300, 300, 450);
        drawLine(400, 300, 500, 450);
        drawLine(700, 300, 600, 450);
        drawLine(700, 300, 800, 450);
    }
    
    void drawSkillNode(SkillNode* node, int x, int y) {
        if (!node) return;
        
        sf::CircleShape circle(35);
        circle.setPosition(x - 35, y - 35);
        
        if (node->unlocked) {
            circle.setFillColor(sf::Color::Green);
        } else {
            circle.setFillColor(sf::Color(100, 100, 100));
        }
        
        circle.setOutlineThickness(2);
        circle.setOutlineColor(sf::Color::White);
        window.draw(circle);
        
        sf::Text text;
        text.setFont(font);
        text.setString(node->name);
        text.setCharacterSize(12);
        text.setFillColor(sf::Color::White);
        text.setPosition(x - 30, y - 10);
        window.draw(text);
        
        if (!node->unlocked) {
            text.setString(to_string(node->cost) + "g");
            text.setCharacterSize(10);
            text.setPosition(x - 12, y + 5);
            window.draw(text);
        }
    }
    
    void drawLine(int x1, int y1, int x2, int y2) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x1, y1), sf::Color::White),
            sf::Vertex(sf::Vector2f(x2, y2), sf::Color::White)
        };
        window.draw(line, 2, sf::Lines);
    }
};

int main() {
    DungeonGame game;
    game.run();
    return 0;
}