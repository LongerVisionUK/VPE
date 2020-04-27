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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "base_type.h"
#include "vpi_video_enc_common.h"
#include "vpi_error.h"
#include "vpi_log.h"

/**
 * Parse the string to get the parameter name and value
 * The format of the input string is "NAMEx=VALUEy:" or "NAMEx=VALUEy". After parsing,
 * the NAMEx is output by param_name and the VALUEy is output by param_val
 *
 * @param string The input string for parsing. The format is "NAMEx=VALUEy:" or "NAMEx=VALUEy"
 * @param param_name The output of the NAMEx in the string. strlen(param_name) is 0 means no valid param_name
 * @param param_val The output of the VALUEy in the string. strlen(param_val) is 0 means no valid param_val
 * @return str The end location of the string. strlen(str) is 0 means parsing to the end of the string.
 */
char *vpi_enc_get_paraname_paravalue(char *string, char *param_name,
                                     char *param_val)
{
    int equal_pos = 0, colon_pos = 0, i = 0;
    char *str = string;

    for (i = 0; i < strlen(string); i++) {
        equal_pos = strspn(str, "=");
        if (equal_pos != 0)
            break;
        str++;
    }
    if (i >=
        (strlen(string) -
         1)) { /*The "=" is at the end of the string. Or there is no "=" in the string. Both are illegal */
        VPILOGE("%s,%d,The string format isn't right,string is %s,i = %d, strlen(string):%ld, str=%s, strlen(str)=%ld\n",
                __FUNCTION__, __LINE__, string, i, strlen(string), str,
                strlen(str));
        param_name[0] = '\0';
        param_val[0]  = '\0';
        return str;
    }
    equal_pos = i; /*equal_pos is the offset of the "=" at the string */
    strncpy(param_name, string, equal_pos); /*copy the name string before "=" */
    param_name[equal_pos] = '\0';

    str = string;
    for (i = 0; i < strlen(string); i++) {
        colon_pos = strspn(str, ":");
        if (colon_pos != 0)
            break;
        str++;
    }
    colon_pos =
        i; /*colon_pos is the offset of the ":" at the string or the end of the string without ":" */

    memcpy(param_val, string + equal_pos + 1,
           colon_pos - equal_pos -
               1); /*copy the value string between "=" and ":". "string+equal_pos+1" is the next char after "=", it is also the first char of the value."colon_pos-1" is the lastest char before ":",equal_pos is the the offset of the "=". So "colon_pos-equal_pos-1" is the length of the value */
    param_val[colon_pos - equal_pos - 1] = '\0';
    return str;
}

/*
 * This function was used to parse long string based parameters
 * into structure
 *
 * Input: string combined with "NAMEx=VALUEy:", for example below string:
 * "bitrate_window=180:intrs_pic_rate=60"
 *
 * Output: pointer to encoder setting structure, like VpiEncVp9Setting
 */
int vpi_enc_parse_param(char *src, VpiEncSetting setting[], int length,
                        void *output)
{
    int ret     = 0;
    char *p     = src;
    size_t pos1 = 0, pos2 = 0;
    char name[255] = { "\0" };
    char val[255]  = { "\0" };
    int v = 0, i = 0;
    VpiEncSetting *op = NULL;

    if (setting == NULL || output == NULL) {
        return -1;
    }

    while (1) {
        p = vpi_enc_get_paraname_paravalue(p, name, val);
        if (strlen(name) != 0 && strlen(val) != 0) {
            /* Step 2, Match name */
            for (i = 0; i < length; i++) {
                if (!strcmp(setting[i].name, name)) {
                    /*Matched */
                    op = &setting[i];
                    break;
                }
            }
            p++;

            /* Step 3, get value */
            if (i == length) {
                VPILOGE("Can't find opition %s\n", name);
                ret = VPI_ERR_VALUE;
            } else {
                v = atoi(val);
                if (v >= op->min && v < op->max) {
                    VPILOGD("Str and value matched: %s=%d\n", name, v);
                    u32 *pp = (u32 *)output + op->offset;
                    *pp     = v;
                } else {
                    VPILOGD("Value for %s is not valid: [%d-%d]\n", name,
                            op->min, op->max);
                    ret = VPI_ERR_VALUE;
                }
            }
            if (strlen(p) == 0) {
                VPILOGD("parse to the end of the string\n");
                break;
            }
        } else {
            VPILOGD("no valid param or name at %s\n", src);
            break;
        }
    }

    return ret;
}
