// gcc godseed0.c libcubiomes.a -fwrapv -lm && a.out
// todo: make target

#include "finders.h"
#include "generator.h"
#include <stdio.h>
#include <sys/random.h>

int toRegXZ(int x)
{
    if (x < 0)
    {
        return -1;
    }
    return 0;
}

int main()
{
    int mc = MC_1_16_1;

    Generator g;
    setupGenerator(&g, mc, 0);

    uint64_t seed;
    for (;;)
    {
        getrandom(&seed, 8, 0);
        // printf("checking seed %" PRId64 "\n", (int64_t)seed);

        applySeed(&g, DIM_OVERWORLD, seed);

        Pos spawn = getSpawn(&g);
        // printf("pos %d %d\n", spawn.x, spawn.z);

        Pos shipwreck;
        // printf("region %d %d\n", lmao(spawn.x), lmao(spawn.z));
        getStructurePos(Shipwreck, mc, seed, toRegXZ(spawn.x), toRegXZ(spawn.z), &shipwreck);
        if (!isViableStructurePos(Shipwreck, &g, shipwreck.x, shipwreck.z, 0))
            continue;

        int shipwreckProximity = 32;
        if (shipwreck.x > spawn.x + shipwreckProximity ||
            shipwreck.x < spawn.x - shipwreckProximity ||
            shipwreck.z > spawn.z + shipwreckProximity ||
            shipwreck.z < spawn.z - shipwreckProximity)
        {
            continue;
        }

        applySeed(&g, DIM_NETHER, seed);

        Pos bastion;
        int bastionProximity = 128;
        int isBastionNearby = 0;
        for (int i = 0; i < 4; i++)
        {
            int regx = (i % 2) - 1;
            int regz = ((i + 2) / 2) - 2;
            // printf("%d %d\n", regx, regz);

            if (!getStructurePos(Bastion, mc, seed, regx, regz, &bastion))
                continue;
            if (!isViableStructurePos(Bastion, &g, bastion.x, bastion.z, 0))
                continue;

            if (
                bastion.x > bastionProximity ||
                bastion.x < bastionProximity * -1 ||
                bastion.z > bastionProximity ||
                bastion.z < bastionProximity * -1
                //
            )
            {
                continue;
            }

            isBastionNearby = 1;
            break;
        }
        if (!isBastionNearby)
            continue;

        Pos fortress;
        int fortressProximity = 256;
        int isFortressNearby = 0;
        for (int i = 0; i < 4; i++)
        {
            int regx = (i % 2) - 1;
            int regz = ((i + 2) / 2) - 2;
            // printf("%d %d\n", regx, regz);

            if (!getStructurePos(Fortress, mc, seed, regx, regz, &fortress))
                continue;
            if (!isViableStructurePos(Fortress, &g, fortress.x, fortress.z, 0))
                continue;

            if (
                fortress.x > fortressProximity ||
                fortress.x < fortressProximity * -1 ||
                fortress.z > fortressProximity ||
                fortress.z < fortressProximity * -1
                //
            )
            {
                continue;
            }

            isFortressNearby = 1;
            break;
        }
        if (!isFortressNearby)
            continue;

        // printf("bastion %d\n", isBastionNearby);
        printf("%" PRId64 "\n", (int64_t)seed);

        return 0;
    }
}
