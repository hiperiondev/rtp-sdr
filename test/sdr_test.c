/*
 * Copyright 2023 Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/rtp-sdr *
 *
 * This is based on other projects:
 *      IDEA: https://github.com/OpenResearchInstitute/ka9q-sdr (not use any code of this)
 *       RTP: https://github.com/Daxbot/librtp/
 *       FEC: https://github.com/wesen/poc
 *    SOCKET: https://github.com/njh/mast
 *    OTHERS: see individual files
 *
 *    please contact their authors for more information.
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "rtp_sdr_iq.h"

#define DEFAULT_HOST     "127.0.0.1"
#define DEFAULT_PORT     (5002)
#define DEFAULT_RATE     (48000) // 48 kHz
#define DEFAULT_DURATION (20)    // 2 0ms
#define DEFAULT_TYPE     (16)    // 16 bits

static sig_atomic_t shutdown_flag = false;

/** Command line arguments. */
static struct option opts_long[] = {
        { "host",     1, NULL, 'h' },
        { "port",     1, NULL, 'p' },
        { "rate",     1, NULL, 'r' },
        { "duration", 1, NULL, 'd' },
        { "help",     0, NULL, 'H' },
        { "type",     0, NULL, 't' },
        { NULL,       0, NULL, 0   },
};

static void signal_cb(int signal) {
    (void) signal;
    shutdown_flag = true;
}

void* rcp_iq_transmit_handler(void *arg) {
    session_iq_t *session = (session_iq_t*) arg;

    while (1) {
        while ((*session)->tx_enabled) {
            if (rcp_iq_transmit(session) == RTP_SDR_ERROR)
                return NULL;
        }

        usleep(1000);
    }

    return NULL;
}

void* rcp_iq_receive_handler(void *arg) {
    session_iq_t *session = (session_iq_t*) arg;

    while (1) {
            if (rcp_iq_receive(session) == RTP_SDR_ERROR)
                return NULL;
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int error = 0;
    uint32_t port = DEFAULT_PORT;
    uint32_t rate = DEFAULT_RATE;
    uint32_t duration = DEFAULT_DURATION;
    uint8_t type = DEFAULT_TYPE;
    char host[256];

    snprintf(host, sizeof(host), "%s", DEFAULT_HOST);

    while (1) {
        int c = getopt_long(argc, argv, "h:p:r:d:t:H", opts_long, NULL);
        if (c == -1)
            break;

        switch (c) {
            default:
            case 'H':
                printf("Usage: %s [options]\n", argv[0]);
                printf("\n");
                printf("Supported options:\n");
                printf("  -H, --help        Displays this menu\n");
                printf("  -h, --host        RTP transmit host, e.g. --host=%s\n", DEFAULT_HOST);
                printf("  -p, --port        RTP transmit port, e.g. --port=%d\n", DEFAULT_PORT);
                printf("  -r, --rate        Sample rate in Hz(48000, 96000, 192000, 384000, 768000 or 1536000), e.g. --rate=%d\n", DEFAULT_RATE);
                printf("  -t, --type        RTP data type (8, 16, 24 or 32 bits), e.g. --type=%d\n", DEFAULT_TYPE);
                printf("  -d, --duration    Frame duration in ms (only transmission), e.g. --duration=%d\n", DEFAULT_DURATION);
                printf("\n");
                exit(1);

            case 'h':
                snprintf(host, sizeof(host), "%s", optarg);
                printf("Host set to %s\n", host);
                break;

            case 'p':
                port = strtoul(optarg, NULL, 0);
                printf("Port set to %d\n", port);
                break;

            case 'r':
                rate = strtoul(optarg, NULL, 0);
                printf("Sample rate set to %d Hz\n", rate);
                break;

            case 't':
                type = strtoul(optarg, NULL, 0);
                printf("Data type set to %d bits\n", type);
                break;

            case 'd':
                duration = strtoul(optarg, NULL, 0);
                printf("Frame duration set to %d ms\n", duration);
                break;
        }
    }

    // Sanity checks
    if (!(rate == SR_48K || rate == SR_96K || rate == SR_192K || rate == SR_384K || rate == SR_768K || rate == SR_1536K)) {
        fprintf(stderr, "Sample rate must be 48000, 96000, 192000, 384000, 768000 or 1536000\n");
        error = 1;
    }

    if (!(duration == 5 || duration == 10 || duration == 20 || duration == 40 || duration == 60)) {
        fprintf(stderr, "Frame duration must be 5, 10, 20, 40, or 60\n");
        error = 1;
    }

    if (!(type == 8 || type == 16 || type == 24 || type == 32)) {
        fprintf(stderr, "Data type must be 8, 16. 24 or 32 bits\n");
        error = 1;
    }
    else
        type = (type / 8) + 96;

    if (error)
        exit(EXIT_FAILURE);

    // Capture SIGINT/SIGTERM and set shutdown_flag for cleanup
    struct sigaction sighandler;
    sigemptyset(&sighandler.sa_mask);
    sighandler.sa_handler = signal_cb;
    sighandler.sa_flags = 0;

    sigaction(SIGINT, &sighandler, NULL);
    sigaction(SIGTERM, &sighandler, NULL);

    ////////////////////////////////////////////

    ////////////////////////////////////////////

    printf("Shutting down\n");

    return 0;
}
