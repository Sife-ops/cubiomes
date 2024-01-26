#include "finders.h"
#include "generator.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/random.h>

#define OUTPUTLOG 0
#define OUTPUTPPM 0

int toRegXZ(int x)
{
    if (x < 0)
    {
        return -1;
    }
    return 0;
}

void drawSquare(int *b, int x, int z, int color)
{
    for (int i = 0; i < 15; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            int sup = 256 * (z - 7 + i) + x - 7 + j;
            if (sup < 256 * 256 && sup >= 0)
            {
                b[sup] = color;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    FILE *log;
    if (OUTPUTLOG > 0)
    {
        log = fopen("dev.log", "a+");
        fprintf(log, "generating god seed\n");
        fclose(log);
    }

    int mc = MC_1_16_1;

    Generator generator_ow;
    setupGenerator(&generator_ow, mc, 0);

    Generator generator_n;
    setupGenerator(&generator_n, mc, 0);

    uint64_t seed;
    for (;;)
    {
        getrandom(&seed, 8, 0);
        if (OUTPUTLOG > 0)
        {
            log = fopen("dev.log", "a+");
            fprintf(log, "filtering seed %" PRId64 "\n", (int64_t)seed);
            fclose(log);
        }

        applySeed(&generator_ow, DIM_OVERWORLD, seed);

        Pos spawn = getSpawn(&generator_ow);
        // printf("spawn %d %d\n", spawn.x, spawn.z);

        Pos shipwreck;
        // printf("region %d %d\n", lmao(spawn.x), lmao(spawn.z));
        getStructurePos(Shipwreck, mc, seed, toRegXZ(spawn.x), toRegXZ(spawn.z), &shipwreck);
        if (!isViableStructurePos(Shipwreck, &generator_ow, shipwreck.x, shipwreck.z, 0))
            continue;

        int shipwreckProximity = 32;
        if (shipwreck.x > spawn.x + shipwreckProximity ||
            shipwreck.x < spawn.x - shipwreckProximity ||
            shipwreck.z > spawn.z + shipwreckProximity ||
            shipwreck.z < spawn.z - shipwreckProximity)
        {
            continue;
        }

        applySeed(&generator_n, DIM_NETHER, seed);

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

            isBastionNearby = 1;
            break;
        }
        if (!isBastionNearby)
            continue;

        Pos fortress;
        int fortressProximity = 128;
        int isFortressNearby = 0;
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

            isFortressNearby = 1;
            break;
        }
        if (!isFortressNearby)
        {
            continue;
        }

        if (OUTPUTPPM < 1)
        {
            goto End;
        }

        {
            Range r;
            r.scale = 1;
            r.x = spawn.x - 128, r.z = spawn.z - 128; // position (x,z)
            r.sx = 256, r.sz = 256;                   // size (width,height)
            r.y = 15, r.sy = 1;

            int *biomeIds = allocCache(&generator_ow, r);
            genBiomes(&generator_ow, biomeIds, r);

            int avx = abs(spawn.x - shipwreck.x);
            int avz = abs(spawn.z - shipwreck.z);
            int osx;
            if (spawn.x > shipwreck.x)
            {
                osx = 128 - avx;
            }
            else
            {
                osx = 128 + avx;
            }
            int osz;
            if (spawn.z > shipwreck.z)
            {
                osz = 128 - avz;
            }
            else
            {
                osz = 128 + avz;
            }
            drawSquare(biomeIds, osx, osz, crimson_forest);

            int pix4cell = 4;
            int imgWidth = pix4cell * r.sx, imgHeight = pix4cell * r.sz;
            unsigned char biomeColors[256][3];
            initBiomeColors(biomeColors);
            unsigned char *rgb = (unsigned char *)malloc(3 * imgWidth * imgHeight);
            biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);

            char previewPath[strlen(argv[1]) + strlen("/preview/overworld:.ppm") + 20];
            sprintf(previewPath, "%s/preview/overworld:%" PRId64 ".ppm", argv[1], (int64_t)seed);
            savePPM(previewPath, rgb, imgWidth, imgHeight);

            free(biomeIds);
            free(rgb);
        }

        {
            Range r;
            r.scale = 1;
            r.x = spawn.x - 128, r.z = spawn.z - 128; // position (x,z)
            r.sx = 256, r.sz = 256;                   // size (width,height)
            r.y = 15, r.sy = 1;

            int *biomeIds = allocCache(&generator_n, r);
            genBiomes(&generator_n, biomeIds, r);
            if (OUTPUTLOG > 0)
            {
                log = fopen("dev.log", "a+");
                fprintf(log, "generated biomes: nether\n");
                fclose(log);
            }

            drawSquare(biomeIds, 128 + bastion.x, 128 + bastion.z, beach);
            if (OUTPUTLOG > 0)
            {
                log = fopen("dev.log", "a+");
                fprintf(log, "located bastion\n");
                fclose(log);
            }

            if (
                fortress.x <= 128 &&
                fortress.z <= 128 &&
                fortress.x >= -128 &&
                fortress.z >= -128
                //
            )
            {
                drawSquare(biomeIds, 128 + fortress.z, 128 + fortress.z, warm_ocean);
                if (OUTPUTLOG > 0)
                {
                    log = fopen("dev.log", "a+");
                    fprintf(log, "located fortress\n");
                    fclose(log);
                }
            }

            int pix4cell = 4;
            int imgWidth = pix4cell * r.sx, imgHeight = pix4cell * r.sz;
            unsigned char biomeColors[256][3];
            initBiomeColors(biomeColors);
            unsigned char *rgb = (unsigned char *)malloc(3 * imgWidth * imgHeight);
            biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);

            char previewPath[strlen(argv[1]) + strlen("/preview/nether:.ppm") + 20];
            sprintf(previewPath, "%s/preview/nether:%" PRId64 ".ppm", argv[1], (int64_t)seed);
            savePPM(previewPath, rgb, imgWidth, imgHeight);

            free(biomeIds);
            free(rgb);
        }

    End:
        printf("%" PRId64 ":%d,%d:%d,%d", (int64_t)seed, spawn.x, spawn.z, shipwreck.x, shipwreck.z);
        return 0;
    }
}
