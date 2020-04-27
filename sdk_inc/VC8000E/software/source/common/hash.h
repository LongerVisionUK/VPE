/*------------------------------------------------------------------------------
-- --
--       This software is confidential and proprietary and may be used --
--        only as expressly authorized by a licensing agreement from --
-- --
--                            Verisilicon. --
-- --
--                   (C) COPYRIGHT 2017 VERISILICON --
--                            ALL RIGHTS RESERVED --
-- --
--                 The entire notice above must be reproduced --
--                  on all copies and should not be removed. --
-- --
--------------------------------------------------------------------------------*/

#ifndef HASH_H
#define HASH_H

#include "crc.h"
#include "checksum.h"

typedef struct {
    unsigned int hash_type;
    union {
        crc32_ctx crc32_ctx;
        checksum_ctx checksum_ctx;
    } _ctx;
} hashctx;
void hash_init(hashctx *ctx, unsigned int hash_type);
unsigned int hash(hashctx *ctx, unsigned char *data, int len);
unsigned int hash_finalize(hashctx *ctx);
void hash_reset(hashctx *ctx, int init_hash, int offset);
void hash_getstate(hashctx *ctx, unsigned int *hash, int *offset);

#endif
