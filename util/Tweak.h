/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/

/* sac_tweak - v0.1

    Live hot-reload of variables for C/C++

    Supported Platforms:
     - Linux (depends on pthread)

    Do this:
       #define SAC_TWEAK_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation.

    3 supported value types:
      TWEAK(int, sec) = 1;
      TWEAK(char_ptr, who) = "world";
      TWEAK(float, value) = 2500;

    TBD:
      - other platforms
      - optionnal threading
      - remove char_ptr hack
*/

typedef const char* char_ptr;

#ifndef TWEAK

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

/* Public API */
/* Supported types of variable. Use one of these as the first argument to TWEAK */
enum __sac_tweak_type {
    type_int,
    type_float,
    type_char_ptr
};
/* Possible usages:
       TWEAK(int, my_int) = 3;
       TWEAK(float, my_float_var) = 1.2f;
       TWEAK(char_ptr, my_string) = "hello";
    Note: use char_ptr because char* will break macro expansions.
*/
#define TWEAK(T, name) T name; \
    if (!__sac_tweak_##T(\
        #name, \
        __FILE__, \
        __sac_tweak_id( \
            #name, \
            __FILE__, \
            type_##T, \
            __SAC_XSTR(__SAC_LABEL(__LINE__, _id))), \
        &name)) name

/* Call this to override default value of tweaks. Default prefix is --t_ so using the same examples as above:
       --t_my_int=3
       --t_my_float_var=1.2
       --t_my_string=hello
    You can replace the prefix by defining TWEAK_ARG_PREFIX
*/
void tweak_consume_command_line_args(int argc, char** argv);

#ifndef TWEAK_ARG_PREFIX
#define TWEAK_ARG_PREFIX "--t_"
#endif


/* Private macro foo */
#define __SAC_MERGE_(b, c)  b##c
#define __SAC_LABEL(b, c) __SAC_MERGE_(b, c)
#define __SAC_STR(s) #s
#define __SAC_XSTR(s) __SAC_STR(s)


extern int __sac_tweak_id(const char* name, const char* file, enum __sac_tweak_type type, const char* id);

extern bool __sac_tweak_float(const char* name, const char* file, int id, float* out);
extern bool   __sac_tweak_int(const char* name, const char* file, int id, int* out);
extern bool __sac_tweak_char_ptr(const char* name, const char* file, int id, const char** out);

#ifdef SAC_TWEAK_IMPLEMENTATION

#include <sys/inotify.h>
#include <string.h>
#include <unistd.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct __sac_tweak_datas {
    int watch;
    const char* unique_id;
    const char* name;
    const char* filename;
    enum __sac_tweak_type type;
    union __sac_tweak_value {
        int i;
        float f;
        char* s;
    } value;
    bool is_valid;
};

struct __sac_tweak_default_value {
    const char* name;
    const char* value;
};

#define __SAC_MAX_TWEAKS 1024
static struct __sac_global_datas {
    pthread_t th;
    int fd;
    struct __sac_tweak_datas tweaks[__SAC_MAX_TWEAKS];
    int tweak_count;
    int self_pipe[2];
    int default_count;
    struct __sac_tweak_default_value* defaults;
}* datas;

static pthread_mutex_t __sac_fastmutex = PTHREAD_MUTEX_INITIALIZER;

static char* __sac_remove_suffix_spaces(char* text) {
    while(*text == ' ' || *text == '\t') text--;
    return text;
}
static char* __sac_remove_prefix_spaces(char* text) {
    while(*text == ' ' || *text == '\t') text++;
    return text;
}

static const char* __sac_read_value_from_file(const char* name, const char* file) {
    char* content;

    FILE* f = fopen(file, "r");
    if (f == NULL) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    int content_size = ftell(f);
    content = (char*)malloc(content_size + 1);
    if (content == NULL) {
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    fread(content, 1, content_size, f);
    content[content_size] = '\0';

    char* tweak_start = content;
    char* result = NULL;
    while ((tweak_start = strstr(tweak_start, "TWEAK"))) {
        tweak_start++;

        /* next '(' */
        char* paren_start = strchr(tweak_start, '(');
        if (!paren_start) {
            continue;
        }

        /* next ')' */
        char* paren_end = strchr(paren_start, ')');
        if (!paren_end) {
            continue;
        }
        paren_end = __sac_remove_suffix_spaces(paren_end - 1);

        char* comma = (char*)memchr(paren_start + 1, ',', paren_end - paren_start + 1);

        if (!comma) {
            continue;
        }
        char* parsed_name = __sac_remove_prefix_spaces(comma + 1);

        /* is correct id? */
        if (strncmp(parsed_name, name, strlen(name)) == 0) {
            /* value is between next = and ; */
            char* next_equal = strchr(paren_end, '=');
            if (!next_equal) {
                continue;
            }
            char* next_semi_colon = strchr(next_equal, ';');
            if (!next_semi_colon) {
                continue;
            }
            next_equal++;
            next_semi_colon--;
            int length = next_semi_colon - next_equal + 1;
            result = strndup(next_equal, length);
            break;
        }
    }

    fclose(f);
    free(content);
    return result;
}

static void __sac_update_tweak(int id, const char* value) {
    const char* raw = value ? value : __sac_read_value_from_file(datas->tweaks[id].name, datas->tweaks[id].filename);
    if (raw) {
        switch (datas->tweaks[id].type) {
            case type_int: {
                char* endptr = NULL;
                long val = strtol(raw, &endptr, 0);
                if (endptr != raw) {
                    datas->tweaks[id].value.i = (int)val;
                    datas->tweaks[id].is_valid = true;
                }
            } break;
            case type_float: {
                char* endptr = NULL;
                float val = strtof(raw, &endptr);
                if (endptr != raw) {
                    datas->tweaks[id].value.f = val;
                    datas->tweaks[id].is_valid = true;
                }
            } break;
            case type_char_ptr: {
                const size_t len = strlen(raw);
                const char* start = strchr(raw, '"');
                /* strip quotes */
                if (start && len && raw[len-1] == '"') {
                    datas->tweaks[id].value.s = strndup(start + 1, strlen(start) - 2);
                    datas->tweaks[id].is_valid = true;
                } else {
                    datas->tweaks[id].value.s = strdup(raw);
                    datas->tweaks[id].is_valid = true;
                }
            } break;
        }
    }
}

static void* __sac_update_loop(void* args);

static bool __sac_tweak_global_init() {
    datas = (struct __sac_global_datas*) malloc(sizeof(struct __sac_global_datas));
    if (datas == NULL) {
        goto error_malloc;
    }
    datas->fd = inotify_init();
    if (datas->fd == -1) {
        goto error_inotify;
    }
    datas->tweak_count = 0;

    if (pipe2(datas->self_pipe, O_NONBLOCK) != 0) {
        goto error_pipe;
    }

    if (pthread_create(&datas->th, NULL, __sac_update_loop, NULL) != 0) {
        goto error_thread;
    }
    pthread_detach(datas->th);

    datas->default_count = 0;
    datas->defaults = NULL;

    return true;

error_thread:
    close(datas->self_pipe[0]);
    close(datas->self_pipe[1]);
error_pipe:
    close(datas->fd);
error_inotify:
    free (datas);
error_malloc:
    return false;

}

int __sac_tweak_id(const char* name, const char* file, enum __sac_tweak_type type, const char* unique_id) {
    char tmp[512];
    snprintf(tmp, 512, "%s%s", unique_id, file);
    tmp[511] = '\0';

    pthread_mutex_lock(&__sac_fastmutex);
    if (!datas) {
        if (!__sac_tweak_global_init()) {
            pthread_mutex_unlock(&__sac_fastmutex);
            return -1;
        }
    }

    for (int i=0; i<datas->tweak_count; i++) {
        if (strcmp(unique_id, datas->tweaks[i].unique_id) == 0) {
            pthread_mutex_unlock(&__sac_fastmutex);
            return i;
        }
    }

    /* unblock select */
    write(datas->self_pipe[1], "a", 1);

    int id = datas->tweak_count++;
    if (id == __SAC_MAX_TWEAKS) {
        pthread_mutex_unlock(&__sac_fastmutex);
        return 0;
    }

    datas->tweaks[id].unique_id = strdup(unique_id);
    datas->tweaks[id].watch = inotify_add_watch(datas->fd, file, IN_CLOSE_WRITE);
    datas->tweaks[id].name = name;
    datas->tweaks[id].filename = file;
    datas->tweaks[id].type = type;
    datas->tweaks[id].is_valid = false;

    const char* default_value = NULL;
    for (int i=0; i<datas->default_count; i++) {
        if (strcmp(datas->defaults[i].name, name) == 0) {
            default_value = datas->defaults[i].value;
        }
    }
    __sac_update_tweak(id, default_value);
    pthread_mutex_unlock(&__sac_fastmutex);
    return id;
}

void tweak_consume_command_line_args(int argc, char** argv) {
    if (!datas) {
        if (!__sac_tweak_global_init()) {
            return;
        }
    }

    int count = 0;
    const size_t prefix_len = strlen(TWEAK_ARG_PREFIX);
    /* clone argument with for form '--XXXXX=XXXX' */
    for (int i=0; i<argc; i++) {
        size_t len = strlen(argv[i]);
        if (len <= prefix_len) {
            continue;
        }
        if (memcmp(argv[i], TWEAK_ARG_PREFIX, prefix_len) != 0) {
            continue;
        }

        const char* equal = strchr(&argv[i][prefix_len], '=');
        if (equal == NULL) {
            continue;
        }
        datas->defaults = (struct __sac_tweak_default_value*) realloc(
            datas->defaults,
            (1 + count) * sizeof(struct __sac_tweak_default_value));
        datas->defaults[count].name = strndup(&argv[i][prefix_len], equal - argv[i] - prefix_len);
        datas->defaults[count].value = strdup(equal + 1);
        count++;
    }
    datas->default_count = count;
}

static void* __sac_update_loop(void* args) {
    fd_set fds;
    int nfds;
    char buffer[sizeof(struct inotify_event)];

    nfds = 1 + ((datas->self_pipe[0] > datas->fd) ? datas->self_pipe[0] : datas->fd);

    while (1) {

        FD_ZERO(&fds);
        FD_SET(datas->fd, &fds);
        FD_SET(datas->self_pipe[0], &fds);

        int modified = select(nfds, &fds, NULL, NULL, NULL);
        pthread_mutex_lock(&__sac_fastmutex);
        if (FD_ISSET(datas->self_pipe[0], &fds)) {
            while (read(datas->self_pipe[0], buffer, 1) > 0) { }
            modified--;
        }
        for (int i=0; i<modified; i++) {
            if (read(datas->fd, buffer, sizeof(struct inotify_event)) > 0) {
                struct inotify_event *event = (struct inotify_event *) buffer;

                for (int j=0; j<datas->tweak_count; j++) {
                    if (datas->tweaks[j].watch == event->wd) {
                        __sac_update_tweak(j, NULL);
                    }
                }

            }
        }
        pthread_mutex_unlock(&__sac_fastmutex);
    }
}

bool __sac_tweak_float(const char* name, const char* file, int id, float* out) {
    if (id < 0) {
        return false;
    }
    pthread_mutex_lock(&__sac_fastmutex);
    const bool is_valid = datas->tweaks[id].is_valid;
    if (is_valid) {
        *out = datas->tweaks[id].value.f;
    }
    pthread_mutex_unlock(&__sac_fastmutex);
    return is_valid;
}

bool __sac_tweak_int(const char* name, const char* file, int id, int* out) {
    if (id < 0) {
        return false;
    }
    pthread_mutex_lock(&__sac_fastmutex);
    const bool is_valid = datas->tweaks[id].is_valid;
    if (is_valid) {
        *out = datas->tweaks[id].value.i;
    }
    pthread_mutex_unlock(&__sac_fastmutex);
    return is_valid;
}

bool __sac_tweak_char_ptr(const char* name, const char* file, int id, const char** out) {
    if (id < 0) {
        return false;
    }
    pthread_mutex_lock(&__sac_fastmutex);
    const bool is_valid = datas->tweaks[id].is_valid;
    if (is_valid) {
        *out = datas->tweaks[id].value.s;
    }

    pthread_mutex_unlock(&__sac_fastmutex);
    return is_valid;
}

#endif /* SAC_TWEAK_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* TWEAK */