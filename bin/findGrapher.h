
// Find, compile, and run a DSO.
//
// Returns 1 if code was run, 0 if not, and less than 0 on error.
extern int findGrapher(const char *filename, const char *template,
        struct PnWidget *parent);

extern void cleanupGrapher(void);


// template is a path relative to the template directory.
//
extern int testTemplate(const char *template,
        struct PnWidget *parent);

extern int testCFile(const char *cPath,
        struct PnWidget *parent);
