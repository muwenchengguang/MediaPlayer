#ifndef _LOG_H_
#define _LOG_H_

#ifndef LOGI
#include <stdio.h>
#ifdef LOG_TAG
#define LOGI(format, args...) { \
    printf("I/%s:    ",LOG_TAG);\
    printf(format, ##args);     \
    printf("\n");               \
}
#else
#define LOGI(format, args...) { \
    printf("I/:    ");          \
    printf(format, ##args);     \
    printf("\n");               \
}
#endif
#endif


#ifndef LOGW
#include <stdio.h>
#ifdef LOG_TAG
#define LOGW(format, args...) { \
    printf("W/%s:    ",LOG_TAG);\
    printf(format, ##args);     \
    printf("\n");               \
}
#else
#define LOGW(format, args...) { \
    printf("W/:    ");          \
    printf(format, ##args);     \
    printf("\n");               \
}
#endif
#endif


#ifndef LOGE
#include <stdio.h>
#ifdef LOG_TAG
#define LOGE(format, args...) { \
    printf("E/%s:    ",LOG_TAG);\
    printf(format, ##args);     \
    printf("\n");               \
}
#else
#define LOGE(format, args...) { \
    printf("E/:    ");          \
    printf(format, ##args);     \
    printf("\n");               \
}
#endif
#endif

#endif
