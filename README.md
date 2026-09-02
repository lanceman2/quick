# quick

A programming framework.

Programs and libraries written in C.  Small unopinionated code.

Currently being developed on a desktop with Debian GNU/Linux 13, KDE
Plasma, with Wayland, kernel version 6.12.74+deb13+1-amd64 (64-bit) as of
this writing.  We have found that the Gnome desktop does not follow some
to the Wayland standards, preferring most intrusive methods.  Gnome forces
methods upon users, like forcing you to link with there libraries to get
common GUIs (graphical user interfaces), and the KDE desktop (KWin) does
not.  GUI programs get super bloated just to support the Gnome desktop.


## Parts

We keep things in small library pieces so as to keep this software
non-opinionated.  We keep the dependences in a tree structure.

### libquickdraw

2D direct rendering line and shape drawing functions.

## libtext

Simple text drawing wrapper using libfontconfig and libfreetype.

### libpanels

GUI (graphical user interface) widget toolkit.
Depends on libquickdraw and libquicktext.

### Program quickplot

An interactive 2D plotter and software oscilloscope.  Depends on
libpanels.


## Building and Installing

We use the meson build system.

First make a "build" directory.  From the top source directory run:

```sh
meson setup --prefix /usr/local/encap/quick BUILD
```

This will make a build directory named BUILD with the installation
prefix configured to be /usr/local/encap/quick.

Now compile:
```sh
cd BUILD && meson compile
```

Now you should see many generated files in BUILD.  The binary
executable file should be able to run without being installed.

Now, if you like, install:
```sh
meson install
```

## Tests

quick is developed with both non-interactive and interactive test
programs.  There are a lot of command options to "meson test"; bash
tab-complication is your friend.

To run all non-interactive tests run (for example):
```sh
cd BUILD && meson test --verbose
```

If you have time to burn: run all interactive tests run (for example):
```sh
cd BUILD && meson test -j 1  --suite module:interactive 
```

quick has a valgrind meson exe_wrapper that you can use with,
for example:
```sh
meson test --setup valgrind -j 1 --suite module:non_interactive
```

We sometime have assertions in the test code, which if you hit will spew
to stdout and hang the test program.  To help see them add the -j 1 and
--verbose options, like for example:
```sh
meson test -j 1 --verbose
```

The following may take some time to run:
```sh
meson test -j 1 --setup valgrind
```
for it runs all tests including interactive tests.  It supposed to run
them one at a time, but I see it run them about three at a time.



