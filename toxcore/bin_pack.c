/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022-2025 The TokTok team.
 */

#include "bin_pack.h"

#include <assert.h>
#include <string.h>

#include "../third_party/cmp/cmp.h"
#include "attributes.h"
#include "ccompat.h"
#include "logger.h"

struct Bin_Pack {
    uint8_t *bytes;
    uint32_t bytes_size;
    uint32_t bytes_pos;
    cmp_ctx_t ctx;
};

static bool null_reader(cmp_ctx_t *_Nonnull ctx, void *_Nonnull data, size_t limit)
{
    assert(limit == 0);
    return false;
}

static bool null_skipper(cmp_ctx_t *_Nonnull ctx, size_t count)
{
    assert(count == 0);
    return false;
}

static size_t buf_writer(cmp_ctx_t *_Nonnull ctx, const void *_Nonnull data, size_t count)
{
    const uint8_t *bytes = (const uint8_t *)data;
    Bin_Pack *bp = (Bin_Pack *)ctx->buf;
    assert(bp != nullptr);
    const uint32_t new_pos = bp->bytes_pos + count;
    if (new_pos < bp->bytes_pos) {
        // 32 bit overflow.
        return 0;
    }
    if (bp->bytes != nullptr) {
        if (new_pos > bp->bytes_size) {
            // Buffer too small.
            return 0;
        }
        memcpy(&bp->bytes[bp->bytes_pos], bytes, count);
    }
    bp->bytes_pos += count;
    return count;
}

static void bin_pack_init(Bin_Pack *_Nonnull bp, uint8_t *_Nullable buf, uint32_t buf_size)
{
    bp->bytes = buf;
    bp->bytes_size = buf_size;
    bp->bytes_pos = 0;
    cmp_init(&bp->ctx, bp, null_reader, null_skipper, buf_writer);
}

uint32_t bin_pack_obj_size(bin_pack_cb *callback, const void *obj, const Logger *logger)
{
    Bin_Pack bp;
    bin_pack_init(&bp, nullptr, 0);
    if (!callback(obj, logger, &bp)) {
        return UINT32_MAX;
    }
    return bp.bytes_pos;
}

bool bin_pack_obj(bin_pack_cb *callback, const void *obj, const Logger *logger, uint8_t *buf, uint32_t buf_size)
{
    Bin_Pack bp;
    bin_pack_init(&bp, buf, buf_size);
    return callback(obj, logger, &bp);
}

uint32_t bin_pack_obj_array_b_size(bin_pack_array_cb *callback, const void *arr, uint32_t arr_size, const Logger *logger)
{
    Bin_Pack bp;
    bin_pack_init(&bp, nullptr, 0);
    if (arr == nullptr) {
        assert(arr_size == 0);
    }
    for (uint32_t i = 0; i < arr_size; ++i) {
        if (!callback(arr, i, logger, &bp)) {
            return UINT32_MAX;
        }
    }
    return bp.bytes_pos;
}

bool bin_pack_obj_array_b(bin_pack_array_cb *callback, const void *arr, uint32_t arr_size, const Logger *logger, uint8_t *buf, uint32_t buf_size)
{
    Bin_Pack bp;
    bin_pack_init(&bp, buf, buf_size);
    if (arr == nullptr) {
        assert(arr_size == 0);
    }
    for (uint32_t i = 0; i < arr_size; ++i) {
        if (!callback(arr, i, logger, &bp)) {
            return false;
        }
    }
    return true;
}

bool bin_pack_obj_array(Bin_Pack *bp, bin_pack_array_cb *callback, const void *arr, uint32_t arr_size, const Logger *logger)
{
    if (arr == nullptr) {
        assert(arr_size == 0);
        return bin_pack_array(bp, 0);
    }

    if (!bin_pack_array(bp, arr_size)) {
        return false;
    }

    for (uint32_t i = 0; i < arr_size; ++i) {
        if (!callback(arr, i, logger, bp)) {
            return false;
        }
    }

    return true;
}

bool bin_pack_array(Bin_Pack *bp, uint32_t size)
{
    return cmp_write_array(&bp->ctx, size);
}

bool bin_pack_bool(Bin_Pack *bp, bool val)
{
    return cmp_write_bool(&bp->ctx, val);
}

bool bin_pack_u08(Bin_Pack *bp, uint8_t val)
{
    return cmp_write_uinteger(&bp->ctx, val);
}

bool bin_pack_u16(Bin_Pack *bp, uint16_t val)
{
    return cmp_write_uinteger(&bp->ctx, val);
}

bool bin_pack_u32(Bin_Pack *bp, uint32_t val)
{
    return cmp_write_uinteger(&bp->ctx, val);
}

bool bin_pack_u64(Bin_Pack *bp, uint64_t val)
{
    return cmp_write_uinteger(&bp->ctx, val);
}

bool bin_pack_bin(Bin_Pack *bp, const uint8_t *data, uint32_t length)
{
    return cmp_write_bin(&bp->ctx, data, length);
}

bool bin_pack_nil(Bin_Pack *bp)
{
    return cmp_write_nil(&bp->ctx);
}

bool bin_pack_bin_marker(Bin_Pack *bp, uint32_t size)
{
    return cmp_write_bin_marker(&bp->ctx, size);
}

bool bin_pack_u08_b(Bin_Pack *bp, uint8_t val)
{
    return bp->ctx.write(&bp->ctx, &val, 1) == 1;
}

bool bin_pack_u16_b(Bin_Pack *bp, uint16_t val)
{
    return bin_pack_u08_b(bp, (val >> 8) & 0xff)
           && bin_pack_u08_b(bp, val & 0xff);
}

bool bin_pack_u32_b(Bin_Pack *bp, uint32_t val)
{
    return bin_pack_u16_b(bp, (val >> 16) & 0xffff)
           && bin_pack_u16_b(bp, val & 0xffff);
}

bool bin_pack_u64_b(Bin_Pack *bp, uint64_t val)
{
    return bin_pack_u32_b(bp, (val >> 32) & 0xffffffff)
           && bin_pack_u32_b(bp, val & 0xffffffff);
}

bool bin_pack_bin_b(Bin_Pack *bp, const uint8_t *data, uint32_t length)
{
    return bp->ctx.write(&bp->ctx, data, length) == length;
}
