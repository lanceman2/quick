#ifndef __QUICKPLOT_PLUGIN_H__
#define __QUICKPLOT_PLUGIN_H__
#define __TEXT_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


// This is being compiled into a plugin library,
// FIXME: This is not very portable code:
#  define EXPORT __attribute__((visibility("default"))) extern


// These are the interfaces used by the quickplot program to access
// the plugin C code.
EXPORT void qp_graph(struct PnWidget *parent);
EXPORT void qp_cleanup(void);


#undef EXPORT

#endif // #ifndef __QUICKPLOT_PLUGIN_H__
