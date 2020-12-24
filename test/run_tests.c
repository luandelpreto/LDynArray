// Run the tests
#if defined(__linux__) || (defined(__unix__) && defined(__posix))
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NTHREADS 2U
#define ERROR(stream, msg) fprintf(stream, "[%s:%lu]: %s\n", __FILE__, __LINE__+0UL, msg)

typedef void *(*ldyna_test_fn)(void *);

void *ldyna_test_int(void *);
void *ldyna_test_sorted_int(void *);

int main(void)
{
    const ldyna_test_fn functions[] = { ldyna_test_int, ldyna_test_sorted_int, };

    pthread_t threads[NTHREADS];
    for (size_t i = 0; i < NTHREADS; i++) {
        if (pthread_create(&threads[i], NULL, functions[i], NULL) != 0) {
            ERROR(stderr, "failed to initialize thread");
            return EXIT_FAILURE;
        }
    }

    for (size_t i = 0; i < NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return EXIT_SUCCESS;
}
#endif
