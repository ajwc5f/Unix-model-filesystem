#define DYN_MAX_CAPACITY 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/dyn_array.c"

// clang-format off
/*
    dyn_array_t *dyn_array_create(size_t capacity, size_t data_type_size, void (*destruct_func)(void *));
        1. NORMAL, capacity 0, assert 16
        2. NORMAL, capacity 15, assert 16
        3. NORMAL, capacity 16, assert 16
        4. NORMAL, capacity 17, assert 32
        5. NORMAL, capacity DYN_MAX_CAPACITY - 1, assert DYN_MAX_CAPACITY
        6. NORMAL, capacity DYN_MAX_CAPACITY, assert DYN_MAX_CAPACITY
        7. NORMAL, NULL function pointer
        8. FAIL, capacity > DYN_MAX_CAPACITY
        9. FAIL, data_size == 0


    void dyn_array_destroy(dyn_array_t *const dyn_array);
        1. NORMAL, empty
        2. NORMAL, with contents, NULL destructor
        3. NORMAL, with contents, using destructor
        4. FAIL, NULL pointer


    void *dyn_array_front(const dyn_array_t *const dyn_array);
        1. NORMAL, has contents
        2. FAIL, empty.
        3. FAIL, NULL pointer

    bool dyn_array_push_front(dyn_array_t *const dyn_array, void *object);
        1. NORMAL, has contents
        2. NORMAL, empty
        3. NORMAL, at capacity boundary
        4. FAIL, NULL array
        5. FAIL, NULL object
        6. FAIL, at max capacity

    bool dyn_array_pop_front(dyn_array_t *const dyn_array);
        1. NORMAL, size = 1
        2. NORMAL, size > 1
        3. FAIL, size = 0
        4. NORMAL, with destructor
        5. FAIL, NULL pointer

    bool dyn_array_extract_front(dyn_array_t *const dyn_array, void *object);
        1. NORMAL, size = 1
        2. NORMAL, size > 1
        3. NORMAL, with destructor, assert not destructed
        4. FAIL, size = 0
        5. FAIL, null array
        6. FAIL, null object


    void *dyn_array_back(dyn_array_t *const dyn_array);
        1. NORMAL, has contents
        2. FAIL, empty.
        3. FAIL, NULL pointer
        4. NORMAL, assert front == back on size = 1


    bool dyn_array_push_back(dyn_array_t *const dyn_array, void *object);
        1. NORMAL, has contents
        2. NORMAL, empty
        3. NORMAL, at capacity boundary
        4. FAIL, NULL array
        5. FAIL, NULL object
        6. FAIL, at max capacity


    bool dyn_array_pop_back(dyn_array_t *const dyn_array);
        1. NORMAL, size = 1
        2. NORMAL, size > 1
        3. NORMAL(?), size = 0
        4. NORMAL, with destructor
        5. FAIL, NULL pointer


    bool dyn_array_extract_back(dyn_array_t *const dyn_array, void *object);
        1. NORMAL, size = 1
        2. NORMAL, size > 1
        3. NORMAL, with destructor, assert not destructed
        4. FAIL, size = 0
        5. FAIL, null array
        6. FAIL, null object


    void *dyn_array_at(const dyn_array_t *const dyn_array, size_t index);
        1. NORMAL, front
        2. NORMAL, back
        3. NORMAl, arbitrary
        4. FAIL, idx = size
        5. FAIL, idx > size
        6. FAIL, empty, idx = 0
        7. FAIL, empty, idx = 1
        8. FAIl, null pointer


    bool dyn_array_insert(dyn_array_t *const dyn_array, size_t index, void *object);
        1. NORMAL, front
        2. NORMAL, back
        3. NORMAL, arbitrary
        4. NORMAL (FAIL?), idx = size
        5. FAIL, empty, idx = 1 (just to make sure)
        6. FAIL, out of bounds
        7. FAIL, null object
        8. FAIL, null array


    bool dyn_array_erase(dyn_array_t *const dyn_array, size_t index);
        1. NORMAL, front
        2. NORMAL, back
        3. NORMAL, arbitrary
        4. NORMAL, with destructor
        5. FAIL, empty, idx = 0
        6. FAIL, empty, idx = 1
        7. FAIL, idx = size
        8. FAIL, idx > size
        9. FAIL, null ptr


    bool dyn_array_extract(dyn_array_t *const dyn_array, size_t index, void *object);
        1. NORMAL, front
        2. NORMAL, back
        3. NORMAL, arbitrary
        4. NORMAL, with destructor, assert not destructed
        5. FAIL, empty, idx = 0
        6. FAIL, empty, idx = 1
        7. FAIL, idx = size
        8. FAIL, idx > size
        9. FAIL, null ptr
        10.FAIL, null array


    void dyn_array_clear(dyn_array_t *const dyn_array);
        1. NORMAL, contents
        2. NORMAL, contents with destructor
        3. NORMAL, empty
        4. FAIL, null ptr

    bool dyn_array_empty(dyn_array_t *const dyn_array);
        1. NORMAL, contents
        2. NORMAL, empty
        3. FAIL, null ptr

    size_t dyn_array_size(const dyn_array_t *const dyn_array);
        1. NORMAL, contents
        2. NORMAL, empty
        3. FAIL, null ptr

    size_t dyn_array_capacity(const dyn_array_t *const dyn_array);
        1. NORMAL
        2. FAIL, null ptr

    size_t dyn_array_data_size(const dyn_array_t *const dyn_array);
        1. NORMAL
        2. FAIL, null ptr

    const void *dyn_array_export(const dyn_array_t *const dyn_array);
        SEE FRONT

    dyn_array_t *dyn_array_import(const void *const data, const size_t count, const size_t data_type_size, void (*destruct_func)(void *));
        SEE INIT (capacity -> size)
        10. FAIL, void data

    bool dyn_array_insert_sorted(dyn_array_t *const dyn_array, const void *const object,
                                 int (*compare)(const void *, const void *));
        1. NORMAL (probably best for a begin, middle and end insert test)
        2. NORMAL, insert into empty array
        3. FAIL, NULL array
        4. FAIL, null object
        5. FAIL, null comparator

    bool dyn_array_sort(dyn_array_t *const dyn_array, int (*compare)(const void *, const void *));
        1. NORMAL, contents (PLURAL!)
        2. NORMAL, sort with 1 object (technically a failure, but we won't make fun of the user too mcuh)
        3. FAIL, no array
        4. FAIL, empty array
        5. FAIL, null comparator

    bool dyn_array_for_each(dyn_array_t *const dyn_array, void (*func)(void *const), void * arg);
        1. NORMAL, contents
        2. NORMAL, empty
        3. NORMAL, null arg
        4. FAIL, null array
        5. FAIL, null func
*/
// clang-format on

// Shamelessly stolen from
// http://pixelscommander.com/wp-content/uploads/2014/12/P10.pdf
// modded a bit so it dies when false

#define assert(e) \
    ((e) ? (true) \
         : (fprintf(stderr, "%s,%d: assertion '%s' failed\n", __FILE__, __LINE__, #e), fflush(stdout), abort(), 0))

#define DATA_BLOCK_SIZE 100
uint8_t DATA_BLOCKS[6][DATA_BLOCK_SIZE];
int destruct_counter = 0;

void block_destructor(void *block) {
    memset(block, 0xFF, 100);
    // right, probably can't check if the destructor ran if the memory gets freed
    ++destruct_counter;
}

void block_destructor_mini(void *block) {
    memset(block, 0xAA, 1);
    // right, probably can't check if the destructor ran if the memory gets freed
    ++destruct_counter;
}

int for_each_counter = 0;
void block_for_each(void *const block, void *num) {
    if (block) {
        if (num)
            for_each_counter += *((int *) num);
        else
            for_each_counter += 10;
    }
}

int block_compare(const void *const a, const void *const b) {
    return ((int) (((const uint8_t *) a)[0])) - (((const uint8_t *) b)[0]);
}

int block_compare_inv(const void *const a, const void *const b) {
    return ((int) (((const uint8_t *) b)[0])) - (((const uint8_t *) a)[0]);
}

void init_data_blocks() {
    memset(DATA_BLOCKS[0], 0x11, 100);
    memset(DATA_BLOCKS[1], 0x22, 100);
    memset(DATA_BLOCKS[2], 0x33, 100);
    memset(DATA_BLOCKS[3], 0x44, 100);
    memset(DATA_BLOCKS[4], 0x55, 100);
    memset(DATA_BLOCKS[5], 0xFF, 100);
}

// CONSTRUCTOR, DESTRUCTOR, AT, FRONT, PUSH_FRONT, BACK, SIZE, CLEAR, EMPTY
void run_basic_tests_a();

// PUSH_BACK, POP_FRONT, POP_BACK, EXTRACT_FRONT, EXTRACT_BACK
void run_basic_tests_b();

// INSERT, EXTRACT, ERASE
void run_basic_tests_c();

// EXTRACT, IMPORT
void run_basic_tests_d();

// SORT and INSERT_SORTED
void run_basic_tests_e();

void run_tests() {
    init_data_blocks();

    // CONSTRUCTOR, DESTRUCTOR, AT, FRONT, PUSH_FRONT, BACK, SIZE, CLEAR, EMPTY, CAPACITY, DATA_SIZE
    run_basic_tests_a();

    // PUSH_BACK, POP_FRONT, POP_BACK, EXTRACT_FRONT, EXTRACT_BACK
    run_basic_tests_b();

    // INSERT, EXTRACT, ERASE
    run_basic_tests_c();

    // EXTRACT, IMPORT
    run_basic_tests_d();

    // SORT INSERT_SORTED
    run_basic_tests_e();

    puts("TESTS COMPLETE");
}

void run_basic_tests_a() {
    dyn_array_t *dyn_a = NULL, *dyn_b = NULL;

    // CONSTRUCTOR / DESTRUCTOR
    // 1 & 7 CONSTRUCTOR
    dyn_a = dyn_array_create(0, 4, NULL);
    assert(dyn_a);
    assert(dyn_a->size == 0);
    assert(dyn_a->capacity == 16);
    assert(dyn_a->data_size == 4);
    assert(dyn_a->destructor == NULL);
    assert(dyn_a->array);

    // CAPACITY 1
    assert(dyn_array_capacity(dyn_a) == 16);

    // CAPACITY 2
    assert(dyn_array_capacity(NULL) == 0);

    // CAPACITY tested and clear for use

    // DATA_SIZE 1
    assert(dyn_array_data_size(dyn_a) == 4);

    // DATA_SIZE 2
    assert(dyn_array_data_size(NULL) == 0);

    // DATA_SIZE tested and clear fo use

    // 1 & 4 DESTRUCTOR
    dyn_array_destroy(dyn_a);
    dyn_array_destroy(NULL);

    // 2 CONSTRUCTOR
    dyn_a = dyn_array_create(15, 5, &block_destructor);
    assert(dyn_a);
    assert(dyn_a->size == 0);
    assert(dyn_a->capacity == 16);
    assert(dyn_a->data_size == 5);
    assert(dyn_a->destructor == &block_destructor);
    assert(dyn_a->array);

    dyn_array_destroy(dyn_a);

    // 3 CONSTRUCTOR
    dyn_a = dyn_array_create(16, 8, &block_destructor);
    assert(dyn_a);
    assert(dyn_a->size == 0);
    assert(dyn_a->capacity == 16);
    assert(dyn_a->data_size == 8);
    assert(dyn_a->destructor == &block_destructor);
    assert(dyn_a->array);

    dyn_array_destroy(dyn_a);

    // 4 CONSTRUCTOR
    dyn_a = dyn_array_create(17, 4, &block_destructor);
    assert(dyn_a);
    assert(dyn_a->size == 0);
    assert(dyn_a->capacity == 32);
    assert(dyn_a->data_size == 4);
    assert(dyn_a->destructor == &block_destructor);
    assert(dyn_a->array);

    dyn_array_destroy(dyn_a);

    // 5 CONSTRUCTOR
    dyn_a = dyn_array_create(DYN_MAX_CAPACITY - 1, 4, &block_destructor);

    assert(dyn_a);
    assert(dyn_a->size == 0);
    assert(dyn_a->capacity == DYN_MAX_CAPACITY);
    assert(dyn_a->data_size == 4);
    assert(dyn_a->destructor == &block_destructor);
    assert(dyn_a->array);

    dyn_array_destroy(dyn_a);

    // 6 CONSTRUCTOR
    dyn_a = dyn_array_create(DYN_MAX_CAPACITY, 4, &block_destructor);

    assert(dyn_a);
    assert(dyn_a->size == 0);
    assert(dyn_a->capacity == DYN_MAX_CAPACITY);
    assert(dyn_a->data_size == 4);
    assert(dyn_a->destructor == &block_destructor);
    assert(dyn_a->array);

    dyn_array_destroy(dyn_a);

    // 8 CONSTRUCTOR
    dyn_a = dyn_array_create(DYN_MAX_CAPACITY + 1, 4, &block_destructor);
    assert(!dyn_a);

    // 9 CONSTRUCTOR
    dyn_a = dyn_array_create(16, 0, &block_destructor);
    assert(!dyn_a);

    // CONSTRUCTOR TESTS COMPLETE


    // PUSH_FRONT, FRONT, SIZE, CLEAR, DESTRUCTOR
    // so basic operations can be used

    dyn_a = dyn_array_create(0, DATA_BLOCK_SIZE, NULL);
    assert(dyn_a);

    // 2 & 3 FRONT
    assert(dyn_array_front(dyn_a) == NULL);
    assert(dyn_array_front(NULL) == NULL);
    // 2 SIZE
    assert(dyn_array_size(dyn_a) == 0);
    // 3 SIZE
    assert(dyn_array_size(NULL) == 0);
    // 2 EMPTY
    assert(dyn_array_empty(dyn_a));
    // 3 EMPTY
    assert(dyn_array_empty(NULL));

    // 2 PUSH_FRONT
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[0]));
    assert(dyn_a->size == 1);
    assert(memcmp(dyn_a->array, DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 1 EMPTY
    assert(dyn_array_empty(dyn_a) == false);
    // 1 SIZE
    assert(dyn_array_size(dyn_a) == 1);

    // SIZE and EMPTY TESTS COMPLETE

    // 1 FRONT
    assert(dyn_array_front(dyn_a) == dyn_a->array);
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // FRONT TESTS COMPLETE

    // 1 PUSH_FRONT
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[1]));
    assert(dyn_array_size(dyn_a) == 2);
    assert(dyn_array_front(dyn_a) == dyn_a->array);
    assert(memcmp(dyn_a->array, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);

    // 4 & 5 PUSH_FRONT
    assert(dyn_array_push_front(NULL, DATA_BLOCKS[1]) == false);
    assert(dyn_array_push_front(dyn_a, NULL) == false);
    assert(dyn_a->size == 2);

    // 1 CLEAR
    dyn_array_clear(dyn_a);
    assert(dyn_a->size == 0);
    // 3 CLEAR
    dyn_array_clear(dyn_a);
    assert(dyn_a->size == 0);
    // 4 CLEAR
    dyn_array_clear(NULL);


    // 6 AT
    assert(dyn_array_at(dyn_a, 0) == NULL);
    // 7 AT
    assert(dyn_array_at(dyn_a, 1) == NULL);

    dyn_b = dyn_array_create(0, DATA_BLOCK_SIZE, &block_destructor);
    assert(dyn_b);
    // 2 BACK
    assert(dyn_array_back(dyn_b) == NULL);
    // 3 BACK
    assert(dyn_array_back(NULL) == NULL);
    assert(dyn_array_push_front(dyn_b, DATA_BLOCKS[0]));
    // 4 BACK
    assert(dyn_array_front(dyn_b) == dyn_array_back(dyn_b));
    assert(dyn_array_push_front(dyn_b, DATA_BLOCKS[1]));

    // 2 CLEAR
    dyn_array_clear(dyn_b);
    assert(destruct_counter == 2);
    destruct_counter = 0;

    // CLEAR TESTS COMPLETE

    assert(dyn_array_push_front(dyn_b, DATA_BLOCKS[0]));
    assert(dyn_array_push_front(dyn_b, DATA_BLOCKS[1]));
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[0]));
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[1]));
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[2]));

    // 1 AT
    assert(dyn_array_at(dyn_a, 0) == dyn_array_front(dyn_a));
    assert(memcmp(dyn_array_at(dyn_a, 0), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    // 2 AT
    // 1 BACK
    assert(dyn_array_at(dyn_a, 2) == dyn_array_back(dyn_a));
    assert(memcmp(dyn_array_at(dyn_a, 2), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // BACK TESTS COMPLETE

    // 3 AT
    assert(dyn_array_at(dyn_a, 1));
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);

    // 4 AT
    assert(dyn_array_at(dyn_a, 3) == NULL);
    // 5 AT
    assert(dyn_array_at(dyn_a, 4) == NULL);
    // 8 AT
    assert(dyn_array_at(NULL, 0) == NULL);

    // AT TESTS COMPLETE

    // 2 DESTRUCTOR
    dyn_array_destroy(dyn_a);

    // 3 DESTRUCTOR
    dyn_array_destroy(dyn_b);
    assert(destruct_counter == 2);
    destruct_counter = 0;

    // DESTRUCTOR TESTS COMPLETE

    dyn_a = dyn_array_create(0, DATA_BLOCK_SIZE, NULL);
    assert(dyn_a);
    while (dyn_a->size != 16) {
        assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[0]));
    }
    // 3 PUSH_FRONT
    assert(dyn_a->size == 16);
    assert(dyn_a->capacity == 16);
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[1]));
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 17);
    assert(dyn_a->capacity == 32);

    while (dyn_a->size != DYN_MAX_CAPACITY) {
        assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[0]));
    }
    // 6 PUSH_FRONT
    assert(dyn_a->size == DYN_MAX_CAPACITY);
    assert(dyn_a->capacity == DYN_MAX_CAPACITY);
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[1]) == false);
    assert(dyn_a->size == DYN_MAX_CAPACITY);
    assert(dyn_a->capacity == DYN_MAX_CAPACITY);
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // PUSH_FRONT TESTS COMPLETE

    dyn_array_destroy(dyn_a);
}

void run_basic_tests_b() {
    destruct_counter = 0;
    dyn_array_t *dyn_a, *dyn_b;

    dyn_a = dyn_array_create(0, DATA_BLOCK_SIZE, NULL);
    assert(dyn_a);
    dyn_b = dyn_array_create(0, DATA_BLOCK_SIZE, &block_destructor);
    assert(dyn_b);

    // 2 POP_FRONT
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[0]));
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[1]));
    assert(dyn_a->size == 2);

    assert(dyn_array_pop_front(dyn_a));
    assert(dyn_a->size == 1);
    assert(memcmp(dyn_a->array, DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 1 POP_FRONT
    assert(dyn_array_pop_front(dyn_a));
    assert(dyn_a->size == 0);

    // 3 POP_FRONT
    assert(dyn_array_pop_front(dyn_a) == false);
    assert(dyn_a->size == 0);

    // 5 POP_FRONT
    assert(dyn_array_pop_front(NULL) == false);

    // 4 POP_FRONT
    assert(dyn_array_push_front(dyn_b, DATA_BLOCKS[0]));
    assert(dyn_array_pop_front(dyn_b));
    assert(dyn_b->size == 0);
    assert(destruct_counter == 1);
    destruct_counter = 0;

    // POP_FRONT TESTS COMPLETE

    // 2 PUSH_BACK
    assert(dyn_array_push_back(dyn_a, DATA_BLOCKS[2]));
    assert(dyn_a->size == 1);
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);

    // 1 PUSH_BACK
    assert(dyn_array_push_back(dyn_a, DATA_BLOCKS[3]));
    assert(dyn_a->size == 2);
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_back(dyn_a), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);

    while (dyn_array_size(dyn_a) != 16) {
        assert(dyn_array_push_back(dyn_a, DATA_BLOCKS[3]));
    }
    assert(dyn_a->capacity == 16);

    // 5 PUSH_BACK
    assert(dyn_array_push_back(dyn_a, NULL) == false);
    assert(dyn_a->size == 16);
    assert(dyn_a->capacity == 16);

    // 3 PUSH_BACK
    assert(dyn_array_push_back(dyn_a, DATA_BLOCKS[4]));
    assert(dyn_a->size == 17);
    assert(dyn_a->capacity == 32);
    assert(memcmp(dyn_array_back(dyn_a), DATA_BLOCKS[4], DATA_BLOCK_SIZE) == 0);

    // 4 PUSH_BACK
    assert(dyn_array_push_back(NULL, DATA_BLOCKS[0]) == false);
    assert(dyn_a->size == 17);

    // 6 PUSH_BACK
    while (dyn_a->size != DYN_MAX_CAPACITY) {
        assert(dyn_array_push_back(dyn_a, DATA_BLOCKS[4]));
    }
    assert(memcmp(dyn_array_back(dyn_a), DATA_BLOCKS[4], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->capacity == DYN_MAX_CAPACITY);
    assert(dyn_array_push_back(dyn_a, DATA_BLOCKS[5]) == false);
    assert(dyn_a->size == DYN_MAX_CAPACITY);
    assert(dyn_a->capacity == DYN_MAX_CAPACITY);
    assert(memcmp(dyn_array_back(dyn_a), DATA_BLOCKS[4], DATA_BLOCK_SIZE) == 0);

    // PUSH_BACK TESTS COMPLETE

    dyn_array_clear(dyn_a);
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[0]));
    assert(dyn_array_push_front(dyn_b, DATA_BLOCKS[0]));
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[1]));
    assert(dyn_array_push_front(dyn_b, DATA_BLOCKS[1]));

    // 2 POP_BACK
    assert(dyn_array_pop_back(dyn_a));
    assert(dyn_a->size == 1);
    assert(memcmp(dyn_array_back(dyn_a), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);

    // 1 POP_BACK
    assert(dyn_array_pop_back(dyn_a));
    assert(dyn_a->size == 0);

    // 3 POP_BACK
    assert(dyn_array_pop_back(dyn_a) == false);
    assert(dyn_a->size == 0);

    // 4 POP_BACK
    assert(dyn_array_pop_back(dyn_b));
    assert(dyn_b->size == 1);
    assert(memcmp(dyn_array_back(dyn_b), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(destruct_counter == 1);
    destruct_counter = 0;
    assert(dyn_array_pop_back(dyn_b));
    assert(dyn_b->size == 0);
    assert(destruct_counter == 1);
    destruct_counter = 0;

    // 5 POP_BACK
    assert(dyn_array_pop_back(NULL) == false);

    // POP_BACK TESTS COMPLETE

    uint8_t extraction_point[DATA_BLOCK_SIZE];

    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[0]));
    assert(dyn_array_push_front(dyn_b, DATA_BLOCKS[0]));
    assert(dyn_array_push_front(dyn_a, DATA_BLOCKS[1]));
    assert(dyn_array_push_front(dyn_b, DATA_BLOCKS[1]));

    // 2 EXTRACT_FRONT
    assert(dyn_array_extract_front(dyn_a, extraction_point));
    assert(dyn_a->size == 1);
    assert(memcmp(extraction_point, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);

    // 1 EXTRACT_FRONT
    assert(dyn_array_extract_front(dyn_a, extraction_point));
    assert(dyn_a->size == 0);
    assert(memcmp(extraction_point, DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 3 EXTRACT_FRONT
    assert(dyn_array_extract_front(dyn_b, extraction_point));
    assert(dyn_b->size == 1);
    assert(memcmp(extraction_point, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(destruct_counter == 0);

    // 4 EXTRACT_FRONT
    assert(dyn_array_extract_front(dyn_a, extraction_point) == false);
    assert(memcmp(extraction_point, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 0);

    // 5 EXTRACT_FRONT
    assert(dyn_array_extract_front(NULL, extraction_point) == false);
    assert(memcmp(extraction_point, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);

    // 6 EXTRACT_FRONT
    assert(dyn_array_extract_front(dyn_a, NULL) == false);
    assert(dyn_a->size == 0);

    dyn_array_clear(dyn_b);
    destruct_counter = 0;

    // EXTRACT_FRONT TESTS COMPLETE

    assert(dyn_array_push_back(dyn_a, DATA_BLOCKS[0]));
    assert(dyn_array_push_back(dyn_b, DATA_BLOCKS[0]));
    assert(dyn_array_push_back(dyn_a, DATA_BLOCKS[1]));
    assert(dyn_array_push_back(dyn_b, DATA_BLOCKS[1]));

    // 2 EXTRACT_BACK
    assert(dyn_array_extract_back(dyn_a, extraction_point));
    assert(dyn_a->size == 1);
    assert(memcmp(extraction_point, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);

    // 1 EXTRACT_BACK
    assert(dyn_array_extract_back(dyn_a, extraction_point));
    assert(dyn_a->size == 0);
    assert(memcmp(extraction_point, DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 3 EXTRACT_BACK
    assert(dyn_array_extract_back(dyn_b, extraction_point));
    assert(dyn_b->size == 1);
    assert(memcmp(extraction_point, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(destruct_counter == 0);

    // 4 EXTRACT_BACK
    assert(dyn_array_extract_back(dyn_a, extraction_point) == false);
    assert(memcmp(extraction_point, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 0);

    // 5 EXTRACT_BACK
    assert(dyn_array_extract_back(NULL, extraction_point) == false);
    assert(memcmp(extraction_point, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);

    // 6 EXTRACT_BACK
    assert(dyn_array_extract_back(dyn_a, NULL) == false);
    assert(dyn_a->size == 0);

    dyn_array_destroy(dyn_a);
    dyn_array_destroy(dyn_b);
    destruct_counter = 0;

    // EXTRACT_BACK TESTS COMPLETE
}

// INSERT, EXTRACT, ERASE
void run_basic_tests_c() {
    dyn_array_t *dyn_a = NULL, *dyn_b = NULL;
    uint8_t extraction_point[DATA_BLOCK_SIZE];
    destruct_counter = 0;

    dyn_a = dyn_array_create(0, DATA_BLOCK_SIZE, NULL);
    assert(dyn_a);
    dyn_b = dyn_array_create(0, DATA_BLOCK_SIZE, &block_destructor);
    assert(dyn_b);

    // 4 INSERT
    assert(dyn_array_insert(dyn_a, 0, DATA_BLOCKS[0]));
    assert(dyn_a->size == 1);
    assert(memcmp(dyn_a->array, DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 1 INSERT
    assert(dyn_array_insert(dyn_a, 0, DATA_BLOCKS[1]));
    assert(dyn_a->size == 2);
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 2 INSERT
    assert(dyn_array_insert(dyn_a, 1, DATA_BLOCKS[2]));
    assert(dyn_a->size == 3);
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 2), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 3 INSERT
    assert(dyn_array_insert(dyn_a, 1, DATA_BLOCKS[3]));
    assert(dyn_a->size == 4);
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 2), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 3), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 5 INSERT
    assert(dyn_array_insert(dyn_b, 1, DATA_BLOCKS[0]) == false);
    assert(dyn_b->size == 0);

    // 6 INSERT
    assert(dyn_array_insert(dyn_a, 5, DATA_BLOCKS[5]) == false);
    assert(dyn_a->size == 4);
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 2), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 3), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 7 INSERT
    assert(dyn_array_insert(dyn_a, 0, NULL) == false);
    assert(dyn_a->size == 4);
    assert(memcmp(dyn_array_front(dyn_a), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 2), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 3), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);

    // 8 INSERT
    assert(dyn_array_insert(NULL, 0, DATA_BLOCKS[0]) == false);

    // INSERT TESTS COMPLETE

    dyn_array_clear(dyn_a);
    dyn_array_clear(dyn_b);
    destruct_counter = 0;

    dyn_array_push_back(dyn_a, DATA_BLOCKS[0]);
    dyn_array_push_back(dyn_a, DATA_BLOCKS[1]);
    dyn_array_push_back(dyn_a, DATA_BLOCKS[2]);
    dyn_array_push_back(dyn_a, DATA_BLOCKS[3]);
    dyn_array_push_back(dyn_b, DATA_BLOCKS[0]);
    dyn_array_push_back(dyn_b, DATA_BLOCKS[1]);
    dyn_array_push_back(dyn_b, DATA_BLOCKS[2]);
    dyn_array_push_back(dyn_b, DATA_BLOCKS[3]);

    // 1 ERASE
    assert(dyn_array_erase(dyn_a, 0));
    assert(memcmp(dyn_array_at(dyn_a, 0), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 2), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 3);

    // 3 ERASE
    assert(dyn_array_erase(dyn_a, 1));
    assert(memcmp(dyn_array_at(dyn_a, 0), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 2);

    // 2 ERASE
    assert(dyn_array_erase(dyn_a, 1));
    assert(memcmp(dyn_array_at(dyn_a, 0), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 1);

    assert(dyn_array_pop_back(dyn_a));
    // 4 ERASE
    assert(dyn_array_erase(dyn_b, 2));
    assert(destruct_counter == 1);
    destruct_counter = 0;
    assert(dyn_b->size == 3);
    assert(memcmp(dyn_array_at(dyn_b, 0), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_b, 1), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_b, 2), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);

    // 5 ERASE
    assert(dyn_array_erase(dyn_a, 0) == false);
    assert(dyn_a->size == 0);

    // 6 ERASE
    assert(dyn_array_erase(dyn_a, 1) == false);
    assert(dyn_a->size == 0);

    // 7 ERASE
    assert(dyn_array_erase(dyn_b, 3) == false);
    assert(destruct_counter == 0);
    assert(memcmp(dyn_array_at(dyn_b, 0), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_b, 1), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_b, 2), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(dyn_b->size == 3);

    // 8 ERASE
    assert(dyn_array_erase(dyn_b, 4) == false);
    assert(destruct_counter == 0);
    assert(memcmp(dyn_array_at(dyn_b, 0), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_b, 1), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_b, 2), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(dyn_b->size == 3);

    // 9 ERASE
    assert(dyn_array_erase(NULL, 0) == false);

    // ERASE TESTS COMPLETE

    dyn_array_clear(dyn_a);
    dyn_array_clear(dyn_b);
    destruct_counter = 0;

    dyn_array_push_back(dyn_a, DATA_BLOCKS[0]);
    dyn_array_push_back(dyn_a, DATA_BLOCKS[1]);
    dyn_array_push_back(dyn_a, DATA_BLOCKS[2]);
    dyn_array_push_back(dyn_a, DATA_BLOCKS[3]);
    dyn_array_push_back(dyn_b, DATA_BLOCKS[0]);
    dyn_array_push_back(dyn_b, DATA_BLOCKS[1]);
    dyn_array_push_back(dyn_b, DATA_BLOCKS[2]);
    dyn_array_push_back(dyn_b, DATA_BLOCKS[3]);

    // 1 EXTRACT
    assert(dyn_array_extract(dyn_a, 0, extraction_point));
    assert(memcmp(extraction_point, DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 0), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 2), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 3);

    // 3 EXTRACT
    assert(dyn_array_extract(dyn_a, 1, extraction_point));
    assert(memcmp(extraction_point, DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 0), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 1), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 2);

    // 2 EXTRACT
    assert(dyn_array_extract(dyn_a, 1, extraction_point));
    assert(memcmp(extraction_point, DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 0), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 1);

    // 7 EXTRACT
    assert(dyn_array_extract(dyn_a, 1, extraction_point) == false);
    assert(memcmp(extraction_point, DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 0), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 1);

    // 8 EXTRACT
    assert(dyn_array_extract(dyn_a, 2, extraction_point) == false);
    assert(memcmp(extraction_point, DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_a, 0), DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(dyn_a->size == 1);

    // 4 EXTRACT
    assert(dyn_array_extract(dyn_b, 1, extraction_point));
    assert(memcmp(extraction_point, DATA_BLOCKS[1], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_b, 0), DATA_BLOCKS[0], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_b, 1), DATA_BLOCKS[2], DATA_BLOCK_SIZE) == 0);
    assert(memcmp(dyn_array_at(dyn_b, 2), DATA_BLOCKS[3], DATA_BLOCK_SIZE) == 0);
    assert(dyn_b->size == 3);
    assert(destruct_counter == 0);

    dyn_array_clear(dyn_a);

    // 5 EXTRACT
    assert(dyn_array_extract(dyn_a, 0, extraction_point) == false);
    assert(dyn_a->size == 0);

    // 6 EXTRACT
    assert(dyn_array_extract(dyn_a, 1, extraction_point) == false);
    assert(dyn_a->size == 0);

    // 9 EXTRACT
    assert(dyn_array_extract(dyn_b, 0, NULL) == false);
    assert(dyn_b->size == 3);
    assert(destruct_counter == 0);

    // 10 EXTRACT
    assert(dyn_array_extract(NULL, 0, extraction_point) == false);

    // EXTRACT TESTS COMPLETE

    dyn_array_destroy(dyn_a);
    dyn_array_destroy(dyn_b);
    destruct_counter = 0;
}

// EXTRACT, IMPORT
void run_basic_tests_d() {
    // by now you've seen the tests. I don't want to make more...
    // but I want to add features. UGH. Just copying tests from earlier and repurposing them
    // because export is just front and import is more or less init

    dyn_array_t *dyn_a = NULL;

    // 1 IMPORT & 2 EXPORT
    dyn_a = dyn_array_import(DATA_BLOCKS[0], 1, 1, NULL);
    assert(dyn_a);
    assert(dyn_a->size == 1);
    assert(dyn_a->capacity == 16);
    assert(dyn_a->data_size == 1);
    assert(dyn_a->destructor == NULL);
    assert(dyn_a->array);
    assert(dyn_array_pop_back(dyn_a));
    assert(dyn_array_export(dyn_a) == NULL);

    dyn_array_destroy(dyn_a);

    // 2 IMPORT & EXPORT 1
    dyn_a = dyn_array_import(DATA_BLOCKS[1], 15, 1, &block_destructor_mini);
    assert(dyn_a);
    assert(dyn_a->size == 15);
    assert(dyn_a->capacity == 16);
    assert(dyn_a->data_size == 1);
    assert(dyn_a->destructor == &block_destructor_mini);
    assert(dyn_a->array);
    assert(memcmp(dyn_a->array, DATA_BLOCKS[1], 15) == 0);
    assert(dyn_array_export(dyn_a) == dyn_a->array);

    dyn_array_destroy(dyn_a);

    // 3 IMPORT
    dyn_a = dyn_array_import(DATA_BLOCKS[2], 16, 1, &block_destructor_mini);
    assert(dyn_a);
    assert(dyn_a->size == 16);
    assert(dyn_a->capacity == 16);
    assert(dyn_a->data_size == 1);
    assert(dyn_a->destructor == &block_destructor_mini);
    assert(dyn_a->array);
    assert(memcmp(dyn_a->array, DATA_BLOCKS[2], 16) == 0);

    dyn_array_destroy(dyn_a);

    // 4 IMPORT
    dyn_a = dyn_array_import(DATA_BLOCKS[3], 17, 1, &block_destructor_mini);
    assert(dyn_a);
    assert(dyn_a->size == 17);
    assert(dyn_a->capacity == 32);
    assert(dyn_a->data_size == 1);
    assert(dyn_a->destructor == &block_destructor_mini);
    assert(dyn_a->array);
    assert(memcmp(dyn_a->array, DATA_BLOCKS[3], 17) == 0);

    dyn_array_destroy(dyn_a);

    // 5 IMPORT
    dyn_a = dyn_array_import(DATA_BLOCKS[4], DYN_MAX_CAPACITY - 1, 1, &block_destructor_mini);

    assert(dyn_a);
    assert(dyn_a->size == DYN_MAX_CAPACITY - 1);
    assert(dyn_a->capacity == DYN_MAX_CAPACITY);
    assert(dyn_a->data_size == 1);
    assert(dyn_a->destructor == &block_destructor_mini);
    assert(dyn_a->array);
    assert(memcmp(dyn_a->array, DATA_BLOCKS[4], DYN_MAX_CAPACITY - 1) == 0);

    dyn_array_destroy(dyn_a);

    // 6 IMPORT
    dyn_a = dyn_array_import(DATA_BLOCKS[5], DYN_MAX_CAPACITY, 1, &block_destructor_mini);

    assert(dyn_a);
    assert(dyn_a->size == DYN_MAX_CAPACITY);
    assert(dyn_a->capacity == DYN_MAX_CAPACITY);
    assert(dyn_a->data_size == 1);
    assert(dyn_a->destructor == &block_destructor_mini);
    assert(dyn_a->array);
    assert(memcmp(dyn_a->array, DATA_BLOCKS[5], DYN_MAX_CAPACITY) == 0);

    dyn_array_destroy(dyn_a);

    // 8 IMPORT
    dyn_a = dyn_array_import(DATA_BLOCKS[0], DYN_MAX_CAPACITY + 1, 1, &block_destructor_mini);
    assert(!dyn_a);

    // 9 IMPORT
    dyn_a = dyn_array_import(DATA_BLOCKS[0], 16, 0, &block_destructor_mini);
    assert(!dyn_a);

    // 3 EXPORT
    assert(dyn_array_export(NULL) == NULL);
}

// SORT INSERT_SORTED
void run_basic_tests_e() {
    dyn_array_t *dyn_a = NULL;

    assert((dyn_a = dyn_array_create(0, DATA_BLOCK_SIZE, NULL)));

    // INSERT_SORTED 2
    assert(dyn_array_insert_sorted(dyn_a, DATA_BLOCKS[3], &block_compare));

    // INSERT_SORTED 1
    assert(dyn_array_insert_sorted(dyn_a, DATA_BLOCKS[0], &block_compare));
    assert(((uint8_t *) dyn_array_at(dyn_a, 0))[0] == DATA_BLOCKS[0][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 1))[0] == DATA_BLOCKS[3][0]);

    assert(dyn_array_insert_sorted(dyn_a, DATA_BLOCKS[1], &block_compare));
    assert(((uint8_t *) dyn_array_at(dyn_a, 0))[0] == DATA_BLOCKS[0][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 1))[0] == DATA_BLOCKS[1][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 2))[0] == DATA_BLOCKS[3][0]);

    assert(dyn_array_insert_sorted(dyn_a, DATA_BLOCKS[5], &block_compare));
    assert(((uint8_t *) dyn_array_at(dyn_a, 0))[0] == DATA_BLOCKS[0][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 1))[0] == DATA_BLOCKS[1][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 2))[0] == DATA_BLOCKS[3][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 3))[0] == DATA_BLOCKS[5][0]);

    // INSERT_SORTED 3
    assert(dyn_array_insert_sorted(NULL, DATA_BLOCKS[4], &block_compare) == false);

    // INSERT_SORTED 4
    assert(dyn_array_insert_sorted(dyn_a, NULL, &block_compare) == false);

    // INSERT_SORTED 5
    assert(dyn_array_insert_sorted(dyn_a, DATA_BLOCKS[5], NULL) == false);

    // INSERT_SORTED complete

    // SORT 1
    // (no actual change to data)
    assert(dyn_array_sort(dyn_a, &block_compare));
    assert(((uint8_t *) dyn_array_at(dyn_a, 0))[0] == DATA_BLOCKS[0][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 1))[0] == DATA_BLOCKS[1][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 2))[0] == DATA_BLOCKS[3][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 3))[0] == DATA_BLOCKS[5][0]);

    // invert contents
    assert(dyn_array_sort(dyn_a, &block_compare_inv));
    assert(((uint8_t *) dyn_array_at(dyn_a, 3))[0] == DATA_BLOCKS[0][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 2))[0] == DATA_BLOCKS[1][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 1))[0] == DATA_BLOCKS[3][0]);
    assert(((uint8_t *) dyn_array_at(dyn_a, 0))[0] == DATA_BLOCKS[5][0]);

    // SORT 2
    dyn_array_clear(dyn_a);
    dyn_array_push_back(dyn_a, DATA_BLOCKS[2]);

    assert(dyn_array_sort(dyn_a, &block_compare));

    // SORT 3
    assert(dyn_array_sort(NULL, &block_compare) == false);

    // SORT 4
    dyn_array_clear(dyn_a);
    assert(dyn_array_sort(dyn_a, &block_compare) == false);

    // SORT 5
    dyn_array_push_back(dyn_a, DATA_BLOCKS[2]);

    assert(dyn_array_sort(dyn_a, NULL) == false);

    // SORT DONE

    dyn_array_clear(dyn_a);

    int num = 1;

    // FOR_EACH 2
    for_each_counter = 0;
    assert(dyn_array_for_each(dyn_a, &block_for_each, &num));
    assert(for_each_counter == 0);

    // FOR_EACH 4
    assert(dyn_array_for_each(NULL, &block_for_each, &num) == false);

    // FOR_EACH 5
    assert(dyn_array_for_each(dyn_a, NULL, &num) == false);

    assert(dyn_array_insert_sorted(dyn_a, DATA_BLOCKS[0], &block_compare));

    // FOR EACH 1
    assert(dyn_array_for_each(dyn_a, &block_for_each, &num));
    assert(for_each_counter == 1);

    assert(dyn_array_insert_sorted(dyn_a, DATA_BLOCKS[1], &block_compare));

    for_each_counter = 0;

    assert(dyn_array_for_each(dyn_a, &block_for_each, &num));
    assert(for_each_counter == 2);

    // FOR_EACH 3
    for_each_counter = 0;
    assert(dyn_array_for_each(dyn_a, &block_for_each, NULL));
    assert(for_each_counter == 20);

    // FOR EACH tested and cleared for use

    dyn_array_destroy(dyn_a);
}
