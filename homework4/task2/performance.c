#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <immintrin.h>
#include <cstring>

#define SIZE (256 * 1024 * 1024)
#define THREADS 4

double getTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void fill_buffer(char *buf) {
    const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        " .,!?;:-+=*/()[]{}";

    for (int i = 0; i < SIZE; i++)
        buf[i] = charset[rand() % (sizeof(charset) - 1)];
}


typedef struct {
    char *buf;
    size_t start;
    size_t end;
} thread_arg_t;

void *convert_mt(void *arg) {
    thread_arg_t *arg_ = (thread_arg_t*)arg;

    for (int i = arg_->start; i < arg_->end; i++) {
        char c = arg_->buf[i];
        if (c >= 'a' && c <= 'z')
            arg_->buf[i] = c - 32;
    }
    return NULL;
}


void convert_simd(char *buf, size_t n) {
    __m256i a = _mm256_set1_epi8('a');
    __m256i z = _mm256_set1_epi8('z');
    __m256i diff = _mm256_set1_epi8(32);

    for (int i = 0; i + 32 <= n; i += 32) {
        __m256i v = _mm256_loadu_si256((__m256i*)&buf[i]);

        __m256i is_lower =
            _mm256_and_si256(
                _mm256_cmpgt_epi8(v, _mm256_sub_epi8(a, _mm256_set1_epi8(1))),
                _mm256_cmpgt_epi8(_mm256_add_epi8(z, _mm256_set1_epi8(1)), v)
            );

        __m256i to_upper = _mm256_sub_epi8(v, diff);
        __m256i result = _mm256_blendv_epi8(v, to_upper, is_lower);

        _mm256_storeu_si256((__m256i*)&buf[i], result);
    }

}

void *convert_simd_mt(void *arg) {
    thread_arg_t *arg_ = (thread_arg_t*)arg;
    convert_simd(arg_->buf + arg_->start, arg_->end - arg_->start);
    return NULL;
}


int main(int argc, char **argv) {
    printf("Buffer size: %lu MB\n", SIZE / (1024*1024));
    printf("Threads used: %d\n\n", THREADS);

    char *buf1 = (char*)malloc(SIZE);
    char *buf2 = (char*)malloc(SIZE);
    char *buf3 = (char*)malloc(SIZE);

    fill_buffer(buf1);
    memcpy(buf2, buf1, SIZE);
    memcpy(buf3, buf1, SIZE);

    // ----------------------
    // Multithreading
    // ----------------------
    pthread_t threads[THREADS];
    thread_arg_t args[THREADS];

    double mth_start = getTime();

    int chunk = SIZE / THREADS;
    for (int i = 0; i < THREADS; i++) {
        args[i].buf = buf1;
        args[i].start = i * chunk;
        args[i].end = (i == THREADS - 1) ? SIZE : (i + 1) * chunk;
        pthread_create(&threads[i], NULL, convert_mt, &args[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    double mth_end = getTime();
    printf("Multithreading time:  %.6f sec\n", mth_end - mth_start);

    // ----------------------
    // SIMD
    // ----------------------
    double simd_start = getTime();
    convert_simd(buf2, SIZE);
    double simd_end = getTime();
    printf("SIMD time:  %.6f sec\n", simd_end - simd_start);

    // ----------------------
    // SIMD + Multithreading
    // ----------------------
    double simd_th_start = getTime();

    for (int i = 0; i < THREADS; i++) {
        args[i].buf = buf3;
        args[i].start = i * chunk;
        args[i].end   = (i == THREADS - 1) ? SIZE : (i + 1) * chunk;
        pthread_create(&threads[i], NULL, convert_simd_mt, &args[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    double simd_th_end = getTime();
    printf("SIMD + Multithreading:    %.6f sec\n", simd_th_end - simd_th_start);

    free(buf1);
    free(buf2);
    free(buf3);

    return 0;
}
