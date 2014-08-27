#ifndef H_VLLA
#define H_VLLA

// TODO store width and height
typedef struct VLLA {
    int ser1_fd;
    int ser2_fd;
    uint32_t* pixels;
} VLLA;

VLLA* vlla_init(char* ser1, char* ser2);
void vlla_update(VLLA*);
void vlla_close(VLLA*);

#endif
