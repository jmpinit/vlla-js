#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include "vlla.h"
#include "serial.h"

#define FRAMERATE 30.0

#define CHANNEL_COUNT 3

#define WIDTH 60
#define HEIGHT 32
#define PIXEL_COUNT WIDTH*HEIGHT

#define SERIAL_DATA_LEN WIDTH*HEIGHT*CHANNEL_COUNT

#define ROLE_MASTER '*'
#define ROLE_SLAVE '%'

typedef struct {
    // order is part of vlla's protocol
    uint8_t role;
    uint8_t time_l;
    uint8_t time_h;
} SyncInfo;

// local functions
int color_wiring(uint32_t c);
void format_led(uint32_t* pixels, uint8_t* data, int len);
int open_serial(char* portname);

VLLA* vlla_init(char* ser1, char* ser2) {
    // create vlla representation
    VLLA* vlla = calloc(1, sizeof(VLLA));
    vlla->pixels = calloc(PIXEL_COUNT, sizeof(uint32_t));

    // setup communication to actual display
    vlla->ser1_fd = open_serial(ser1);
    vlla->ser2_fd = open_serial(ser2);

    serial_set_interface_attribs(vlla->ser1_fd, B115200, 0);
    serial_set_interface_attribs(vlla->ser2_fd, B115200, 0);

    // TODO ask display for information

    return vlla;
}

void vlla_update(VLLA* vlla) {
    uint16_t synctime = (uint16_t)((1000000.0 / FRAMERATE) * 0.75);

    SyncInfo master_info = {
        ROLE_MASTER,
        synctime & 0xFF,
        synctime >> 8
    };

    //SyncInfo slave_info = { ROLE_SLAVE, 0, 0 };

    uint8_t* led_data_top = calloc(SERIAL_DATA_LEN, sizeof(uint8_t));
    uint8_t* led_data_bottom = calloc(SERIAL_DATA_LEN, sizeof(uint8_t));
    
    format_led(vlla->pixels + PIXEL_COUNT/2, led_data_top, PIXEL_COUNT/2);
    format_led(vlla->pixels, led_data_bottom, PIXEL_COUNT/2);

    // write to master controller
    write(vlla->ser2_fd, &master_info, sizeof(SyncInfo));
    write(vlla->ser2_fd, led_data_top, SERIAL_DATA_LEN); 
    tcdrain(vlla->ser2_fd);

    // write to slave controller
    write(vlla->ser1_fd, &master_info, sizeof(SyncInfo)); 
    write(vlla->ser1_fd, led_data_bottom, SERIAL_DATA_LEN); 
    tcdrain(vlla->ser1_fd);
}

void vlla_close(VLLA* vlla) {
    close(vlla->ser1_fd);
    close(vlla->ser2_fd);

    free(vlla->pixels);
    free(vlla);
}

void format_led(uint32_t* pixels, uint8_t* data, int len) {
    int layout = true;

    int x, y, xbegin, xend, xinc, mask;
    int linesPerPin = HEIGHT / 2 / 8;
    int pixel[8];

    int index = 0;
    for (y = 0; y < linesPerPin; y++) {
        if ((y & 1) == (layout ? 0 : 1)) {
            // even numbered rows are left to right
            xbegin = 0;
            xend = WIDTH;
            xinc = 1;
        } else {
            // odd numbered rows are right to left
            xbegin = WIDTH - 1;
            xend = -1;
            xinc = -1;
        }

        for (x = xbegin; x != xend; x += xinc) {
            for (int i=0; i < 8; i++) {
                // fetch 8 pixels from the image, 1 for each pin
                int px_index = x + (y + linesPerPin * i) * WIDTH;
                if(px_index < len) {
                    pixel[i] = pixels[px_index];
                    pixel[i] = color_wiring(pixel[i]);
                } else {
                    printf("out of bounds at %d\n", px_index);
                }
            }

            // convert 8 pixels to 24 bytes
            for(mask = 0x800000; mask != 0; mask >>= 1) {
                uint8_t b = 0;
                for (int i=0; i < 8; i++) {
                    if ((pixel[i] & mask) != 0) b |= (1 << i);
                }

                data[index++] = b;
            }
        }
    } 
}

int color_wiring(uint32_t d) {
    int32_t r_h = (0xf00000 & d);
    int32_t r_l = (0xf0000 & d) >> 4*2;
    int32_t r = r_h | r_l;

    int32_t g_h = (0xf000 & d);
    int32_t g_l = (0xf00 & d) >> 4*2;
    int32_t g = g_h | g_l;

    int32_t b_h = (0xf0 & d) << 4*3;
    int32_t b_l = (0xf & d) << 4;
    int32_t b = b_h | b_l;

    int32_t c = r | g | b;

    return ((c & 0xFF0000) >> 8) | ((c & 0x00FF00) << 8) | (c & 0x0000FF); // GRB - most common wiring
}

int open_serial(char* portname) {
    int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);

    if (fd < 0) {
        printf("error opening %s", portname);
        exit(1);
    }

    return fd;
}
