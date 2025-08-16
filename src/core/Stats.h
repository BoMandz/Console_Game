struct BaseStats {
    int hpMax;
    int manaMax;
    int atk;
    int def;
    int spd;           // turn order + move range
    int intl;          // CC resist + special potency
};

struct Stats
{
    BaseStats base_stats;
    int currentHp;
    int currentMana;

    Stats(const BaseStats& bs)
        : base_stats(bs),
          currentHp(bs.hpMax),
          currentMana(bs.manaMax)
    {}

    void takeDamage(int amount);
    void heal(int amount);
    void useMana(int amount);
    void restoreMana(int amount);

    void increaseMaxHp(int amount);
    void increaseMaxMana(int amount);
    void increaseAtk(int amount);
    void increaseDef(int amount);
    void increaseSpd(int amount);
    void increaseIntl(int amount);    

    void print() const;  
};
