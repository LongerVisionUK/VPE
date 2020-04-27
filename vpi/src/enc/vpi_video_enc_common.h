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

#ifndef __ENC_COMMON__
#define __ENC_COMMON__

#include "vpe_types.h"

typedef struct {
    const char *name;
    int offset;
    double min;
    double max;
} VpiEncSetting;

char *vpi_enc_get_paraname_paravalue(char *string, char *param_name,
                                     char *param_val);
int vpi_enc_parse_param(char *src, VpiEncSetting setting[], int length,
                        void *output);

#endif
