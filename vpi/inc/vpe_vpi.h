/*
 * Copyright 2019 VeriSilicon, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __VPE_VPI_H__
#define __VPE_VPI_H__

/*
 * provides application programming interface for the application layer.
 */

#include "vpe_types.h"

/**
 * @brief Create empty context structure and api function pointers.
 *        Use functions in VpeApi to access vpe services.
 * @param ctx pointer of the vpe context
 * @param vpi pointer of the vpe api function
 * @param plugin vpe plugin type
 */

int vpe_create(VpeCtx *ctx, VpeApi **vpi, VpePlugin plugin);

/**
 * @brief Destroy vpe context and free both context and vpi structure
 * @param ctx The context of vpe
 */

int vpe_destroy(VpeCtx ctx);

#endif
