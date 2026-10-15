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


## We Strive For

A robust, unopinionated C library:

- Clear ownership: Every resource has an identifiable owner and lifetime.
  The library releases resources it owns; callers release resources they
  own.
- Complete cleanup: Every acquired resource has a cleanup path.
  Library-wide resources are cleaned up automatically at the appropriate
  lifecycle boundary, including library unload.
- Unload and reload safety: Teardown leaves no live callbacks, threads,
  registrations, references, or other state pointing into the library.
  Reinitialization after unload works correctly.
- Multiple-client safety: Several independent libraries or components can
  use the library without needing to inspect its private state or
  coordinate hidden cleanup.
- Repeatable operations: Initialization, cleanup, and failure recovery
  behave predictably when called repeatedly or after partial failure.
- Explicit contracts: Document ownership, lifetimes, thread-safety, error
  behavior, and which operations may block. Don’t make callers infer these
  from implementation details.
- No hidden policy: Avoid surprising process-wide behavior, implicit
  persistence, or irreversible actions. Expose caller-controlled choices
  as API options.
- Failure containment: Errors are reported through defined return values
  or status objects; partial failure doesn’t leave resources or state in
  an inconsistent condition.
- Bounded resource use: Repeated use does not cause unbounded growth in
  memory, descriptors, threads, registrations, or other operating system
  resources.
- Dependency hygiene: Cleanup doesn’t depend on undocumented teardown
  order or private knowledge of other libraries.
- Documentation: Documenting a defect (non-robust code) does not make the
  defect go away.

We can try to write code like that.


References:

https://nob.cs.ucdavis.edu/bishop/secprog/robust.html
https://www.baeldung.com/cs/opinionated-software-design

### It's An Unattainable Goal

... at least for this dummy.


- Robust VS Easy: Is it possible to make code easy to write/use while
  keeping it robust?  Robust code tends to expose more interfaces
  to the user in order to increase failure containment.

We are writing code here that can do many many things for each API
function call.  When comparing to the Wayland client library
(libwayland-client.so), we are defining a higher level interface that
removes so many user API details that the user has no time to learn, at
the cost of adding many hidden paths of execution the user will not be
aware of until one of those paths bites them.

We use assertions in the code which make this code somewhat opinionated.
Covering all the error recovery execution paths could make the code too complex (at least for me).  
The guess work in recovery execution paths, or user options for
recovery execution paths tend to get large.  Yes, of course the
initial base objects have error recovery execution paths, but once
objects have lots of structured child objects the number of available
error recovery execution paths become daunting, which could cause the
user interfaces get large.


## Parts

We keep things in small library pieces so as to keep this software
more unopinionated.  We keep the dependences in a tree structure.

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

I've always had a problem with duplicating data and making files larger
than what is needed.  The meson build system does not provide a way to
separate downloading sources files and setting up the build directory.  We
did not want to add git sub-modules to this software project just to get a
few stable source files; that could end up making the generated tarball
releases unnecessarily large (at least by default).

If you got this from a git repository, you need to get some more source
files.  Run this bash/wget script to get files that we do not keep in our
repository:
```sh
./download_files
```
If you skip this step "meson compile" will do it automatically, but we
just think that downloading files in the software "build" step is bad
form, can lead to broken build scripts, and makes building robust software
distributions much more difficult.

Next make a "build" directory.  From the top source directory run:
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
meson test --verbose
```

If you have time to burn: run all interactive tests run (for example):
```sh
meson test -j 1  --suite module:interactive 
```

quick has a valgrind meson exe_wrapper that you can use with,
for example:
```sh
meson test --setup valgrind -j 1 --suite module:non_interactive --verbose
```

We sometime have assertions in the test code, which if you hit will spew
to stderr and hang the test program.  To help see them add the -j 1 and
--verbose options, like for example:
```sh
meson test -j 1 --verbose
```

The following may take some time to run:
```sh
meson test -j 1 --setup valgrind
```
for it runs all tests including interactive tests.  It's supposed to run
them one at a time, but I see it run them about three at a time.



