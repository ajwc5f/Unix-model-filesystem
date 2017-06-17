#include <iostream>
#include <cstddef>
#include <cstring>
#include "gtest/gtest.h"

#include "block_store.h"

TEST(bs_create_open, null_fname) {
    block_store_t *res = block_store_create(NULL);
    ASSERT_EQ(nullptr, res);

    res = block_store_open(NULL);
    ASSERT_EQ(nullptr, res);
}

TEST(bs_create_close, basic) {
    block_store_t *res = block_store_create("test_a.bs");
    ASSERT_NE(nullptr, res);

    block_store_close(res);
}

TEST(bs_destroy, null_object) {
    block_store_close(NULL);
    // Congrats, you didn't crash!
}


TEST(bs_read, bad_values) {
    block_store_t *bs = block_store_create("test_c.bs");

    ASSERT_NE(nullptr, bs);

    uint8_t block[512];

    // make sure we can't touch the FBM
    for (unsigned i = 0; i < 16; ++i) {
        ASSERT_FALSE(block_store_read(bs, i, block));
    }

    unsigned block_a = block_store_allocate(bs);

    ASSERT_FALSE(block_store_read(bs, block_a, NULL));

    ASSERT_FALSE(block_store_read(NULL, block_a, block));

    block_store_close(bs);
}

TEST(bs_write, bad_values) {
    block_store_t *bs = block_store_create("test_d.bs");

    ASSERT_NE(nullptr, bs);

    uint8_t block[512];

    // make sure we can't touch the FBM
    for (unsigned i = 0; i < 16; ++i) {
        ASSERT_FALSE(block_store_write(bs, i, block));
    }

    unsigned block_a = block_store_allocate(bs);

    ASSERT_FALSE(block_store_write(bs, block_a, NULL));

    ASSERT_FALSE(block_store_write(NULL, block_a, block));

    block_store_close(bs);
}

TEST(bs_a_lot, basic_use) {
    // pretty much do everything
    // read: did you save the fbm changes? data?
    block_store_t *bs = block_store_create("test_b.bs");
    ASSERT_NE(nullptr, bs);

    unsigned block_a = block_store_allocate(bs);
    ASSERT_NE(0, block_a);

    unsigned block_b = 548;
    ASSERT_TRUE(block_store_request(bs, block_b));

    uint8_t data_blocks[4][512];
    memset(data_blocks[0], 0x05, 512);
    memset(data_blocks[1], 0xFF, 512);
    memset(data_blocks[2], 0x00, 512);

    ASSERT_TRUE(block_store_read(bs, block_a, data_blocks[3]));

    // was it zero-init'd?
    ASSERT_EQ(0, memcmp(data_blocks[2], data_blocks[3], 512));

    ASSERT_TRUE(block_store_read(bs, block_b, data_blocks[3]));

    // was it zero-init'd? Did you just get lucky last time?
    ASSERT_EQ(0, memcmp(data_blocks[2], data_blocks[3], 512));

    // write data out
    ASSERT_TRUE(block_store_write(bs, block_a, data_blocks[0]));
    ASSERT_TRUE(block_store_write(bs, block_b, data_blocks[1]));

    block_store_close(bs);

    bs = block_store_open("test_b.bs");

    // Did the FBM get saved?
    ASSERT_FALSE(block_store_request(bs, block_a));
    ASSERT_FALSE(block_store_request(bs, block_b));

    // was the data saved?
    ASSERT_TRUE(block_store_read(bs, block_a, data_blocks[3]));
    ASSERT_EQ(0, memcmp(data_blocks[0], data_blocks[3], 512));

    ASSERT_TRUE(block_store_read(bs, block_b, data_blocks[3]));
    ASSERT_EQ(0, memcmp(data_blocks[1], data_blocks[3], 512));

    // free then re-request
    block_store_release(bs, block_a);
    block_store_release(bs, block_b);
    ASSERT_TRUE(block_store_request(bs, block_a));
    ASSERT_TRUE(block_store_request(bs, block_b));

    block_store_close(bs);
}

TEST(bs_request_release, fbm_attack) {
    uint8_t buffer[512];
    block_store_t *bs = block_store_create("test_k.bs");  // I forgot what letter I was on
    for (unsigned i = 0; i < 16; ++i) {
        block_store_release(bs, i);
        ASSERT_FALSE(block_store_request(bs, i));
        ASSERT_FALSE(block_store_read(bs,i,buffer));
        ASSERT_FALSE(block_store_write(bs,i,buffer));
    }
    block_store_close(bs);
}

TEST(bs_request, fill_device) {
    block_store_t *bs = block_store_create("test_l.bs");
    for (unsigned i = 16; i < 65536; ++i) {
        ASSERT_TRUE(block_store_request(bs, i));
    }
    ASSERT_EQ(block_store_allocate(bs), 0);
    block_store_close(bs);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
