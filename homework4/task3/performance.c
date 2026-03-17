#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <immintrin.h>

double getTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

typedef struct {
    int width;
    int height;
    int maxval;
    unsigned char* data;
} Image;

void skip_comments(FILE* f) {
    int c;
    while ((c = fgetc(f)) == '#') {
        while (fgetc(f) != '\n');
    }
    ungetc(c, f);
}

Image read_ppm(const char* filename) {
    Image img;
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Error opening file.\n");
        exit(1);
    }

    char format[3];
    skip_comments(f);
    fscanf(f, "%2s", format);     // Read "P6"
    skip_comments(f);
    fscanf(f, "%d %d", &img.width, &img.height);
    skip_comments(f);
    fscanf(f, "%d", &img.maxval);
    fgetc(f);                     // Skip one whitespace after maxval

    int size = img.width * img.height * 3;
    img.data = (unsigned char*)malloc(size);

    fread(img.data, 1, size, f);  // Read pixel data
    fclose(f);
    return img;
}

void write_ppm(const char* filename, Image* img) {
    FILE* f = fopen(filename, "wb");
    fprintf(f, "P6\n%d %d\n%d\n", img->width, img->height, img->maxval);
    fwrite(img->data, 1, img->width * img->height * 3, f);
    fclose(f);
}

//============================================================
//                SCALAR VERSION
//============================================================
void grayscale_scalar(Image* in, Image* out) {
    int N = in->width * in->height;
    unsigned char* src = in->data;
    unsigned char* dst = out->data;

    for (int i = 0; i < N; i++) {
        unsigned char R = src[3*i+0];
        unsigned char G = src[3*i+1];
        unsigned char B = src[3*i+2];

        unsigned char gray =
            (unsigned char)(0.299f*R + 0.587f*G + 0.114f*B);

        dst[3*i+0] = gray;
        dst[3*i+1] = gray;
        dst[3*i+2] = gray;
    }
}

//============================================================
//                SIMD VERSION
//============================================================
// Processes 4 pixels at once
void grayscale_simd(Image* in, Image* out) {
    int N = in->width * in->height;
    unsigned char* src = in->data;
    unsigned char* dst = out->data;

    const __m128 wR = _mm_set1_ps(0.299f);
    const __m128 wG = _mm_set1_ps(0.587f);
    const __m128 wB = _mm_set1_ps(0.114f);

    int i = 0;
    for (; i+4 <= N; i += 4) {
        float R[4], G[4], B[4];

        for (int k = 0; k < 4; k++) {
            R[k] = src[3*(i+k)+0];
            G[k] = src[3*(i+k)+1];
            B[k] = src[3*(i+k)+2];
        }

        __m128 vR = _mm_loadu_ps(R);
        __m128 vG = _mm_loadu_ps(G);
        __m128 vB = _mm_loadu_ps(B);

        __m128 gray = _mm_add_ps(
                         _mm_add_ps(_mm_mul_ps(vR, wR),
                                    _mm_mul_ps(vG, wG)),
                         _mm_mul_ps(vB, wB));

        float g[4];
        _mm_storeu_ps(g, gray);

        for (int k = 0; k < 4; k++) {
            unsigned char val = (unsigned char) g[k];
            dst[3*(i+k)+0] = val;
            dst[3*(i+k)+1] = val;
            dst[3*(i+k)+2] = val;
        }
    }

    // leftover
    for (; i < N; i++) {
        unsigned char Rval = src[3*i+0];
        unsigned char Gval = src[3*i+1];
        unsigned char Bval = src[3*i+2];
        unsigned char gray =
            (unsigned char)(0.299f*Rval + 0.587f*Gval + 0.114f*Bval);
        dst[3*i+0] = dst[3*i+1] = dst[3*i+2] = gray;
    }
}

//============================================================
//                MULTITHREADED
//============================================================
typedef struct {
    Image* in;
    Image* out;
    int start;
    int end;
} thread_arg_t;

void* worker_scalar(void* arg) {
    thread_arg_t* T = (thread_arg_t*)arg;
    unsigned char* src = T->in->data;
    unsigned char* dst = T->out->data;

    for (int i = T->start; i < T->end; i++) {
        unsigned char R = src[3*i+0];
        unsigned char G = src[3*i+1];
        unsigned char B = src[3*i+2];

        unsigned char gray =
            (unsigned char)(0.299f*R + 0.587f*G + 0.114f*B);

        dst[3*i+0] = dst[3*i+1] = dst[3*i+2] = gray;
    }
    return NULL;
}

void grayscale_threads(Image* in, Image* out, int nthreads) {
    int N = in->width * in->height;
    pthread_t threads[nthreads];
    thread_arg_t args[nthreads];

    int chunk = N / nthreads;

    for (int t = 0; t < nthreads; t++) {
        args[t].in = in;
        args[t].out = out;
        args[t].start = t * chunk;
        args[t].end = (t == nthreads-1) ? N : (t+1)*chunk;
        pthread_create(&threads[t], NULL, worker_scalar, &args[t]);
    }
    for (int t = 0; t < nthreads; t++)
        pthread_join(threads[t], NULL);
}

//============================================================
//                MULTITHREADED + SIMD
//============================================================
// Reuse SIMD code inside threads
void* worker_simd(void* arg) {
    thread_arg_t* T = (thread_arg_t*)arg;

    int N = T->end - T->start;
    int base = T->start;

    unsigned char* src = T->in->data;
    unsigned char* dst = T->out->data;

    const __m128 wR = _mm_set1_ps(0.299f);
    const __m128 wG = _mm_set1_ps(0.587f);
    const __m128 wB = _mm_set1_ps(0.114f);

    int i = 0;
    for (; i+4 <= N; i += 4) {
        int idx[4] = {base+i, base+i+1, base+i+2, base+i+3};
        float R[4], G[4], B[4];

        for (int k = 0; k < 4; k++) {
            R[k] = src[3*idx[k]+0];
            G[k] = src[3*idx[k]+1];
            B[k] = src[3*idx[k]+2];
        }

        __m128 vR = _mm_loadu_ps(R);
        __m128 vG = _mm_loadu_ps(G);
        __m128 vB = _mm_loadu_ps(B);
        __m128 gray = _mm_add_ps(
                        _mm_add_ps(_mm_mul_ps(vR, wR),
                                   _mm_mul_ps(vG, wG)),
                        _mm_mul_ps(vB, wB));

        float g[4];
        _mm_storeu_ps(g, gray);

        for (int k = 0; k < 4; k++) {
            unsigned char val = (unsigned char) g[k];
            int pos = idx[k];
            dst[3*pos+0] = dst[3*pos+1] = dst[3*pos+2] = val;
        }
    }

    // leftover
    for (; i < N; i++) {
        int pos = base + i;
        unsigned char Rv = src[3*pos+0];
        unsigned char Gv = src[3*pos+1];
        unsigned char Bv = src[3*pos+2];
        unsigned char gray =
            (unsigned char)(0.299f*Rv + 0.587f*Gv + 0.114f*Bv);
        dst[3*pos+0] = dst[3*pos+1] = dst[3*pos+2] = gray;
    }

    return NULL;
}

void grayscale_threads_simd(Image* in, Image* out, int nthreads) {
    int N = in->width * in->height;
    pthread_t threads[nthreads];
    thread_arg_t args[nthreads];

    int chunk = N / nthreads;

    for (int t = 0; t < nthreads; t++) {
        args[t].in = in;
        args[t].out = out;
        args[t].start = t * chunk;
        args[t].end = (t == nthreads-1) ? N : (t+1)*chunk;
        pthread_create(&threads[t], NULL, worker_simd, &args[t]);
    }
    for (int t = 0; t < nthreads; t++)
        pthread_join(threads[t], NULL);
}

//============================================================
//                        MAIN
//============================================================
int main() {
    Image in = read_ppm("input.ppm");

    Image out_scalar = in;
    out_scalar.data = (unsigned char*)malloc(in.width*in.height*3);

    Image out_simd = in;
    out_simd.data = (unsigned char*)malloc(in.width*in.height*3);

    Image out_mt = in;
    out_mt.data = (unsigned char*)malloc(in.width*in.height*3);

    Image out_mt_simd = in;
    out_mt_simd.data = (unsigned char*)malloc(in.width*in.height*3);

    printf("Image size: %d x %d\n", in.width, in.height);
    int threads = 4;
    printf("Threads: %d\n\n", threads);

    double start = getTime();
    grayscale_scalar(&in, &out_scalar);
    double end = getTime();
    double scalar_t = end - start;

    start = getTime();
    grayscale_simd(&in, &out_simd);
    end = getTime();
    double simd_t = end - start;

    start = getTime();
    grayscale_threads(&in, &out_mt, threads);
    end = getTime();
    double mt_t = end - start;

    start = getTime();
    grayscale_threads_simd(&in, &out_mt_simd, threads);
    end = getTime();
    double mt_simd_t = end - start;

    printf("Scalar:           %.6f sec\n", scalar_t);
    printf("SIMD:             %.6f sec\n", simd_t);
    printf("Multithread:      %.6f sec\n", mt_t);
    printf("MT + SIMD:        %.6f sec\n", mt_simd_t);

    // Save output
    write_ppm("gray_output.ppm", &out_mt_simd);
    printf("\nOutput image: gray_output.ppm\n");

    return 0;
}
