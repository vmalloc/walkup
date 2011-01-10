#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

enum {
    ARGV_PROGNAME,
    ARGV_RELPATH,
    ARGV_NUM_ARGS,
};

enum {
    ERR_SUCCESS,
    ERR_NOT_FOUND,
    ERR_SYSTEM_ERROR,
};

static int _is_same_path(const struct stat * a, const struct stat * b)
{
    return a->st_dev == b->st_dev && a->st_ino == b->st_ino;
}

static int _go_up_one_directory(void)
{
    if (0 != chdir("..")) {
        perror("Cannot chdir");
        return ERR_SYSTEM_ERROR;
    }
    return ERR_SUCCESS;
}

static int _find_path(const char * path, const char * end, char ** name)
{
    struct stat end_stat;
    struct stat cwd_stat;
    struct stat needle;
    int retval;

    if (0 > stat(end, &end_stat)) {
        perror("Cannot stat root");
        return ERR_SYSTEM_ERROR;
    }
    
    do {    
        if (0 > stat(path, &needle)) {
            if (errno != ENOENT) {
                perror("Cannot stat");
                return ERR_SYSTEM_ERROR;
            }
        }
        else {
            *name = getcwd(NULL, 0);
            if (NULL == *name) {
                perror("Cannot get current directory");
                return ERR_SYSTEM_ERROR;
            }
            return ERR_SUCCESS;
        }

        retval = _go_up_one_directory();
        if (ERR_SUCCESS != retval) {
            return retval;
        }

        if (0 != stat(".", &cwd_stat)) {
            perror("Cannot stat cwd");
            return ERR_SYSTEM_ERROR;
        }
    } while (!_is_same_path(&cwd_stat, &end_stat));

    return ERR_NOT_FOUND;
}
    
int main(int argc, const char * const argv[])
{
    int retval;
    char * name = NULL;

    if (argc != ARGV_NUM_ARGS) {
        fprintf(stderr, "Invalid usage. Usage: %s RELPATH\n", argv[0]);
        return ERR_SYSTEM_ERROR;
    }

    retval = _find_path(argv[ARGV_RELPATH], "/", &name);
    if (ERR_SUCCESS == retval) {
        printf("%s/%s\n", name, argv[ARGV_RELPATH]);
    }
    
    free(name);
    return retval;

}

