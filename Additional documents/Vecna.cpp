#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <unordered_set>
#include <chrono>
#include <windows.h>
#include <map>
#include <thread>
#include <iomanip>
#include <sstream>

template<typename T>
std::string toStr(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const std::vector<std::string> SPELLS = { "IGNIS", "WATER", "TERRA", "AERO" };


void typeWriter(const std::string& text, int delayMs = 10) {
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
}

std::string generateLetterString(int length) {
    std::string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    std::string result;

    std::unordered_set<char> neededSet = { 'I', 'G', 'N', 'S', 'W', 'A', 'T', 'E', 'R', 'O', 'L', 'C', 'V', 'P', 'U', 'M', 'B'};
    std::vector<char> neededLetters(neededSet.begin(), neededSet.end());

    if (length < neededLetters.size()) {
        throw std::runtime_error("Length is too short to include all needed letters");
    }

    result = std::string(neededLetters.begin(), neededLetters.end());

    while (result.size() < static_cast<size_t>(length)) {
        result += letters[rand() % letters.size()];
    }

    std::random_shuffle(result.begin(), result.end());

    return result;
}


struct SpellResult {
    std::string spell;
    float castTimeSeconds;
};

SpellResult spellInputWindow(sf::Font& font) {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Spell Input");

    std::string letterString = generateLetterString(20);
    std::string castedSpell;

    sf::Text castText("", font, 30);
    castText.setPosition(50, 100);
    castText.setFillColor(sf::Color::Yellow);

    sf::Text resultText("", font, 30);
    resultText.setPosition(50, 150);
    resultText.setFillColor(sf::Color::Green);

    sf::RectangleShape castButton(sf::Vector2f(200, 50));
    castButton.setPosition(300, 500);
    castButton.setFillColor(sf::Color(50, 50, 200));

    sf::Text castButtonText("Cast", font, 30);
    castButtonText.setPosition(340, 510);
    castButtonText.setFillColor(sf::Color::White);

    sf::RectangleShape background(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    background.setFillColor(sf::Color(30, 30, 40));

    std::vector<sf::Text> letterTexts;
    for (int i = 0; i < letterString.size(); ++i) {
        sf::Text letterText(std::string(1, letterString[i]), font, 30);
        letterText.setPosition(50 + i * 30, 50);
        letterText.setFillColor(sf::Color::White);
        letterTexts.push_back(letterText);
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                if (castButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                    auto endTime = std::chrono::high_resolution_clock::now();
                    float duration = std::chrono::duration<float>(endTime - startTime).count();
                    window.close();
                    return { castedSpell, duration };
                }

                for (int i = 0; i < letterString.size(); ++i) {
                    if (letterTexts[i].getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                        castedSpell += toupper(letterString[i]);
                        letterString = generateLetterString(20);
                        letterTexts.clear();
                        for (int i = 0; i < letterString.size(); ++i) {
                            sf::Text letterText(std::string(1, letterString[i]), font, 30);
                            letterText.setPosition(50 + i * 30, 50);
                            letterText.setFillColor(sf::Color::White);
                            letterTexts.push_back(letterText);
                        }
                    }
                }
            }
        }

        castText.setString("Cast: " + castedSpell);

        window.clear();
        window.draw(background);
        for (const auto& letterText : letterTexts)
            window.draw(letterText);
        window.draw(castText);
        window.draw(resultText);
        window.draw(castButton);
        window.draw(castButtonText);
        window.display();
    }

    return { "", 0.0f };
}


void printStatus(const std::string& name, int hp, int maxHp, const std::string& status) {
    std::cout << name << " — HP: " << hp << "/" << maxHp;
    if (!status.empty()) std::cout << " | Эффекты: " << status;
    std::cout << "\n";
}

enum EffectType {
    NONE, BURN, WEAKEN, DODGE, STRENGTHEN, FREEZE, VAMP_BUFF
};

struct Effect {
    EffectType type;
    int duration;
};

enum ItemType {
    TRASH,
    HEAL_POTION,
    POISON,
    TOTEM
};

struct Item {
    ItemType type;
    std::string name;
    std::string description;
};

class Hero {
public:
    int hp = 20;
    int maxHp = 20;
    bool hasDodge = false;
    bool hasStrengthen = false;
    bool frozen = false;
    std::string name = "Игрок";
    std::vector<Effect> activeEffects;
    std::vector<Item> inventory;

    void applyEffect(Effect effect) {
        activeEffects.push_back(effect);
    }

    void addItem(const Item& item) {
        inventory.push_back(item);
        std::cout << "Ты получил: " << item.name << "\n";
    }

    void processEffects() {
        frozen = false;
        for (auto it = activeEffects.begin(); it != activeEffects.end();) {
            if (it->type == BURN) {
                std::cout << name << " горит! Получает 2 урона.\n";
                hp -= 2;
            }
            else if (it->type == FREEZE) {
                std::cout << name << " заморожен и пропускает ход.\n";
                frozen = true;
            }

            it->duration--;
            if (it->duration <= 0) it = activeEffects.erase(it);
            else ++it;
        }
    }

    void clearNegativeEffects() {
        activeEffects.erase(
            std::remove_if(activeEffects.begin(), activeEffects.end(), [](Effect& eff) {
                return eff.type == BURN || eff.type == WEAKEN || eff.type == FREEZE;
                }),
            activeEffects.end()
                    );
    }

    bool hasEffect(EffectType type) {
        for (auto& e : activeEffects) {
            if (e.type == type) return true;
        }
        return false;
    }

    std::string getEffectStatus() {
        std::string result;
        for (auto& e : activeEffects) {
            switch (e.type) {
            case BURN: result += "Горение "; break;
            case WEAKEN: result += "Ослабление "; break;
            case DODGE: result += "Уклонение "; break;
            case STRENGTHEN: result += "Усиление "; break;
            case FREEZE: result += "Заморозка "; break;
            case VAMP_BUFF: result += "Кровавый урон "; break;
            default: break;
            }
        }
        return result;
    }
};

class Enemy {
public:
    std::string name;
    int damage;
    int hp;
    int maxHp;
    bool frozen = false;
    std::vector<Effect> activeEffects;
    bool skipTurn = false;

    virtual void attack(Hero& hero) {
        if (frozen) {
            std::cout << name << " заморожен и пропускает ход!\n";
            frozen = false;
            return;
        }

        int actualDamage = damage;
        if (hasEffect(WEAKEN)) actualDamage = damage / 2;
        if (hero.hasEffect(DODGE)) {
            if (rand() % 2 == 0) {
                std::cout << "Ты уклонился от удара!\n";
                return;
            }
        }
        hero.hp -= actualDamage;
        std::cout << name << " атакует и наносит " << actualDamage << " урона.\n";
    }

    virtual void onAttacked(Hero& hero) {}

    virtual void applyEffect(Effect effect) {
        activeEffects.push_back(effect);
    }

    virtual void processEffects() {
        frozen = false;
        for (auto it = activeEffects.begin(); it != activeEffects.end();) {
            if (it->type == BURN) {
                std::cout << name << " горит! Получает 2 урона.\n";
                hp -= 2;
            }
            else if (it->type == FREEZE) {
                std::cout << name << " заморожен и пропускает ход.\n";
                frozen = true;
            }

            it->duration--;
            if (it->duration <= 0) it = activeEffects.erase(it);
            else ++it;
        }
    }

    bool hasEffect(EffectType type) {
        for (auto& e : activeEffects) {
            if (e.type == type) return true;
        }
        return false;
    }

    std::string getEffectStatus() {
        std::string result;
        for (auto& e : activeEffects) {
            switch (e.type) {
            case BURN: result += "Горение "; break;
            case WEAKEN: result += "Ослабление "; break;
            case FREEZE: result += "Заморозка "; break;
            case VAMP_BUFF: result += "Кровавый урон "; break;
            default: break;
            }
        }
        return result;
    }
};

class Zombie : public Enemy {
public:
    Zombie() {
        name = "Зомби";
        damage = 2;
        hp = 10;
        maxHp = 10;
    }

    void onAttacked(Hero& hero) override {
        if (frozen) {
            std::cout << name << " заморожен и не может использовать специальное умение!\n";
            frozen = false;
            return;
        }
        std::cout << name << " разъярён! Наносит дополнительно 1 урона.\n";
        hero.hp -= 1;
    }
};

class Skeleton : public Enemy {
public:
    Skeleton() {
        name = "Низший скелет";
        damage = 3;
        hp = 8;
        maxHp = 8;
    }
};


class Draugr : public Enemy {
public:
    Draugr() {
        name = "Драург";
        damage = 4;
        hp = 8;
        maxHp = 8;
    }

    void attack(Hero& hero) override {
        if (frozen) {
            std::cout << name << " заморожен и пропускает ход!\n";
            frozen = false;
            return;
        }

        int actualDamage = damage;
        if (hasEffect(WEAKEN)) actualDamage /= 2;

        if (rand() % 4 == 0) {
            std::cout << name << " впадает в ярость и бьёт дважды!\n";
            hero.hp -= actualDamage;
            std::cout << "Первый удар: " << actualDamage << " урона.\n";
            hero.hp -= actualDamage;
            std::cout << "Второй удар: " << actualDamage << " урона.\n";
        }
        else {
            Enemy::attack(hero);
        }
    }
};

class Ghoul : public Enemy {
public:
    Ghoul() {
        name = "Гуль";
        damage = 5;
        hp = 10;
        maxHp = 10;
    }

    void attack(Hero& hero) override {
        Enemy::attack(hero);
        if (rand() % 2 == 0) {
            std::cout << name << " парализует тебя! Ты пропускаешь следующий ход.\n";
            hero.applyEffect({ FREEZE, 3 });
        }
    }
};

class Revenant : public Enemy {
public:
    Revenant() {
        name = "Ревенант";
        damage = 6;
        hp = 14;
        maxHp = 14;
    }

    void applyEffect(Effect effect) override {
        if (effect.type == BURN) {
            std::cout << name << " неуязвим к огню!\n";
            return;
        }
        Enemy::applyEffect(effect);
    }
};

class Mummy : public Enemy {
public:
    Mummy() {
        name = "Мумия";
        damage = 7;
        hp = 16;
        maxHp = 16;
    }

    void attack(Hero& hero) override {
        Enemy::attack(hero);
        if (rand() % 2 == 0) {
            std::cout << name << " насылает древнее проклятие! Урон игрока уменьшен на 3 хода.\n";
            hero.applyEffect({ WEAKEN, 3 });
        }
    }
};

class Bloodsucker : public Enemy {
public:
    bool reflecting = false;

    Bloodsucker() {
        name = "Кровопийца";
        damage = 8;
        hp = 20;
        maxHp = 20;
    }

    void processEffects() override {
        Enemy::processEffects();
        reflecting = false;
        if (rand() % 10 < 3) {
            std::cout << name << " покрывается кровавым щитом. Следующая атака будет отражена!\n";
            reflecting = true;
        }
    }

    void onAttacked(Hero& hero) override {
        if (reflecting) {
            std::cout << name << " отражает атаку! Ты получаешь урон.\n";
            hero.hp -= 5;
            reflecting = false;
        }
    }
};

class Ghost : public Enemy {
public:
    Ghost() {
        name = "Дух";
        damage = 9;
        hp = 22;
        maxHp = 22;
    }

    void applyEffect(Effect effect) override {
        if (effect.type == FREEZE) {
            std::cout << name << " неуязвим к льду!\n";
            return;
        }
        Enemy::applyEffect(effect);
    }
};

class DeathKnight : public Enemy {
public:
    DeathKnight() {
        name = "Рыцарь смерти";
        damage = 10;
        hp = 50;
        maxHp = 50;
    }

    void attack(Hero& hero) override {
        if (frozen) {
            std::cout << name << " заморожен и пропускает ход!\n";
            frozen = false;
            return;
        }

        if (rand() % 4 == 0) {
            std::cout << name << " обрушивает невообразимую силу — тройной удар!\n";
            hero.hp -= damage * 3;
            std::cout << "Ты получаешь " << damage * 3 << " урона!\n";
        }
        else {
            Enemy::attack(hero);
        }
    }
};

std::vector<Item> dropLoot() {
    std::vector<Item> loot;

    if (rand() % 100 < 75)
        loot.push_back({ TRASH, "Останки", "Просто мусор." });

    if (rand() % 100 < 25)
        loot.push_back({ HEAL_POTION, "Зелье исцеления", "Восстанавливает 10 HP." });

    if (rand() % 100 < 15)
        loot.push_back({ POISON, "Яд", "Наносит врагу 10 урона." });

    if (rand() % 100 < 1)
        loot.push_back({ TOTEM, "Древний тотем", "Спасает тебя от смерти один раз." });

    return loot;
}

void castSpell(const std::string& spell, Hero& hero, Enemy& enemy) {
    if (spell == "IGNIS") {
        std::cout << "Ты поджигаешь врага. Он начинает гореть.\n";
        enemy.hp -= 5;
        enemy.applyEffect({ BURN, 2 });
        enemy.onAttacked(hero);
    }
    else if (spell == "TERRO") {
        std::cout << "Ты насылаешь тяжесть земли. Враг замедлен.\n";
        enemy.applyEffect({ WEAKEN, 2 });
        enemy.onAttacked(hero);
    }
    else if (spell == "AERO") {
        std::cout << "Порыв ветра окутывает тебя. Ты становишься быстрее.\n";
        hero.applyEffect({ DODGE, 1 });
        hero.applyEffect({ STRENGTHEN, 1 });
    }
    else if (spell == "WATER") {
        std::cout << "Прохладная вода заливает раны. Ты восстанавливаешь здоровье.\n";
        hero.hp += 5;
        if (hero.hp > hero.maxHp) hero.hp = hero.maxHp;
    }
    else if (spell == "VAMPIRUM") {
        std::cout << "Ты насылаешь проклятие крови. Жизненная сила врага уходит к тебе.\n";
        enemy.hp -= 3;
        hero.hp += 3;
        if (hero.hp > hero.maxHp) hero.hp = hero.maxHp;
        hero.applyEffect({ VAMP_BUFF, 3 });
        enemy.onAttacked(hero);
    }
    else if (spell == "GLACIA") {
        std::cout << "Ты насылаешь ледяной удар.\n";
        enemy.hp -= 5;
        if (rand() % 2 == 0) {
            std::cout << "Враг заморожен!\n";
            enemy.applyEffect({ FREEZE, 1 });
        }
        enemy.onAttacked(hero);
    }
    else if (spell == "VITA") {
        std::cout << "Свет наполняет тебя. Боль отступает.\n";
        hero.hp += 15;
        if (hero.hp > hero.maxHp) hero.hp = hero.maxHp;
        hero.clearNegativeEffects();
    }
    else if (spell == "UMBRA") {
        std::cout << "Тьма поглощает всё. Ни один свет не спасёт врага. Но за силу нужно платить... своей кровью.\n";
        enemy.hp -= 25;
        hero.hp -= 15;
        if (hero.hp < 0) hero.hp = 0;
        enemy.onAttacked(hero);
    }
    else {
        std::cout << "Ты колдуешь, но ничего не происходит...\n";
    }
}


enum class WorldLevel {
    WASTELAND = 0,
    SNOW_RUINS,
    SAND_WASTES,
    BURNED_JUNGLE,
    CRATER,
    COMPLETED
};

std::string getLevelName(WorldLevel level) {
    switch (level) {
    case WorldLevel::WASTELAND: return "Пустошь";
    case WorldLevel::SNOW_RUINS: return "Снежные руины";
    case WorldLevel::SAND_WASTES: return "Песчаные пустоши";
    case WorldLevel::BURNED_JUNGLE: return "Сожжённые джунгли";
    case WorldLevel::CRATER: return "Кратер";
    default: return "Неизвестно";
    }
}

void restCamp(Hero& player) {
    std::cout << "\nТы находишь укромное место среди руин и устраиваешь простой привал.\n";
    std::cout << "1. Посмотреть на небо\n";
    std::cout << "2. Поспать\n";
    std::cout << "3. Открыть инвентарь\n";
    std::cout << "4. Поговорить с голосом\n";
    std::cout << "5. Покинуть привал\n";
    std::cout << "Выбор: ";

    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1: {
        std::vector<std::string> thoughts = {
            "Хмм, а остались ли ещё люди...",
            "Я мечтаю о свободе, но не помню, что это значит...",
            "Иногда мне кажется, что я был кем-то важным...",
            "Небо такое же, как и раньше. Или нет?",
            "Если бы я мог вспомнить хоть одно имя...",
            "Сколько ещё пройдёт, прежде чем я стану собой?"
        };
        srand(static_cast<unsigned int>(time(nullptr)));
        std::cout << "\"" << thoughts[rand() % thoughts.size()] << "\"\n";
        break;
    }
    case 2: {
        std::cout << "Ты немного отдыхаешь. Силы восстанавливаются.\n";
        break;
    }
    case 3: {
        if (player.inventory.empty()) {
            std::cout << "Инвентарь пуст.\n";
        }
        else {
            std::cout << "Инвентарь:\n";
            for (size_t i = 0; i < player.inventory.size(); ++i) {
                std::cout << i + 1 << ". " << player.inventory[i].name << " — " << player.inventory[i].description << "\n";
            }
            std::cout << "Выбрать предмет для использования? (1 — Да, 2 — Назад): ";
            int sub;
            std::cin >> sub;
            if (sub == 1) {
                std::cout << "Введите номер предмета: ";
                int num;
                std::cin >> num;
                if (num >= 1 && num <= player.inventory.size()) {
                    Item& item = player.inventory[num - 1];
                    if (item.type == HEAL_POTION) {
                        std::cout << "Ты выпил зелье, восстановлено 10 HP.\n";
                        player.hp += 10;
                        if (player.hp > player.maxHp) player.hp = player.maxHp;
                        player.inventory.erase(player.inventory.begin() + num - 1);
                    }
                    else if (item.type == TRASH) {
                        std::cout << "Это просто мусор.\n";
                    }
                    else {
                        std::cout << "Этот предмет нельзя использовать вне боя.\n";
                    }
                }
            }
        }
    }
    case 4: {
        std::vector<std::string> thoughts = {
            "В этом мире ты либо учишься страдать, либо исчезаешь. Ты выбрал первое.",
            "Не трогай меня сейчас. Я устал… Даже призраки бывают не в духе.",
            "#&*^&$*@$^&^#!@*%!&#$^!**(!",
            "Ты чувствуешь, как магия щекочет тебе кости? Это значит, ты ещё жив. Пока."
            "Вот теперь я чувствую силу. Продолжай в том же духе, маг.",
            "Когда ты закричал от боли, мне стало… теплее. Странно, да?",
        };
        srand(static_cast<unsigned int>(time(nullptr)));
        std::cout << "\"" << thoughts[rand() % thoughts.size()] << "\"\n";
        break;
    }
    case 5: {
        std::cout << "Ты покидаешь привал, идя дальше.\n";
        break;
    }
    default: {
        std::cout << "Ты сидишь в тишине, чувствуя, как проходит время...\n";
        break;
    }
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void printMainMenu() {
    system("cls");
    std::cout << "==============================\n";
    std::cout << "      🌑 TEXT RPG: VEKNA 🌕\n";
    std::cout << "==============================\n";
    std::cout << " 1. ~#~  Новая игра ~#~\n";
    //std::cout << " 2. 💾 Продолжить (в разработке)\n";
    std::cout << " 2. #+ Выйти из игры +#\n";
    std::cout << "==============================\n";
    std::cout << "Выберите действие: ";
}

void combat(Hero& player, Enemy& enemy, sf::Font& font) {
    std::cout << "Ты встретил врага: " << enemy.name << "!\n";

    while (player.hp > 0 && enemy.hp > 0) {
        printStatus("Ты", player.hp, player.maxHp, player.getEffectStatus());
        printStatus(enemy.name, enemy.hp, enemy.maxHp, enemy.getEffectStatus());

        std::cout << "\nТвой ход: выбери заклинание\n";

        std::cout << "1. Кастовать заклинание\n2. Использовать предмет\n> ";
        int actionChoice;
        std::cin >> actionChoice;

        if (actionChoice == 1) {
            SpellResult result = spellInputWindow(font);
            std::cout << "Ты ввёл: " << result.spell << "\n";
            castSpell(result.spell, player, enemy);
        }
        else if (actionChoice == 2) {
            if (player.inventory.empty()) {
                std::cout << "Инвентарь пуст.\n";
            }
            else {
                std::cout << "Инвентарь:\n";
                for (size_t i = 0; i < player.inventory.size(); ++i) {
                    std::cout << i + 1 << ". " << player.inventory[i].name << " — " << player.inventory[i].description << "\n";
                }
                std::cout << "Выбери номер предмета: ";
                int idx;
                std::cin >> idx;
                if (idx >= 1 && idx <= player.inventory.size()) {
                    Item item = player.inventory[idx - 1];
                    if (item.type == HEAL_POTION) {
                        std::cout << "Ты выпиваешь зелье. Восстановлено 10 HP.\n";
                        player.hp += 10;
                        if (player.hp > player.maxHp) player.hp = player.maxHp;
                        player.inventory.erase(player.inventory.begin() + idx - 1);
                    }
                    else if (item.type == POISON) {
                        std::cout << "Ты кидаешь яд. Враг получает 10 урона!\n";
                        enemy.hp -= 10;
                        player.inventory.erase(player.inventory.begin() + idx - 1);
                    }
                    else {
                        std::cout << "Этот предмет нельзя использовать сейчас.\n";
                    }
                }
            }
        }


        if (enemy.hp <= 0) {
            std::cout << "Враг побеждён!\n";
            auto loot = dropLoot();
            if (!loot.empty()) {
                std::cout << "Добыча:\n";
                for (size_t i = 0; i < loot.size(); ++i) {
                    std::cout << i + 1 << ". " << loot[i].name << " — " << loot[i].description << "\n";
                }

                std::cout << "Выбери:\n";
                std::cout << "1. Взять всё\n2. Взять определённый предмет\n3. Ничего не брать\n> ";
                int choice;
                std::cin >> choice;

                if (choice == 1) {
                    for (const auto& item : loot) player.addItem(item);
                }
                else if (choice == 2) {
                    std::cout << "Номер предмета: ";
                    int num;
                    std::cin >> num;
                    if (num >= 1 && num <= loot.size()) player.addItem(loot[num - 1]);
                }
            }
            break;
        }

        player.processEffects();
        enemy.processEffects();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::cout << "\nХод врага...\n";
        if (!enemy.skipTurn) enemy.attack(player);
        else {
            std::cout << "Враг пропускает ход.\n";
            enemy.skipTurn = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    if (player.hp <= 0) {
        auto it = std::find_if(player.inventory.begin(), player.inventory.end(),
            [](const Item& item) { return item.type == TOTEM; });

        if (it != player.inventory.end()) {
            std::cout << "Тотем оживляет тебя! Ты снова полон сил!\n";
            player.hp = player.maxHp;
            player.inventory.erase(it);
        }
        else {
            std::cout << "Ты пал в бою...\n";
            exit(0);
        }
    }
}

void voiceLine(WorldLevel level, int step) {
    std::vector<std::string> lines;
    switch (level) {
    case WorldLevel::WASTELAND:
        lines = {
            "Голос в голове: Ха. Поздравляю! Твой первый побеждённый враг!",
            "Голос в голове: Вот так вот! Продолжай в том же духе.",
            "Голос в голове: Ты начинаешь привыкать к боли. Это хорошо. Боль — отличный учитель. Надёжнее меня.",
            "Голос в голове: Ты спрашиваешь, кто я? Пока просто голос. Проводник. Друг… может быть. Если заслужишь."
        }; break;
    case WorldLevel::SNOW_RUINS:
        lines = {
            "Голос в голове: Холод проникает под кожу... Прям до костей... Привыкай.",
            "Голос в голове: Видишь, как ломаются? Даже лёд не вечен. А ты — пока держишься.",
            "Голос в голове: Это место помнит боль. Ты её лишь усилил.",
            "Голос в голове: А ты не замечаешь, как стал жестче? Не спеши. Это только начало."
        }; break;
    case WorldLevel::SAND_WASTES:
        lines = {
            "Голос в голове: Ты думаешь, огонь сжигает плоть? Нет. Плоть сгорает от предсмертного бешенства клеток…",
            "Голос в голове: Солнце здесь убивает медленно. Ты — быстрее.",
            "Голос в голове: Песок… он забивает тебе глаза, рот, душу. Но ты всё равно смотришь вперёд.",
            "Голос в голове: СИЛА! СИЛА! СИЛА!"
        }; break;
    case WorldLevel::BURNED_JUNGLE:
        lines = {
            "Голос в голове: Ты слышишь, как хрустит пепел под ногами? Это мёртвые, что даже не встали. Им не повезло.",
            "Голос в голове: Жара съедает всё. Даже страх. Но не ярость… её тут хватает.",
            "Голос в голове: ХА-ХА-ХА ГОРИ! ГОРИ! ВСЁ СГОРЕЛО! ХА-ХА-ХА",
            "Голос в голове: $/&#NБ.?#|%Л.&?#$И.%^@З.*&$$К.&@$О.*@%"
        }; break;
    case WorldLevel::CRATER:
        lines = { "..." };
        break;
    default:
        lines = { "..." };
    }
    if (step < lines.size()) std::cout << "\nГолос: \"" << lines[step] << "\"\n";
}

void transitionText(WorldLevel level) {
    switch (level) {
    case WorldLevel::SNOW_RUINS:
        std::cout << "\nШесть месяцев спустя... Пустошь сменилась ледяными пейзажами. Ты вступаешь в Снежные руины.\n";
        break;
    case WorldLevel::SAND_WASTES:
        std::cout << "\nШесть месяцев спустя... Холод сменился зноем. Ты входишь в Песчаные пустоши.\n";
        break;
    case WorldLevel::BURNED_JUNGLE:
        std::cout << "\nШесть месяцев спустя... Пески обуглились. Ты пробираешься в Сожжённые джунгли.\n";
        break;
    case WorldLevel::CRATER:
        std::cout << "\nШесть месяцев спустя... Джунгли исчезли. Перед тобой зияет Кратер.\n";
        break;
    default: break;
    }
}

void finalChoice() {
    typeWriter("\nТы победил. Это чудовище свержено."
        "\nНо... когда ты использовал своё последнее, сильнейшее заклинание — всё изменилось."
        "\n\nТвои глаза заполнил ослепительный свет, и внутри него... ты увидел:"
        "\nТурниры. Победы. Ученики. Признание."
        "\nБыл один ученик — не просто талантливый. Гениальный."
        "\nЕго звали Векна."
        "\n\nОн постигал законы магии быстрее, чем его учителя успевали их формулировать. Он превзошёл всех."
        "\nОн стал сильнейшим магом на земле."
        "\nНо… ему было мало."
        "\n\nПрошли десятилетия. Он начал стареть. Даже могущественнейшие заклинания не могли остановить время."
        "\nНо что если можно найти путь?"
        "\nОн посвятил века поиску. И нашёл: стал личом."
        "\nОбрёл бессмертие. Почти."
        "\n\nИ снова ему было мало. Он захотел большего — стать архиличом."
        "\nОн собрал могущественные артефакты. Разрушил их. Поглотил магическую силу целых эпох."
        "\n\nНо... его оболочка не выдержала."
        "\nМагическая энергия вырвалась наружу. Катастрофа. Планета была уничтожена."
        "\nОн исчез. Но филакторий остался — кольцо. Оно создало новую оболочку из пепла и памяти."
        "\nСначала — младенец. Потом — ты."
        "\nНо магия снова сорвалась — и оболочку отбросило на край планеты. Память утеряна. Но ты выжил."
        "\n\n\tТы — Векна.");


    typeWriter("\n\n\nГолос в голове : Вот и всё. Теперь ты знаешь."
        "\nГолос в голове : Ты — я. Я — ты. Остаток твоей души. Я — то, что ты оставил в кольце."
        "\nИгрок : Ты знал с самого начала?"
        "\nГолос в голове : Да. Но разве бы ты поверил, если бы я сразу сказал?"
        "\nГолос в голове : Я хотел... нет. Я хочу лучшего для нас обоих."
        "\nГолос в голове : Мы можем снова стать единым целым. Мы можем стать сильнейшим существом, которое когда-либо ступало по этой земле."
        "\nГолос в голове : Архилич. Почти бог."
        "\nИгрок : ..."
        "\nГолос в голове : Просто надень кольцо"
        "\nГолос в голове : Так что, Векна...");

    typeWriter("\nГолос: \"Теперь ты должен выбрать.\"\n");
    std::cout << "1. Попробовать вернуть всё, как было до катастрофы.\n";
    std::cout << "2. Надеть кольцо и стать архиличом, потеряв человечность.\n";
    std::cout << "3. Оставить кольцо и жить как человек в этом мире.\nВыбор: ";
    int choice;
    std::cin >> choice;
    switch (choice) {
    case 1:
        typeWriter("\nТы собираешь остатки магической силы. Фокусируешь волю. Пространство искривляется.\n");
        typeWriter("ЧТО ТЫ ДЕЛАЕШЬ?\n");
        typeWriter("Ты пытаешься сконцентрировать всю свою магическую силу лишь на одном...\n");
        typeWriter("Голос в голове : СТОЙ! ОСТАНОВИСЬ! НЕ ДЕЛАЙ ЭТОГО!\n");
        typeWriter("Ты представляешь мгновения прошлого, которые тебе довелось вспомнить... Мама. Академия. Покой.\n");
        typeWriter("***Яркая вспышка***\n");
        break;
    case 2:
        typeWriter("\nТы медленно берёшь кольцо. Оно пульсирует.\n");
        typeWriter("Как только ты надеваешь его, всё замирает.\n");
        typeWriter("Ты чувствуешь силу. Силу, которую не может постичь ни один смертный разум.\n");
        typeWriter("Но с этой силой уходит нечто ещё. Тепло. Память. Человечность.\n");
        typeWriter("Ты больше не Векна. Ты — Архилич.\n");
        break;
    case 3:
        typeWriter("\nТы долго смотришь на кольцо. Оно лежит на пепле прошлого, сверкая слабым светом.\n");
        typeWriter("\nДелая шаг назад, ты отворачиваешься и уходишь\n");
        typeWriter("Голос в голове : СТОЙ! ПОЖАЛУЙСТА! НЕ УХОДИ... НЕ ОСТАВЛЯЙ МЕНЯ...\n");
        typeWriter("Оно остаётся там, в центре кратера. Ожидает. Но ты — свободен.\n");
        typeWriter("Мир пуст, суров и жесток. Но в нём ты — человек.\n");
        typeWriter("А может, это и есть настоящая магия.\n");
        break;
    default:
        typeWriter("\nТы не делаешь выбор.\n");
        typeWriter("И всё остаётся так, как есть. Голос затихает. Кольцо тускнеет.\n");
        typeWriter("Мир замирает, ожидая, когда ты вновь будешь готов выбрать.\n");
    }
    typeWriter("Спасибо за прохождение игры!\n");
    exit(0);
}

void exploreLevel() {
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Ошибка загрузки шрифта!\n";
        exit(-1);
    }

    Hero player;

    //std::cout << "Введите имя персонажа: ";
    //std::getline(std::cin, player.name);

    srand(time(0));
    WorldLevel currentLevel = WorldLevel::WASTELAND;
    int step = 0;

    typeWriter("\n🧙‍Ты приходишь в себя, лёжа на потрескавшейся земле. Ветер тащит за собой пепел. "
        "\nМир... пуст. Обугленные кости, вывернутые деревья, мёртвая земля. Ни шороха, ни крика, ни жизни.Только звенящая тишина."
        "\nНо…"
        "\n…тебя кто-то зовёт."
        "\n\n??: "
        "\nТы оборачиваешься, но за тобой никого нет. Голос. Прямо. В твоей голове. "
        "\nГолос в голове: Ты очнулся.. Это хорошо…"
        "\nИгрок : Что произошло? почему всё уничтожено?"
        "\nГолос в голове: Был какой-то магический взрыв. Такой силы, что уничтожил всё живое..."
        "\nИгрок : Кто ты такой? Почему я ничего не помню?"
        "\nГолос в голове: Прости, но я не могу тебе это сказать."
        "\nИгрок : Тогда почему я должен тебе верить?"
        "\nГолос в голове: А у тебя есть выбор? Поблизости нет ни единого человека, слушай меня и я помогу тебе… И когда придут время всё расскажу.."
        "\nОглянувшись и посмотрев на одни руины вокруг ты согласился."
        "\n\nГолос в голове: Начнём с основ магии. 4 элемента - 4 первобытных заклинания."
        "\n\tIGNIS – сильное заклинание, наносит большой урон и поджигает врага. "
        "\n\tWATER — немного залечивает твои раны. +5 hp"
        "\n\tAERO — шанс уклонения от следующей атаки (50%), усиление следующей атаки (50%)"
        "\n\tTERRO — ослабляет твоего противника снижая его урон на 50% на 2 атаки."
        "\n\nГолос в голове: Запомни их как следует. В бою я их напомнить тебе не смогу."
        "\n\nТы чувствуешь, как заклинания загораются в тебе, будто были там всегда. Применяешь их — и…"
        "\n…вспышки."
        "\nНебольшая деревянная комнатушка.Детская кроватка.Красивая золотоволосая девушка.Запах корицы."
        "\nТы приходишь в себя"
        "\nГолос в голове : Ты тут ? Этих заклинаний тебе должно хватить."
        "\nГолос в голове : Отправляйся на восток.Там ты найдёшь ответы…\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    while (true) {
        typeWriter("\nТекущий уровень: " + toStr(getLevelName(currentLevel)) + "\n");
        typeWriter("1. Пойти дальше\n2. Сделать привал\n3. Открыть инвентарь\nВаш выбор: ");

        int action;
        std::cin >> action;

        if (action == 1) {

            typeWriter("\nТы находишься в локации: " + toStr(getLevelName(currentLevel)) + "\n");
            //if (currentLevel != WorldLevel::CRATER)
            //    std::cout << "Голос в голове: \"";

            int maxSteps = 4;

            if (step < maxSteps) {
                if (currentLevel == WorldLevel::WASTELAND) {
                    if (step % 2 == 0) {
                        Zombie zombie;
                        Enemy& enemy = zombie;
                        combat(player, enemy, font);
                    }
                    else {
                        Skeleton skeleton;
                        Enemy& enemy = skeleton;
                        combat(player, enemy, font);
                    }
                }
                else if (currentLevel == WorldLevel::SNOW_RUINS) {
                    if (step % 2 == 0) {
                        Draugr draugr;
                        Enemy& enemy = draugr;
                        combat(player, enemy, font);
                    }
                    else {
                        Ghoul ghoul;
                        Enemy& enemy = ghoul;
                        combat(player, enemy, font);
                    }
                }
                else if (currentLevel == WorldLevel::SAND_WASTES) {
                    if (step % 2 == 0) {
                        Revenant revenant;
                        Enemy& enemy = revenant;
                        combat(player, enemy, font);
                    }
                    else {
                        Mummy mummy;
                        Enemy& enemy = mummy;
                        combat(player, enemy, font);
                    }
                }
                else if (currentLevel == WorldLevel::BURNED_JUNGLE) {
                    if (step % 2 == 0) {
                        Bloodsucker bloodsucker;
                        Enemy& enemy = bloodsucker;
                        combat(player, enemy, font);
                    }
                    else {
                        Ghost ghost;
                        Enemy& enemy = ghost;
                        combat(player, enemy, font);
                    }
                }
                //std::cout << "\n--- ХОД " << (step + 1) << " ---\n";
                //std::cout << "БОЙ!!!";
                voiceLine(currentLevel, step);
                step++;
            }
            else {
                if (currentLevel == WorldLevel::WASTELAND) {
                    typeWriter("\nВремя идёт. Ты идёшь. День сменяет ночь, ночь сменяет день. И так из раза в раз."
                        "\nПепел сменяется льдом…"
                        "\nПустоши остаются позади, в них больше нечего искать."
                        "\n\nПрошло шесть месяцев."
                        "\n\nШесть месяцев странствий по выжженной земле, сражений, шёпота в голове.Ты стал сильнее. Max HP +5"
                        "\nПеред тобой — Снежные руины."
                        "\nКолонны из обледеневшего мрамора, дома, сдавшиеся времени и холоду."
                        "\nИ… новые враги."
                        "\n\nГолос в голове : Будь аккуратнее. Слабая нечисть здесь бы не выжила."
                        "\nГолос в голове : Приспособились только самые сильные… Самые беспощадные...");
                    player.maxHp += 5;
                    typeWriter("\nГолос в голове : Чем дальше ты заходишь, тем суровее условия. "
                        "\nГолос в голове : Придётся мне тебе дать новое заклинание."
                        "\n\tGLACIA Обрушивает ледяной удар, нанося значительный урона и с шансом 50 % замораживает врага на 1 ход."
                        "\n\nТы используешь заклинания и видишь... Мальчик и девочка лет десяти. Они.. играют со мной? В мячик?"
                        "\nИгрок : слушай, тут такое дело, когда я использую новое заклинание, я вижу какие-то видения..."
                        "\nГолос в голове : д - да..точно..Я забыл тебя предупредить, это остаточные видения."
                        "\nНаверное людей, которые умерли здесь .Магия становится катализатором для магической энергии и ты видишь их предсмертные послания.");
                }
                else if (currentLevel == WorldLevel::SNOW_RUINS) {
                    typeWriter("\nВремя идёт. Ты идёшь. День сменяет ночь, ночь сменяет день. И так из раза в раз."
                        "\n\nПрошло шесть месяцев."
                        "\n\nШесть месяцев странствий по замороженным руинам. Сражения. Неистовый мороз. Ты стал сильнее. Max HP +5"
                        "\n\nЛёд трескается, под ним — пыль, древняя и жёлтая, как кости богов."
                        "\nВетер стал горячим. Солнцу неистово печёт."
                        "\nПозади — мёртвый холод.Впереди — жгучий зной."
                        "\nТы в Песчаных пустошах.");
                    player.maxHp += 5;
                    typeWriter("\nГолос в голове : Чем дальше ты заходишь, тем суровее условия. "
                        "\nГолос в голове : Придётся мне тебе дать новое заклинание."
                        "\n\tVITA Мгновенно лечит 15 HP и снимает все негативные эффекты."
                        "\n\nТы используешь заклинания и видишь... Королевская академия магии.Турнир.Победа.Награждение.Лучший ученик.Сильнейший маг."
                        "\nДаже такой одарённый маг не пережил катастрофы... А смог ли тогда хоть кто-то её пережить...");
                }
                else if (currentLevel == WorldLevel::SAND_WASTES) {
                    typeWriter("\nВремя идёт. Ты идёшь. День сменяет ночь, ночь сменяет день. И так из раза в раз."
                        "\n\nПрошло шесть месяцев."
                        "\n\nШесть месяцев странствий по песчаным пустошам. Сражения. Неистовый зной. Ты стал сильнее. Max HP +5"
                        "\n\nПесок сменяется гарью."
                        "\nТы вдыхаешь воздух, и он оседает на стенках твоих лёгких."
                        "\nПод ногами — пепел, угли, обугленные дома, кости, корни деревьев, которых больше нет."
                        "\nЧто - то тут горело… долго. Но на обычный пожар это не похоже, тут замешена магия..."
                        "\nТы ступаешь в Сожжённые джунгли.");
                    player.maxHp += 5;
                    typeWriter("\nГолос в голове : Чем дальше ты заходишь, тем суровее условия. "
                        "\nГолос в голове : Придётся мне тебе дать новое заклинание."
                        "\n\tVAMPIRUM Вампирское заклинание. Вытягивает 3 единицы силы врага, ворует 3 HP врага и лечит тебя на 3 HP."
                        "\n\nТы используешь заклинания и видишь... Какая - то мерзкая тварь.Мозговой месиво с большим глазом и парой щупалец с глазами поменьше.Вспышка."
                        "\nСвет. Загадочная книга. Сосуд со странной жидкостью. Кольцо. В отражении жидкости ты видишь. Это ты. Только будто постаревший. Но это ты."
                        "\nТам был я... Этого не может быть... Это не может быть остаточным предсмертным видением.. я же жив.. ведь так?... Чего-то он мне не договаривает...");
                }
                else if (currentLevel == WorldLevel::BURNED_JUNGLE) {
                    typeWriter("\nПрошло шесть месяцев."
                        "\n\nШесть месяцев странствий по вызженным джунглям. Сражения. Выживание. Ты стал сильнее. Max HP +10"
                        "\n\nВсё затихает."
                        "\nНи пепла, ни песка, ни ветра."
                        "\nТолько земля… вывернутая наизнанку."
                        "\nА перед собой ты видишь огромную воронку. Кратер, границы которого даже трудно разглядеть."
                        "\nЭто оно... Взрыв. Он был тут."
                        "\nТы пришёл туда, где всё началось."
                        "\nГде ты умер."
                        "\nГде ты… родился."
                        "\nДобро пожаловать в Кратер. Начало конца");
                    player.maxHp += 10;
                    typeWriter("\nМы на месте. Если кто-то и выжил в эпицентре — он невероятно силён... будь на чеку."
                        "\nГолос в голове : Пожалуйста, используй это заклинание ТОЛЬКО в крайнем случае..."
                        "\n\tUMBRA Призывает абсолютную тьму, уничтожающую всё.");
                }
                currentLevel = static_cast<WorldLevel>(static_cast<int>(currentLevel) + 1);
                transitionText(currentLevel);
                step = 0;
                
            }

            if (currentLevel == WorldLevel::CRATER) {
                //std::cout << "ФИНАЛЬНЫЙ БОЙ!!!";
                DeathKnight deathKnight;
                Enemy& enemy = deathKnight;
                combat(player, enemy, font);
                voiceLine(currentLevel, 0);
                finalChoice();
                currentLevel = WorldLevel::COMPLETED;
            }
        }
        else if (action == 2) {
            restCamp(player);
        }
        else if (action == 3) {
            if (player.inventory.empty()) {
                typeWriter("Инвентарь пуст.\n");
            }
            else {
                typeWriter("Инвентарь:\n");
                for (size_t i = 0; i < player.inventory.size(); ++i) {
                    std::cout << i + 1 << ". " << player.inventory[i].name << " — " << player.inventory[i].description << "\n";
                }
                typeWriter("Выбрать предмет для использования? (1 — Да, 2 — Назад): ");
                int sub;
                std::cin >> sub;
                if (sub == 1) {
                    typeWriter("Введите номер предмета: ");
                    int num;
                    std::cin >> num;
                    if (num >= 1 && num <= player.inventory.size()) {
                        Item& item = player.inventory[num - 1];
                        if (item.type == HEAL_POTION) {
                            typeWriter("Ты выпил зелье, восстановлено 10 HP.\n");
                            player.hp += 10;
                            if (player.hp > player.maxHp) player.hp = player.maxHp;
                            player.inventory.erase(player.inventory.begin() + num - 1);
                        }
                        else if (item.type == TRASH) {
                            typeWriter("Это просто мусор.\n");
                        }
                        else {
                            typeWriter("Этот предмет нельзя использовать вне боя.\n");
                        }
                    }
                }
            }
        }
        else {
            typeWriter("Неверный выбор. Попробуй ещё раз.\n");
        }
    }
}

int main() {
    SetConsoleOutputCP(1251);
    srand(static_cast<unsigned int>(time(nullptr)));

    while (true) {
        printMainMenu();
        int choice;
        std::cin >> choice;

        if (choice == 1) {
            exploreLevel();

        }
        //else if (choice == 2) {
        //    std::cout << "Функция в разработке...\n";
        //    std::this_thread::sleep_for(std::chrono::seconds(1));
        //}
        else if (choice == 2) {
            typeWriter("До встречи, герой.\n");
            break;
        }
        else {
            typeWriter("Неверный ввод. Попробуй снова.\n");
        }
    }

    return 0;
}

