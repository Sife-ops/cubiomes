#include "finders.h"
#include "generator.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/random.h>

#define OUTPUTLOG 0

int toRegXZ(int x)
{
    if (x < 0)
    {
        return -1;
    }
    return 0;
}

int mc = MC_1_16_1;

int main(int argc, char *argv[])
{
    FILE *log;

    uint64_t seed;
    for (;;)
    {
        if (OUTPUTLOG > 0)
        {
            log = fopen("dev.log", "a+");
            fprintf(log, "filtering seed %" PRId64 "\n", (int64_t)seed);
            fclose(log);
        }

        getrandom(&seed, 8, 0);

        Generator generator_ow;
        setupGenerator(&generator_ow, mc, 0);

        Generator generator_n;
        setupGenerator(&generator_n, mc, 0);

        applySeed(&generator_ow, DIM_OVERWORLD, seed);

        Pos spawn = getSpawn(&generator_ow);
        // printf("spawn %d %d\n", spawn.x, spawn.z);

        Pos shipwreck;
        // printf("region %d %d\n", lmao(spawn.x), lmao(spawn.z));
        getStructurePos(Shipwreck, mc, seed, toRegXZ(spawn.x), toRegXZ(spawn.z), &shipwreck);
        if (!isViableStructurePos(Shipwreck, &generator_ow, shipwreck.x, shipwreck.z, 0))
        {
            // return 33;
            continue;
        }

        int shipwreckProximity = 32;
        if (shipwreck.x > spawn.x + shipwreckProximity ||
            shipwreck.x < spawn.x - shipwreckProximity ||
            shipwreck.z > spawn.z + shipwreckProximity ||
            shipwreck.z < spawn.z - shipwreckProximity)
        {
            // return 33;
            continue;
        }

        applySeed(&generator_n, DIM_NETHER, seed);

        Pos bastion;
        int bastionProximity = 96;
        int bastions = 0;
        for (int i = 0; i < 4; i++)
        {
            int regx = (i % 2) - 1;
            int regz = ((i + 2) / 2) - 2;
            // printf("%d %d\n", regx, regz);

            if (!getStructurePos(Bastion, mc, seed, regx, regz, &bastion))
                continue;
            if (!isViableStructurePos(Bastion, &generator_n, bastion.x, bastion.z, 0))
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

            // todo look for multiple bastions
            bastions++;
            break;
        }
        if (bastions < 1)
        {
            // return 33;
            continue;
        }

        Pos fortress;
        int fortressProximity = 96;
        int fortresses = 0;
        for (int i = 0; i < 4; i++)
        {
            int regx = (i % 2) - 1;
            int regz = ((i + 2) / 2) - 2;
            // printf("%d %d\n", regx, regz);

            if (!getStructurePos(Fortress, mc, seed, regx, regz, &fortress))
                continue;
            if (!isViableStructurePos(Fortress, &generator_n, fortress.x, fortress.z, 0))
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

            fortresses++;
            break;
        }
        if (fortresses < 1)
        {
            // return 33;
            continue;
        }

        printf(
            "%" PRId64 ":%d,%d:%d,%d:%d,%d:%d,%d",
            (int64_t)seed,
            spawn.x, spawn.z,
            shipwreck.x, shipwreck.z,
            bastion.x, bastion.z,
            fortress.x, fortress.z);

        return 0;
    }
}
