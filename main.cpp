#include "include/world/DungeonGeneratorHard.h"
#include "include/world/DungeonCommon.h"

int main(int argc, char const *argv[])
{
    HardFloor b;
    HardDungeonGenerator a ;
    a.generate(b, 6);
    return 0;
}
