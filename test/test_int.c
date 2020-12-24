// ldyna simplified int test
#include "../src/ldyna.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define NTESTS 1000U

#define TEST(msg) printf("[%s]: %s\n", __FILE__, msg)

static int compare_int(const void *key1, const void *key2)
{
    return (*(int *) key1) - (*(int *) key2);
}

void *ldyna_test_int(void *args)
{
    ldyna *list = ldyna_create(sizeof(int), compare_int, LDYNA_NONE);

    assert(list != NULL);

    ldyna_inbulk inbulk = { .inbulk = false };
    int numbers[NTESTS];
    for (size_t i = 0; i < NTESTS; i++) {
        int elem = rand() % 100 + 1;
        numbers[i] = elem;
        assert(ldyna_append(list, &elem, inbulk) == LDYNA_SUCCESS);
    }
    assert(ldyna_len(list) == NTESTS);

    for (size_t i = 0; i < NTESTS; i++) {
        int data;
        int res = ldyna_get(list, i, &data);
        assert(res == LDYNA_SUCCESS);
        assert(data == numbers[i]);
    }

    ldyna *lstcopy = ldyna_copy(list, inbulk);

    assert(ldyna_sort(lstcopy, compare_int) == LDYNA_SUCCESS);
    int prev = -1;
    for (size_t i = 0; i < NTESTS; i++) {
        int data;
        assert(ldyna_get(lstcopy, i, &data) == LDYNA_SUCCESS);
        assert(prev <= data);
        prev = data;
    }

    for (size_t i = 0; i < NTESTS; i++) {
        int data;
        int res = ldyna_remove(list, 0, &data);
        assert(res == LDYNA_SUCCESS);
        assert(data == numbers[i]);
    }
    assert(ldyna_len(list) == 0);

    ldyna_destroy(&lstcopy);
    ldyna_destroy(&list);
    TEST("*** All tests passed");

    return NULL;
}
