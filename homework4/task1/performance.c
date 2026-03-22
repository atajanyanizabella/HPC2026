#include <stdio.h>
#include <cstdlib>
#include <time.h>
#include <pthread.h>
#include <immintrin.h>
#include <stdint.h>

#define DNA_SIZE (256 * 1024 * 1024)
#define THREADS 4

char random_base() {
    int r = rand() % 4;
    char characters[4] = {'A', 'C', 'G', 'T'};
    return characters[r];
    /*switch (r) {
        case 0: return 'A';
        case 1: return 'C';
        case 2: return 'G';
        case 3: return 'T';
    }
    return 'A';*/
}

double getTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

char* gl_buffer;
long gl_countA = 0, gl_countC = 0, gl_countG = 0, gl_countT = 0;
pthread_spinlock_t lock;

// multithreading struct argument 
struct chunk_t {
    int start;
    int end;
};

// multithreading function
void* worker(void* arg) {
    chunk_t* chunk = (chunk_t*)arg;
    long local_countA = 0, local_countC = 0, local_countG = 0, local_countT = 0;

    for (int i = chunk->start; i < chunk->end; ++i) {        
        switch (gl_buffer[i]) {
            case 'A': local_countA++; break;
            case 'C': local_countC++; break;
            case 'G': local_countG++; break;
            case 'T': local_countT++; break;
        }
    }

    pthread_spin_lock(&lock);
    gl_countA += local_countA;
    gl_countC += local_countC;
    gl_countG += local_countG;
    gl_countT += local_countT;
    pthread_spin_unlock(&lock);
    
    return NULL;
}

// simd function
void simd_count(const char *buffer,
                uint64_t *A, uint64_t *C,
                uint64_t *G, uint64_t *T)
{
    __m256i vA = _mm256_set1_epi8('A');
    __m256i vC = _mm256_set1_epi8('C');
    __m256i vG = _mm256_set1_epi8('G');
    __m256i vT = _mm256_set1_epi8('T');

    uint64_t cA = 0, cC = 0, cG = 0, cT = 0;

    size_t i = 0;
    size_t simd_end = DNA_SIZE - (DNA_SIZE % 32);

    for (; i < simd_end; i += 32) {
        __m256i v = _mm256_loadu_si256((const __m256i*)(buffer + i));

        __m256i eqA = _mm256_cmpeq_epi8(v, vA);
        __m256i eqC = _mm256_cmpeq_epi8(v, vC);
        __m256i eqG = _mm256_cmpeq_epi8(v, vG);
        __m256i eqT = _mm256_cmpeq_epi8(v, vT);

        cA += __builtin_popcount(_mm256_movemask_epi8(eqA));
        cC += __builtin_popcount(_mm256_movemask_epi8(eqC));
        cG += __builtin_popcount(_mm256_movemask_epi8(eqG));
        cT += __builtin_popcount(_mm256_movemask_epi8(eqT));
    }

    for (; i < DNA_SIZE; i++) {
        switch (buffer[i]) {
            case 'A': cA++; break;
            case 'C': cC++; break;
            case 'G': cG++; break;
            case 'T': cT++; break;
        }
    }

    *A = cA; *C = cC; *G = cG; *T = cT;
}

struct local_t {
    int start;
    int end;
    uint64_t A, C, G, T;
};

void* simd_worker(void *arg)
{
    local_t *l = (local_t*)arg;

    __m256i vA = _mm256_set1_epi8('A');
    __m256i vC = _mm256_set1_epi8('C');
    __m256i vG = _mm256_set1_epi8('G');
    __m256i vT = _mm256_set1_epi8('T');

    uint64_t cA = 0, cC = 0, cG = 0, cT = 0;
    int simd_end = l->end - ((l->end - l->start) % 32);
    for (int i = l->start; i < simd_end; i += 32) {
        __m256i v = _mm256_loadu_si256((__m256i*)(gl_buffer + i));

        __m256i eqA = _mm256_cmpeq_epi8(v, vA);
        __m256i eqC = _mm256_cmpeq_epi8(v, vC);
        __m256i eqG = _mm256_cmpeq_epi8(v, vG);
        __m256i eqT = _mm256_cmpeq_epi8(v, vT);

        cA += __builtin_popcount(_mm256_movemask_epi8(eqA));
        cC += __builtin_popcount(_mm256_movemask_epi8(eqC));
        cG += __builtin_popcount(_mm256_movemask_epi8(eqG));
        cT += __builtin_popcount(_mm256_movemask_epi8(eqT));
    }

    for (; i < l->end; ++i)  {
    	switch (gl_buffer[i]) {
		case 'A': cA++; break;
		case 'C': cC++; break;
		case 'G': cG++; break;
		case 'T': cT++; break;
	}
    }

    l->A = cA;
    l->C = cC;
    l->G = cG;
    l->T = cT;

    return NULL;
}


int main() {
    gl_buffer = (char*)malloc(DNA_SIZE);
    
    if (!gl_buffer) {
        printf("Memory allocation failed\n");
        return 1;
    }
    for (int i = 0; i < DNA_SIZE; ++i) {
        gl_buffer[i] = random_base();
    }
    /////////////////////////////////////
    //               Scalar            //
    /////////////////////////////////////

    long countA = 0, countC = 0, countG = 0, countT = 0;
    double start = getTime();
    for (int i = 0; i < DNA_SIZE; i++) {
        switch (gl_buffer[i]) {
            case 'A': countA++; break;
            case 'C': countC++; break;
            case 'G': countG++; break;
            case 'T': countT++; break;
        }
    }
    double end = getTime();
    /////////////////////////////////////
    //           MultiThread           //
    /////////////////////////////////////

    pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);
    pthread_t threads[THREADS];
    struct chunk_t chunks[THREADS];
    int chunk_size = DNA_SIZE / THREADS;

    double mth_start = getTime();

    for (int t = 0; t < THREADS; t++) {
        chunks[t].start = t * chunk_size;
        chunks[t].end   = (t == THREADS - 1) ? DNA_SIZE : (t + 1) * chunk_size;
        pthread_create(&threads[t], NULL, worker, &chunks[t]);
    }

    for (int t = 0; t < THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    double mth_end = getTime();
    pthread_spin_destroy(&lock);

    /////////////////////////////////////
    //              SIMD               //
    /////////////////////////////////////

    uint64_t A, C, G, T;
    double simd_start = getTime();
    simd_count(gl_buffer, &A, &C, &G, &T);
    double simd_end = getTime();

    /////////////////////////////////////
    //          SIMD x THREAD          //
    /////////////////////////////////////
 
    pthread_t threads_[THREADS];
    struct local_t args[THREADS];
    int size = DNA_SIZE / THREADS;

    double simd_th_start = getTime();

    for (int t = 0; t < THREADS; t++) {
        args[t].start = t * size;
        args[t].end   = (t == THREADS - 1) ? DNA_SIZE : (t + 1) * size;
        pthread_create(&threads_[t], NULL, simd_worker, &args[t]);
    }

    uint64_t A_ = 0, C_ = 0, G_ = 0, T_ = 0;

    for (int t = 0; t < THREADS; t++) {
        pthread_join(threads_[t], NULL);
        A_ += args[t].A;
        C_ += args[t].C;
        G_ += args[t].G;
        T_ += args[t].T;
    }

    double simd_th_end = getTime();
    
    printf("DNA size: %d\n", DNA_SIZE / (1024 * 1024));
    printf("Threads used: %d\n", THREADS);
    printf("\nScalar time: %.6f sec\n", end - start);
    printf("Multithreading time: %.6f sec\n", mth_end - mth_start);
    printf("SIMD time: %.6f sec\n", simd_end - simd_start);
    printf("SIMDxMultithreading time: %.6f sec\n", simd_th_end - simd_th_start);

    free(gl_buffer);
    return 0;
}
