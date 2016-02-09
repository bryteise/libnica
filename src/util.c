/*
 * This file is part of libthingamabob.
 *
 * Copyright (C) 2016 Intel Corporation
 *
 * libthingamabob is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */


#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/param.h>
#include <assert.h>

#include "util.h"

void dump_file_descriptor_leaks(void)
{
        DIR *dir = NULL;
        struct dirent *entry = NULL;

        dir = opendir("/proc/self/fd");
        if (!dir) {
                return;
        }

        while ((entry = readdir(dir)) != NULL) {
                autofree(char) *filename = NULL;
                char buffer[PATH_MAX + 1] = { 0 };
                size_t size;

                if (streq(entry->d_name, ".")) {
                        continue;
                }
                if (streq(entry->d_name, "..")) {
                        continue;
                }
                /* skip stdin/out/err */
                if (streq(entry->d_name, "0")) {
                        continue;
                }
                if (streq(entry->d_name, "1")) {
                        continue;
                }
                if (streq(entry->d_name, "2")) {
                        continue;
                }

                /* we hold an fd open, the one from opendir above */
                sprintf(buffer, "%i", dirfd(dir));
                if (streq(entry->d_name, buffer)) {
                        continue;
                }

                if (asprintf(&filename, "/proc/self/fd/%s", entry->d_name) <= 0) {
                        abort();
                }

                memset(&buffer, 0, sizeof(buffer));
                size = readlink(filename, buffer, PATH_MAX);
                if (size) {
                        fprintf(stderr, "Possible filedescriptor leak : %s (%s)\n", entry->d_name, buffer);
                }
        }

        closedir(dir);
}


/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */