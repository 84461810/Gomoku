#include "stats.h"

#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include <string.h>
#include "input.h"
#include "utils.h"
#include "md5.h"

/**
 * File format:
 * (little-endian)
 * checksum (16 bytes),
 * wins (4 bytes), lose (4 bytes)
 */

typedef struct {
    char cksum[MD5_CRYPT_LEN];
    int wins;
    int loses;
} stats_format_t;

#define STATS_DATA_LEN 8
#define stats_format_data_ref(stats) (((char *)(stats)) + MD5_CRYPT_LEN)

static char *filename = "gomoku.stats", *backupFilename = "gomoku.stats.bak";
static int wins = 0, loses = 0;
static int loaded = 0;

static int readIntLE(char *buf) {
    int ret = 0;
    for (int j = sizeof(int) - 1; j >= 0; j--) {
        ret = (ret << 8) | buf[j];
    }
    return ret;
}

static void writeIntLE(char *buf, int i) {
    unsigned char mask = 255;
    for (int j = 0; j < sizeof(int); j++) {
        buf[j] = (char) (i & mask);
        i = i >> 8;
    }
}

// return -1 if the file is corrupted / does not exist
static int readStatsFile(char *fname, stats_format_t *stats) {
    // open
    FILE *file = fopen(fname, "rb");
    if (!file) {
        return -1;
    }
    // read
    size_t ret = fread(stats->cksum, 1, MD5_CRYPT_LEN, file);
    if (ret != MD5_CRYPT_LEN) {
        utils_fatalError("FATAL: Fail to read stats files.\nENTER to exit...", 1);
    }
    char buf[2 * sizeof(int)];
    ret = fread(buf, 1, 2 * sizeof(int), file);
    if (ret != 2 * sizeof(int)) {
        utils_fatalError("FATAL: Fail to read stats files.\nENTER to exit...", 1);
    }
    stats->wins = readIntLE(buf);
    stats->loses = readIntLE(buf + sizeof(int));
    // check corruption
    char cksum[MD5_CRYPT_LEN];
    md5(cksum, buf, 2 * sizeof(int));
    for (int i = 0; i < 2 * sizeof(int); i++) {
        if (cksum[i] != stats->cksum[i]) {
            // corrupted
            return -1;
        }
    }
    // close
    if (fclose(file)) {
        utils_fatalError("FATAL: Fail to close stats files.\nENTER to exit...", 1);
    }
    return 0;
}

static void writeStatsFile(char *fname, stats_format_t *stats);
// load stats into global variable wins & loses
// create new stats files if not exist
static void loadStats() {
    static int loaded = 0;
    if (loaded) {
        // latest stats is in memory
        // do nothing
        return;
    }
    // try to load from files
    // do recovery if necessary
    stats_format_t stats;
    if (readStatsFile(backupFilename, &stats)) {
        if (readStatsFile(filename, &stats)) {
            // both files are corrupted / do not exist
            // create new copies of stats file
            stats.wins = 0;
            stats.loses = 0;
            md5(stats.cksum, stats_format_data_ref(&stats), STATS_DATA_LEN);
            writeStatsFile(backupFilename, &stats);
            writeStatsFile(filename, &stats);
        } else {
            // backup file is not available
            // while the original file is available
            // undo changes of backup file
            writeStatsFile(backupFilename, &stats);
        }
    } else {
        // backup file is available
        // recover the original file
        writeStatsFile(filename, &stats);
    }
    // load successfully
    wins = stats.wins;
    loses = stats.loses;
    loaded = 1;
}

static void statsFormatInit(stats_format_t *stats) {
    stats->wins = wins;
    stats->loses = loses;
    // checksum
    char *data = stats_format_data_ref(stats);
    md5(stats->cksum, data, STATS_DATA_LEN);
}

static void writeStatsFile(char *fname, stats_format_t *stats) {
    // open
    FILE *file = fopen(fname, "wb");
    if (!file) {
        utils_fatalError("FATAL: Fail to open stats files.\nENTER to exit...", 1);
    }
    // write
    char data[MD5_CRYPT_LEN + STATS_DATA_LEN];
    memcpy(data, &stats->cksum, MD5_CRYPT_LEN);
    writeIntLE(data + MD5_CRYPT_LEN, wins);
    writeIntLE(data + MD5_CRYPT_LEN + sizeof(int), loses);
    size_t ret = fwrite(data, 1, MD5_CRYPT_LEN + STATS_DATA_LEN, file);
    if (ret != MD5_CRYPT_LEN + STATS_DATA_LEN) {
        utils_fatalError("FATAL: Fail to write stats files.\nENTER to exit...", 1);
    }
    // flush
    if (fflush(file)) {
        utils_fatalError("FATAL: Fail to flush stats files.\nENTER to exit...", 1);
    }
    // close
    if (fclose(file)) {
        utils_fatalError("FATAL: Fail to close stats files.\nENTER to exit...", 1);
    }
}

static void storeStats() {
    // prepare file content
    stats_format_t stats;
    statsFormatInit(&stats);
    // first, store backup
    writeStatsFile(backupFilename, &stats);
    // then, update the original copy
    writeStatsFile(filename, &stats);
}

void stats_display() {
    system("cls");
    loadStats();
    printf(""
           "+----------------------------------+\n"
           "|                                  |\n"
           "|           Gomoku Stats           |\n"
           "|                                  |\n"
           "|    Wins: %20d    |\n"
           "|    Loses: %19d    |\n"
           "|                                  |\n"
           "+----------------------------------+\n",
           wins, loses);
    utils_waitEnter("Press ENTER to return to menu...");
}

void stats_addWin() {
    loadStats();
    wins++;
    storeStats();
}

void stats_addLose() {
    loadStats();
    loses++;
    storeStats();
}
