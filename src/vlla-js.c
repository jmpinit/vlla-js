#include <signal.h>
#include <unistd.h>

#include "duktape.h"
#include "vlla.h"

#define WIDTH 60
#define HEIGHT 32

VLLA* vlla;

int alive = 1;

uint32_t vlla_time = 0;

uint8_t r = 255;
uint8_t g = 255;
uint8_t b = 255;

void cleanup() {
    vlla_close(vlla);
}

void sig_handler(int signo) {
    if(signo == SIGINT)
        alive = 0;
}

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

int t(duk_context *ctx) {
    duk_push_number(ctx, vlla_time);
    return 1;
}

int palette(duk_context *ctx) {
    int n = duk_get_top(ctx);

    if(n == 3) {
        int nr = duk_to_number(ctx, 0);
        int ng = duk_to_number(ctx, 1);
        int nb = duk_to_number(ctx, 2);

        if(nr >= 0 && ng >= 0 && nb >= 0 && nr < 255 && ng < 255 && nb < 255) {
            r = nr;
            g = ng;
            b = nb;
        }
    } else if(n == 1) {
        int c = duk_to_number(ctx, 0);

        if(c >= 0 && c < 255) {
            r = c;
            g = c;
            b = c;
        }
    }

    return 0;
}

int paint(duk_context *ctx) {
    int n = duk_get_top(ctx);

    if(n == 2) {
        int x = duk_to_number(ctx, 0);
        int y = duk_to_number(ctx, 1);

        if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            vlla->pixels[y*WIDTH+x] = rgb(r, g, b);
        }
    }

    return 0;
}

int refresh(duk_context *ctx) {
    vlla_update(vlla);
    return 0;
}

int main(int argc, char *argv[]) {
    // parameter validation
    if(argc != 2) {
        printf("usage: vlla-js <js file>\n");
        exit(1);
    }

    if(access(argv[1], F_OK) < 0) {
        printf("file does not exist.\n");
        exit(1);
    }

    char* script_fn = argv[1];

    atexit(cleanup);

    if(signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("\ncan't catch SIGINT\n");
        exit(1);
    }

    vlla = vlla_init("/dev/ttyACM0", "/dev/ttyACM1");
    for(int i=0; i < WIDTH*HEIGHT; i++)
        vlla->pixels[i] = 0;

    vlla_update(vlla);

    duk_context *ctx = duk_create_heap_default();

    (void) argc; (void) argv;  //suppress warning

    duk_push_global_object(ctx);
    duk_push_c_function(ctx, t, DUK_VARARGS);
    duk_put_prop_string(ctx, -2, "t");
    duk_push_c_function(ctx, palette, DUK_VARARGS);
    duk_put_prop_string(ctx, -2, "palette");
    duk_push_c_function(ctx, paint, DUK_VARARGS);
    duk_put_prop_string(ctx, -2, "paint");
    duk_push_c_function(ctx, refresh, DUK_VARARGS);
    duk_put_prop_string(ctx, -2, "refresh");
    duk_pop(ctx);

    while(alive) {
        duk_eval_file(ctx, script_fn);
        vlla_time++;
        sleep(1);
    }

    duk_pop(ctx);

    duk_destroy_heap(ctx);
    
    return 0;
}

