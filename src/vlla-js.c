#include <unistd.h>

#include "duktape.h"
#include "vlla.h"

#define WIDTH 60
#define HEIGHT 32

VLLA* vlla;

uint8_t r = 255;
uint8_t g = 0;
uint8_t b = 0;

void cleanup() {
    vlla_close(vlla);
}

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
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

    vlla = vlla_init("/dev/ttyACM0", "/dev/ttyACM1");
    for(int i=0; i < WIDTH*HEIGHT; i++)
        vlla->pixels[i] = 0x0F0F00  ;

    vlla_update(vlla);

    duk_context *ctx = duk_create_heap_default();

    (void) argc; (void) argv;  //suppress warning

    duk_push_global_object(ctx);
    duk_push_c_function(ctx, paint, DUK_VARARGS);
    duk_put_prop_string(ctx, -2, "paint");
    duk_push_c_function(ctx, refresh, DUK_VARARGS);
    duk_put_prop_string(ctx, -2, "refresh");
    duk_pop(ctx);

    duk_eval_file(ctx, script_fn);
    duk_pop(ctx);

    duk_destroy_heap(ctx);
    
    return 0;
}

