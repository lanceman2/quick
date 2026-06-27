#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/panels.h"
#include "../include/quickplot.h"
#include "../include/debug.h"

#include "findGrapher.h"



static void *dlhandle = 0;


void cleanupGrapher(void) {

    if(dlhandle) {
        void (*qp_cleanup)(void) = dlsym(dlhandle, "qp_cleanup");
        if(qp_cleanup)
            qp_cleanup();
        dlclose(dlhandle);
        dlhandle = 0;
    }
}

// Return true on success.
static inline bool RunDso(struct PnWidget *parent,
        const char *dsoPath) {

    DASSERT(!dlhandle);

    dlhandle = dlopen(dsoPath, RTLD_NOW|RTLD_LOCAL);

    if(!dlhandle) {
        NOTICE("dlopen(\"%s\",) failed: dlerror()=\"%s\"",
                dsoPath, dlerror());
        return false;
    }

    void (*Graph)(struct PnWidget *parent);

    const char *FUNC_NAME = "qp_graph";

    Graph = dlsym(dlhandle, FUNC_NAME);

    if(!Graph) {
        ERROR("dlsym(\"%s\",) failed: dlerror()=\"%s\"",
                FUNC_NAME, dlerror());
        cleanupGrapher();
        return false;
    }

    Graph(parent);

    // cleanupGrapher() is called later so we may keep any state that may
    // have been created by loading and running Graph() from dsoPath.

    return true; // success
}

// We assume that if this program is in PREFIX/bin/ (or PREFIX/Any_Thing/)
// then panels.h is in PREFIX/include/
//
// Returns a pointer to allocated memory that must be free()ed.
//
char *getIncludeDir(void) {

    const char *exe = "/proc/self/exe";
    char *bin = realpath(exe, 0);
    ASSERT(bin, "realpath(\"%s\",0) failed", exe);
    char *s = bin + strlen(bin);
    for(; *s != '/' && s != bin; --s) *s = '\0';
    ASSERT(*s == '/');
    for(*(--s) = '\0'; *s != '/' && s != bin; --s) *s = '\0';
    ASSERT(*s == '/');

    const char *include = "include";
    const size_t Len = strlen(bin) + strlen(include) + 1;
    char *incdir = malloc(Len);
    ASSERT(incdir, "malloc(%zu) failed", Len);
    strcpy(incdir, bin);
    strcat(incdir + strlen(bin), include);
    DZMEM(bin, strlen(bin));
    free(bin);
    return incdir;
}


// Returns true on success
//
static inline bool
Compile(const char *cPath, const char *dsoPath) {

    char *incdir = getIncludeDir();
    const char *fmt = "gcc -Wall -Werror -fPIC -I%s"
        " -shared -o %s %s -ldl -lm";
    const size_t Len = strlen(fmt) +
        strlen(incdir) + strlen(dsoPath) + strlen(cPath);
    char *run = malloc(Len);
    ASSERT(run, "malloc(%zu) failed", Len);

    // setenv(PKG_CONFIG_PATH environment variable

    snprintf(run, Len, fmt, incdir, dsoPath, cPath);
    DZMEM(incdir, strlen(incdir));
    free(incdir);

    errno = 0;
    INFO("running: %s", run);

    bool ret;

    if(system(run) == 0)
        ret = true; // success
    else {
        ret = false;
        WARN("Failed to compile %s", cPath);
    }

    DZMEM(run, strlen(run));
    free(run);

    return ret;
}

// Return true if we need to compile.
static inline bool
CheckDates(const struct stat *c, const struct stat *dso) {

    // struct timespec  st_atim;  /* Time of last access */
    // struct timespec  st_mtim;  /* Time of last modification */
    // struct timespec  st_ctim;  /* Time of last status change */

    const struct timespec *c_t = &c->st_ctim;
    const struct timespec *dso_t = &dso->st_ctim;

    if(c_t->tv_sec > dso_t->tv_sec || (
            c_t->tv_sec == dso_t->tv_sec &&
            c_t->tv_nsec > dso_t->tv_nsec))
        return true;
    return false;
}



// These two C files get installed.
//
#define TEMPLATE_DIR "/quickplot/template/"


// Default editor
//#define QUICKPLOT_EDITOR      "vim"
#define QUICKPLOT_EDITOR      "gvim --nofork"
#define QUICKPLOT_EDITOR_ENV  "QUICKPLOT_EDITOR"



static inline const char *GetEditor(void) {

    char *editor = getenv(QUICKPLOT_EDITOR_ENV);
    if(editor) return editor;
    return QUICKPLOT_EDITOR;
}


// Run editor and wait.
//
static inline void Edit(const char *cfile) {

    const char *editor = GetEditor();
    const char *format = "%s %s";
    const size_t Len = strlen(editor) + strlen(cfile) + strlen(format);
    char *command = calloc(1, Len);
    ASSERT(command, "calloc(1,%zu) failed", Len);

    snprintf(command, Len, format, editor, cfile);
    INFO("Running: %s", command);

    system(command);

    DZMEM(command, strlen(command));
    free(command);
}


static inline bool MakeCFile(const char *cfile, const char *template) {

    bool ret = true;
    if(!template)
        template = "static_func.c";

    char *path = 0;

    if(template[0] != '/') {
        // Not a full path.
        size_t l = strlen(template);
        const char *addSuffix = "";
        if(l <= 2 || template[l-1] != 'c' || template[l-2] != '.')
            addSuffix = ".c";
        const size_t Len = strlen(db_lib_dir) + strlen(TEMPLATE_DIR) +
            strlen(template) + strlen(addSuffix) + 1;
        path = malloc(Len);
        ASSERT(path, "malloc(%zu) failed", Len);
        snprintf(path, Len, "%s%s%s%s",
                db_lib_dir, TEMPLATE_DIR, template, addSuffix);
    } else {
        path = strdup(template);
        ASSERT(path, "strdup() failed");
    }

    WARN("path=\"%s\"", path);

    FILE *rf = fopen(path, "r");
    FILE *wf = 0;

    const size_t BufLen = 1023;
    char buf[BufLen];
    size_t n;

    if(!rf) {
        ERROR("fopen(\"%s\", \"r\") failed", path);
        goto end;
    }

    // We will assume that we can truncate the file.
    wf = fopen(cfile, "w");

    if(!wf) {
        ERROR("fopen(\"%s\", \"w\") failed", path);
        goto end;
    }

    while((n = fread(buf, 1, BufLen, rf))) {
        if(ferror(rf)) {
            ERROR("fread() from \"%s\" failed", path);
            goto end;
        }
        if(n == 0) break;
        size_t wr = fwrite(buf, 1, n, wf);
        if(wr != n) {
            ERROR("fwrite() to \"%s\" failed", cfile);
            goto end;
        }
    }

    INFO("Created file \"%s\"", cfile);

    ret = false; // success

end:

    if(rf)
        fclose(rf);
    if(wf)
        fclose(wf);

    if(path) {
        DZMEM(path, strlen(path));
        free(path);
    }

    if(!ret)
        Edit(cfile);

    return ret;
}


// Returns a positive error status or 0 on success.
//
int TestCFile(const char *cPath,
        struct PnWidget *parent) {

    DASSERT(cPath);
    DASSERT(parent);

    int ret = 0;

    char *tempDir = strdup("/tmp/quickplot_XXXXXX");
    ASSERT(tempDir, "strdup() failed");
    tempDir = mkdtemp(tempDir);

    const char *x_so = "/x.so";
    const size_t len = strlen(tempDir) + strlen(x_so) + 1;
    char *dsoPath = malloc(len);
    ASSERT(dsoPath, "malloc(%zu) failed", len);
    strcpy(dsoPath, tempDir);
    strcpy(dsoPath + strlen(tempDir), x_so);

    if(Compile(cPath, dsoPath)) {
        ret = (RunDso(parent, dsoPath)? 0: 6);
        if(unlink(dsoPath))
            WARN("unlink(\"%s\") failed", dsoPath);
    } else
        ret = 5; // fail

    // It turns out we can remove the temporary directory, even when this
    // process is still using a mapping of the DSO file that was in that
    // directory.
    if(rmdir(tempDir))
        WARN("rmdir(\"%s\") failed", tempDir);

    // If we succeeded above in running the graph plot thingy, the DSO
    // file that we created is only accessible to this process, and we can
    // "remove" the temporary directory that contained it.  When this
    // process exits the DSO file and the temporary directory will
    // be destroyed by the kernel.  So even if this process crashes,
    // these temporary files (DSO and directory) will be cleaned up.

    DZMEM(tempDir, strlen(tempDir));
    free(tempDir);
    DZMEM(dsoPath, strlen(dsoPath));
    free(dsoPath);
    return ret;
}

int testCFile(const char *cPath,
        struct PnWidget *parent) {

    DASSERT(cPath);
    DASSERT(parent);
    return TestCFile(cPath, parent);
}

int testTemplate(const char *template,
        struct PnWidget *parent) {

    DASSERT(template);
    DASSERT(parent);

    char *cPath;
    int ret = 0;
    size_t l = strlen(template);
    ASSERT(l);
    if(l >= 3 && template[l-1] == 'c' &&
            template[l-2] == '.') {
        // template = "*.c"
        const size_t len = strlen(db_lib_dir) + strlen(TEMPLATE_DIR) + l + 1;
        cPath = malloc(len);
        ASSERT(cPath, "malloc(%zu) failed", len);
        sprintf(cPath, "%s%s%s", db_lib_dir, TEMPLATE_DIR, template);
    } else {
        const size_t len = strlen(db_lib_dir) + strlen(TEMPLATE_DIR) + l + 3;
        cPath = malloc(len);
        ASSERT(cPath, "malloc(%zu) failed", len);
        sprintf(cPath, "%s%s%s.c", db_lib_dir, TEMPLATE_DIR, template);
    }

    DSPEW("cPath=\"%s\"", cPath);
 
    ret = TestCFile(cPath, parent);

    DZMEM(cPath, strlen(cPath));
    free(cPath);
    return ret;
}

// Returns < 0 on error
// Returns 0 on success
// Returns > 0 on nothing happened but no failure
//
int findGrapher(const char *filename, const char *template,
        struct PnWidget *parent) {

    char *cPath;
    char *dsoPath;
    int ret = 0;

    if(filename) {

        size_t len = strlen(filename);
        if(len >= 3 &&
                filename[len-1] == 'c' &&
                filename[len-2] == '.') {
            cPath = strdup(filename);
            ASSERT(cPath, "strdup() failed");
            dsoPath = malloc(len + 2);
            ASSERT(dsoPath, "malloc(%zu) failed", len+2);
            strcpy(dsoPath, cPath);
            strcpy(dsoPath + (len-1), "so");
        }
        else if(len >= 4 &&
                filename[len-1] == 'o' &&
                filename[len-2] == 's' &&
                filename[len-3] == '.') {
            dsoPath = strdup(filename);
            ASSERT(dsoPath, "strdup() failed");
            cPath = malloc(len);
            ASSERT(cPath, "malloc(%zu) failed", len);
            strncpy(cPath, dsoPath, len - 2);
            strcpy(cPath + (len - 2), "c");
        } else {
            cPath = malloc(len + 3);
            dsoPath = malloc(len + 4);
            strcpy(cPath, filename);
            strcpy(cPath + len, ".c"); 
            strcpy(dsoPath, filename);
            strcpy(dsoPath + len, ".so"); 
        }
    } else {
#define DEFAULT_FILENAME   "./qp_graph"
        cPath = strdup(DEFAULT_FILENAME ".c");
        ASSERT(cPath);
        dsoPath = strdup(DEFAULT_FILENAME ".so");
        ASSERT(dsoPath);
    }
    INFO("cPath=\"%s\"  dsoPath=\"%s\"", cPath, dsoPath);

    struct stat cStatbuf;
    struct stat dsoStatbuf;

    int cStat = stat(cPath, &cStatbuf);
    int dsoStat = stat(dsoPath, &dsoStatbuf);

    if(cStat) {
        if(!dsoStat)
            // We have no C file and a DSO exists.
            ret = (RunDso(parent, dsoPath)?1:-1);
        // We have no C file and no DSO.
        if(MakeCFile(cPath, template)) {
            ret = 0; // not creating graphs.
            goto finish;
        }
    }

    // We have a C file.
    if(dsoStat) {
        // We don't have a DSO
        if(Compile(cPath, dsoPath))
            ret = (RunDso(parent, dsoPath)?1:-2);
        else
            ret = -3; // not creating graphs.
        goto finish;
    }
    // We have a C file and a DSO.
    if(CheckDates(&cStatbuf, &dsoStatbuf)) {
        if(Compile(cPath, dsoPath))
            ret = (RunDso(parent, dsoPath)?1:-4);
        else
            // Compiling failed.
            ret = -5; // not creating graphs.
        goto finish;
    }

    // We have a C file and an up to date DSO.
    ret = (RunDso(parent, dsoPath)?1:-6);

finish:

    DZMEM(cPath, strlen(cPath));
    free(cPath);
    DZMEM(dsoPath, strlen(dsoPath));
    free(dsoPath);

    return ret;
}

