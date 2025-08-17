#include "include/core/Stats.h"
#include <algorithm>
#include <iostream>

// Helper to safely modify a base stat (minimum 1)
inline void modifyBaseStat(int& stat, int amount) {
    stat += amount;
    if (stat < 1) stat = 1;
}

// Helper to modify a base stat by percent
inline void modifyBaseStatPercent(int& stat, int percent) {
    int delta = stat * percent / 100;
    modifyBaseStat(stat, delta);
}

// HP/Mana functions
void Stats::takeDamage(int amount) {
    currentHp = std::max(currentHp - amount, 0);
}

void Stats::heal(int amount) {
    currentHp = std::min(currentHp + amount, base_stats.hpMax);
}

void Stats::useMana(int amount) {
    currentMana = std::max(currentMana - amount, 0);
}

void Stats::restoreMana(int amount) {
    currentMana = std::min(currentMana + amount, base_stats.manaMax);
}

// Increase/decrease base stats safely
void Stats::increaseMaxHp(int amount) {
    modifyBaseStat(base_stats.hpMax, amount);
    currentHp = std::max(currentHp + amount, 0);
}

void Stats::increaseMaxMana(int amount) {
    modifyBaseStat(base_stats.manaMax, amount);
    currentMana = std::max(currentMana + amount, 0);
}

void Stats::increaseAtk(int amount) {
    modifyBaseStat(base_stats.atk, amount);
}

void Stats::increaseDef(int amount) {
    modifyBaseStat(base_stats.def, amount);
}

void Stats::increaseSpd(int amount) {
    modifyBaseStat(base_stats.spd, amount);
}

void Stats::increaseIntl(int amount) {
    modifyBaseStat(base_stats.intl, amount);
}

// Percentage-based increases
void Stats::increaseMaxHpPercent(int percent) {
    int oldMax = base_stats.hpMax;
    modifyBaseStatPercent(base_stats.hpMax, percent);
    currentHp += (base_stats.hpMax - oldMax);
    currentHp = std::max(currentHp, 0);
}

void Stats::increaseMaxManaPercent(int percent) {
    int oldMax = base_stats.manaMax;
    modifyBaseStatPercent(base_stats.manaMax, percent);
    currentMana += (base_stats.manaMax - oldMax);
    currentMana = std::max(currentMana, 0);
}

void Stats::increaseAtkPercent(int percent) {
    modifyBaseStatPercent(base_stats.atk, percent);
}

void Stats::increaseDefPercent(int percent) {
    modifyBaseStatPercent(base_stats.def, percent);
}

void Stats::increaseSpdPercent(int percent) {
    modifyBaseStatPercent(base_stats.spd, percent);
}

void Stats::increaseIntlPercent(int percent) {
    modifyBaseStatPercent(base_stats.intl, percent);
}

// Optional: debug print function
void Stats::print() const {
    std::cout << "HP: " << currentHp << "/" << base_stats.hpMax
              << ", Mana: " << currentMana << "/" << base_stats.manaMax
              << ", ATK: " << base_stats.atk
              << ", DEF: " << base_stats.def
              << ", SPD: " << base_stats.spd
              << ", INT: " << base_stats.intl
              << std::endl;
}
