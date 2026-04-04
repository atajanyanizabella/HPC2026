#include <stdio.h>
#include <omp.h>
#include <cstdlib>

struct log_t {
    int request_id;
    int user_id;
    int response_time_ms;
    int performance;
};

int main() {
    int N = 20000;
    struct log_t entries[N];
    #pragma omp parallel num_threads(4)
    {
        #pragma omp single
        {
            for (int i = 0; i < N; ++i) {
                entries[i].request_id = i;
                entries[i].user_id = i % 1000;
                entries[i].response_time_ms = rand() % 500;
            }
        }
        #pragma omp barrier

        #pragma omp for
        for (int i = 0; i < N; ++i) {
            int t = entries[i].response_time_ms;
	    if (t < 100) {
	    	entries[i].performance = 0;
	    } else if (t >= 100 && t < 300) {
	    	entries[i].performance = 1;
	    } else {
	    	entries[i].performance = 2;
	    }
        }
	#pragma omp barrier

	#pragma omp single
	{
		int fast = 0, medium = 0, slow = 0;
		for (int i = 0; i < N; ++i) {
			int perform = entries[i].performance;
			if (perform == 0)
				fast += 1;
			else if (perform == 1)
				medium += 1;
			else
				slow += 1;
		}
		printf("FAST count = %d\n", fast);
		printf("MEDIUM count = %d\n", medium);
		printf("SLOW count = %d\n", slow);
	}
    }

    return 0;
}
