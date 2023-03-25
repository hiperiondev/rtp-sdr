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
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "rtp_sdr_iq.h"
#include "rtp_util.h"

#define DEFAULT_HOST     "127.0.0.1"
#define TX_DEFAULT_PORT  (5002)
#define RX_DEFAULT_PORT  (5003)
#define TX_DEFAULT_RATE  (48000) // 48 kHz
#define RX_DEFAULT_RATE  (48000) // 48 kHz
#define DEFAULT_DURATION (20)    // 2 0ms
#define DEFAULT_TYPE     (16)    // 16 bits

static sig_atomic_t shutdown_flag = false;

char exit_signal[33][17] = {
        "NOSIGNAL",
        "SIGHUP",
        "SIGINT",
        "SIGQUIT",
        "SIGILL",
        "SIGTRAP",
        "SIGABRT",
        "SIGIOT",
        "SIGBUS",
        "SIGFPE",
        "SIGKILL",
        "SIGUSR1",
        "SIGSEGV",
        "SIGUSR2",
        "SIGPIPE",
        "SIGALRM",
        "SIGTERM",
        "SIGSTKFLT",
        "SIGCHLD",
        "SIGCONT",
        "SIGSTOP",
        "SIGTSTP",
        "SIGTTIN",
        "SIGTTOU",
        "SIGURG",
        "SIGXCPU",
        "SIGXFSZ",
        "SIGVTALRM",
        "SIGPROF",
        "SIGWINCH",
        "SIGIO",
        "SIGPWR",
        "SIGSYS/SIGUNUSED",
};

/** Command line arguments. */
static struct option opts_long[] = {
        { "host",     1, NULL, 'h' },
        { "txport",   1, NULL, 'p' },
        { "rxport",   1, NULL, 'o' },
        { "txrate",   1, NULL, 'x' },
        { "rxrate",   1, NULL, 'r' },
        { "duration", 1, NULL, 'd' },
        { "txtype",   1, NULL, 't' },
        { "rxtype",   1, NULL, 'y' },
        { "only",     1, NULL, 'n' },
        { "help",     0, NULL, 'H' },
        { NULL,       0, NULL, 0   },
};

static void signal_cb(int signal) {
    (void) signal;
    shutdown_flag = true;
    fprintf(stderr, "Caught signal - Terminating 0x%x/%d(%s)\n", signal, signal, exit_signal[signal]);
    exit(1);
}

void* rcp_iq_transmit_handler(void *arg) {
    session_iq_t *session = (session_iq_t*) arg;

    if (rtp_socket_open_send(&((*session)->tx_socket), (*session)->host, (*session)->tx_port, NULL) == RTP_ERROR) {
        perror("TX SOCKET error");
        exit(2);
    }

    while (1) {
        if (rtp_sdr_rbuf_empty(&((*session)->tx_iq_buffer)))
            continue;

        rcp_iq_transmit(session);
    }

    return NULL;
}

void* rcp_iq_receive_handler(void *arg) {
    FILE *rxptr;
    session_iq_t *session = (session_iq_t*) arg;

    if (rtp_socket_open_recv(&((*session)->rx_socket), (*session)->host, (*session)->tx_port, NULL) == RTP_ERROR) {
        perror("RX SOCKET error");
        exit(2);
    }

    char filename[254];
    sprintf(filename, "test_%d.bin", (*session)->rx_port);
    rxptr = fopen(filename, "wb");

    while (1) {
        if (rcp_iq_receive(session) == RTP_SDR_ERROR)
            continue;

        if (rtp_sdr_rbuf_empty(&((*session)->rx_iq_buffer)))
            continue;

        fwrite((*session)->rx_iq_buffer, sizeof(iq_t), 1, rxptr);
        fflush(rxptr);
    }

    fclose(rxptr);

    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *txptr = NULL;
    iq_t data;
    int status = 0, only = 0;
    uint32_t tx_port = TX_DEFAULT_PORT;
    uint32_t rx_port = RX_DEFAULT_PORT;
    uint32_t txrate = TX_DEFAULT_RATE;
    uint32_t rxrate = RX_DEFAULT_RATE;
    uint32_t duration = DEFAULT_DURATION;
    uint8_t txtype = DEFAULT_TYPE;
    uint8_t rxtype = DEFAULT_TYPE;
    char host[256];
    session_iq_t session;
    iq_t tx_buff[RTP_PACKET_LENGTH], rx_buff[RTP_PACKET_LENGTH];
    pthread_t rcp_iq_transmit_handler_id, rcp_iq_receive_handler_id;

    snprintf(host, sizeof(host), "%s", DEFAULT_HOST);

    while (1) {
        int c = getopt_long(argc, argv, "h:p:o:x:r:d:t:y:n:H", opts_long, &status);
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
                printf("  -p, --txport      RTP transmit port, e.g. --port=%d\n", TX_DEFAULT_PORT);
                printf("  -o, --rxport      RTP receive port, e.g. --port=%d\n", RX_DEFAULT_PORT);
                printf("  -x, --txrate      TX sample rate in Hz(48000, 96000, 192000, 384000, 768000 or 1536000), e.g. --rate=%d\n", TX_DEFAULT_RATE);
                printf("  -r, --rxrate      RX sample rate in Hz(48000, 96000, 192000, 384000, 768000 or 1536000), e.g. --rate=%d\n", RX_DEFAULT_RATE);
                printf("  -t, --txtype      TX RTP data type (8, 16, 24 or 32 bits), e.g. --txtype=%d\n", DEFAULT_TYPE);
                printf("  -y, --rxtype      RX RTP data type (8, 16, 24 or 32 bits), e.g. --rxtype=%d\n", DEFAULT_TYPE);
                printf("  -n, --only        0: tx/tx, 1: only tx, 2: only rx e.g. --type=0");
                printf("  -d, --duration    Frame duration in ms (only transmission), e.g. --duration=%d\n", DEFAULT_DURATION);
                printf("\n");
                exit(1);

            case 'h':
                snprintf(host, sizeof(host), "%s", optarg);
                printf("Host set to %s\n", host);
                break;

            case 'p':
                tx_port = strtoul(optarg, NULL, 0);
                printf("TX Port set to %d\n", tx_port);
                break;

            case 'o':
                rx_port = strtoul(optarg, NULL, 0);
                printf("RX Port set to %d\n", rx_port);
                break;

            case 'x':
                txrate = strtoul(optarg, NULL, 0);
                printf("TX sample rate set to %d Hz\n", txrate);
                break;

            case 'r':
                rxrate = strtoul(optarg, NULL, 0);
                printf("RX sample rate set to %d Hz\n", rxrate);
                break;

            case 't':
                txtype = strtoul(optarg, NULL, 0);
                printf("TX Data type set to %d bits\n", txtype);
                break;

            case 'y':
                rxtype = strtoul(optarg, NULL, 0);
                printf("RX Data type set to %d bits\n", rxtype);
                break;
            case 'n':
                only = strtoul(optarg, NULL, 0);
                printf("default only: %d\n", only);
                break;

            case 'd':
                duration = strtoul(optarg, NULL, 0);
                printf("Frame duration set to %d ms\n", duration);
                break;
        }
    }

    if (only < 0 || only > 2) {
        fprintf(stderr, "--- only unknown value ---\n");
        status = 1;
    }

    if (!(rxrate == SR_48K || rxrate == SR_96K || rxrate == SR_192K || rxrate == SR_384K || rxrate == SR_768K || rxrate == SR_1536K)) {
        fprintf(stderr, "RX sample rate must be 48000, 96000, 192000, 384000, 768000 or 1536000\n");
        status = 1;
    }

    if (!(txrate == SR_48K || txrate == SR_96K || txrate == SR_192K || txrate == SR_384K || txrate == SR_768K || txrate == SR_1536K)) {
        fprintf(stderr, "TX sample rate must be 48000, 96000, 192000, 384000, 768000 or 1536000\n");
        status = 1;
    }

    if (!(duration == 5 || duration == 10 || duration == 20 || duration == 40 || duration == 60)) {
        fprintf(stderr, "Frame duration must be 5, 10, 20, 40, or 60\n");
        status = 1;
    }

    if (!(txtype == 8 || txtype == 16 || txtype == 24 || txtype == 32 || rxtype == 8 || rxtype == 16 || rxtype == 24 || rxtype == 32)) {
        fprintf(stderr, "Data type must be 8, 16. 24 or 32 bits\n");
        status = 1;
    }
    else {
        txtype = (txtype / 8) + 96;
        rxtype = (rxtype / 8) + 96;
    }

    if (status)
        exit(EXIT_FAILURE);

    // Capture SIGINT/SIGTERM and set shutdown_flag for cleanup
    struct sigaction sighandler;
    sigemptyset(&sighandler.sa_mask);
    sighandler.sa_handler = signal_cb;
    sighandler.sa_flags = 0;

    sigaction(SIGINT, &sighandler, NULL);
    sigaction(SIGTERM, &sighandler, NULL);

    ////////////////////////////////////////////

    session = malloc(sizeof(struct session_iq_s));
    rcp_iq_init(&session, txtype, rxtype, txrate, rxrate, duration, host, tx_port, rx_port, 0, tx_buff, rx_buff, RTP_PACKET_LENGTH, 1, 1);
    (*session).tx_enabled = true;
    printf("-- START --\n");

    PRINT_SESION((&session));

    if (only == 0 || only == 1) {
        pthread_create(
                &rcp_iq_transmit_handler_id,
                NULL,
                &rcp_iq_transmit_handler,
                (void*) &session
                );
       //pthread_detach(rcp_iq_transmit_handler_id);
    }

    if (only == 0 || only == 2) {
        pthread_create(
                &rcp_iq_receive_handler_id,
                NULL,
                &rcp_iq_receive_handler,
                (void*) &session
                );
        //pthread_detach(rcp_iq_receive_handler_id);
    }

    if (only == 0 || only == 1) {
        if ((txptr = fopen("test.tx", "rb")) == NULL) {
            printf("--- ERROR: can't open file ---\n");
            rcp_iq_deinit(&session);
            free(session);
            exit(2);
        }

        while (fread(&data, sizeof(iq_t), 1, txptr) != EOF)
            rtp_sdr_rbuf_put(&(session->tx_iq_buffer), data);
        fclose(txptr);
    }

    while (1) {
        sleep(1);
    }

    ////////////////////////////////////////////

    printf("Shutting down\n");
    rcp_iq_deinit(&session);
    free(session);

    return 0;
}
