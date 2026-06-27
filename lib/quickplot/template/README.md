# quickplot DSO graph templates

quickplot DSO (dynamic shared objects) graphing plugins are compiled from
C files when loaded by the quickplot program.

quickplot DSO (dynamic shared objects) template C source files are example
C files that are used as a starter file to make graphing plugins.

All C (*.c) files in this directory are installed as templates.

It'd be nice to have filenames be descriptive.  A file name refactoring
may be necessary to make that happen, but after we have enough template
files to see a pattern.

The graphs created can have any number of graphs and plots in the graphs
with static and/or scope plots (in the same graph) with and without Cairo
drawing.

TODO: We still need to add the none Cairo drawing to the libpanels.so API
(application programming interface).  None Cairo drawing may be much more
performant than Cairo drawing.

Just open one of these C files in your favorite text editor.  It's not
that fucking hard.
