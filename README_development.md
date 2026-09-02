# Ideas for developers for this software project


Have optionally using static memory to construct objects along with
dynamic memory constructors.

Example:


struct PnWidget w;

?????



RANT:
I have too many reasons to not use C, but using it with C++, FORTRAN, and
any other programming languages is not discouraged.  Fuck Java, RUST, Zig,
and Go.  There are so many reasons to hate them.  The biggest reason, that
nobody talks about, is they all seem to be trying to wrap or replace the
operating system which works better than them in most cases.  In most
cases, they don't even work without the C coded operating system running
to support them.  It's like they are children of C and they hate C, but
they do not subsist without the compiled C code that supports them.  That
compiled C code is called an operating system.  The second reason is bloat
...  Okay zig could be good, but it's so immature, as it has no support
for dynamic shared objects yet; and the talk about it seems to indicate
that it may never support dynamic executables.  It would appear that most
computer programmers no longer bother to learn about operating systems.


## Starting this repo

Other than the ssh crap, just run:

git init --bare

git config --global init.defaultBranch



## For running Valgrind


apt install libc6-dbg

meson compile && meson test --suite module:all --verbose --wrap='valgrind --leak-check=full --error-exitcode=1'

Holy crap it worked.  Sweet!  Remove some free() calls and make sure it
can fail too.




## On simple direct drawing API

Do we want to add a matrix transform stack state layer?

A general drawing context?  That includes the 2 x 2 (or 3 x 3) matrix
stack.

Or is this just the lowest primitive layer of a drawing API.  At some
point, so I think, there is a function that just draws a fucking line to
pixels without a lot of bull shit, even in the most indirect drawing API.
And so, one could build a larger/slower drawing API on top of the simple
direct drawing API.


## What is an unopinionated API

An unopinionated API (application programming interface) tries to let API
users do whatever they want without interfering.  For example it's very
difficult to use pthreads when linking with the GTK+ API.  GTK+ imposes
their own wrapper of pthreads, and if you need more control of your
threads by using GTK+ you've got to do much more coding to work around the
gthreads and the GTK main_loop with your pthreads.  Another example:
unless you want to write your own linker/loader you can't unload the GTK+
libraries (same for Qt).

An unopinionated API does not mean that we can't have simplified default
fall-backs.  We just need to provide interfaces to small pieces in
addition to larger wrappers of the small pieces.

In perfect code we would have all failures go back and unwind cleanup all
the system (and other) resources that the current API owns up to the point
where the failure does not cascade through a chain of dependencies.

For the ideal case this code should not call ASSERT(), but instead deal
with releasing all resources that accumulated in the current API and then
return an error back back to the user.  In all the cases that I can think
of if ASSERT() is called, the code is fucked up, so the program is
likely running in a corrupted state.  In truly "clean", robust,
unopinionated code ASSERT() should not be used; but I suck at coding so I
use the ASSERT() crutch.  You can't just return 0 when malloc() fails, you
need to undo all the things from before the malloc(), at least at the API
user level (directly or indirectly), so the combinations of utility needed
to recover a usable running program gets too large; at least for the case
of failures in allocating small memory sizes; for larger memory allocation
failures there could be (more) recoverable cases.  Because of the use of
ASSERT() this code is not truly unopinionated for all failure modes in
it.

I think that the Wayland client code is a little less opinionated then
this software project, but it looks like you pay for it by having a ton of
objects composed of yet more objects; that is it provides interfaces to
stuff that seem like they should be internal.  I expect it's exposes so
much would-be internal structure to the API user because that is very
unopinionated.  Users can track all possible failures.  In this "quick"
software project we are trying to provide a higher level interface (then
for example libwayland-client), so it's a trade off for fewer interfaces
and doing more per interface, for lots of interfaces and doing less for
each interface.  As a consequence this code is a little bit more
opinionated.

So what's the point: we want to write a program to draw to pixels in
5000 lines of code (using libwayland-client), or 30 lines of code (using
libpanels).


## Strive For Zero Configuration

Nothing is worse than building a software package that has one thousand
build configuration options which need to be set consistently with your
use case.

We lean toward having all the software parts built so as to make it easy
to build without having a good understanding of all the software parts.
If this were mostly C++ code that could be a problem given C++ code
compiles about one hundred times slower than C.


## Quickstream - Modular Coding

Define a module (or block) as a running code that has library API
interfaces to other modules.  If modules can run in other processes all
the module interfaces need to be defined for inter-thread or inter-process
communication without the modules specifying/knowing which (??).  The
modules just publish their interfaces seamlessly, by declaring them with
the library API.  Like CORBA (or RPC) but not using TCP/IP as the IPC
method (NOT REALLY, just brain farting).

The "processing" modules can't have code that "runs" them, so that the
higher level user can control the running.

If a GTK GUI module runs in the same process, it can't be unloaded without
exiting the process.  A GTK GUI module that runs in the different process
can be unloaded by exiting.  The same goes for Qt.


What are the module interfaces: All module interfaces have connections as
they can be displayed as a graph.


   stream:

   parameter:


   and supersets (groups) of all of the above.
   How about types of stream and parameter data?


module is flow graph module or flow graph node.

running is flow graph runner


Flow Graph APIs by Functionality:

simple module  -  module to module connection interfaces: stream, parameters
           Inter-module connection points are all ports.
           Generic connect API that uses port names.

super module -  loads modules into a module group, defines connection groups

                Q: should super modules be flatten-able
                super modules do not run async.

builder - super modules are builders
          It builds putting together simple and sub super blocks

runner  -  creates/runs/destroys threads and processes that run modules
           runner is a simple module,  I think super modules can't be
           runners.


graph   - highest level class that owns all in the process group
          of course there can be many graphs, it's not a singleton.
          Singletons always end up being problematic.
          graph inherits or contains super module with the addition
          of ???  I don't see the user level functional difference
          between inheriting and containing one.



1. Go through the setter/getter game on the white board.

2. Go through the stream start/stop game on the white board.
   Are the start and stop simple module methods needed?
   Could they just detect the flow from the first flow() call?

3. Go through the sources and sinks idea on the white board.
   Since we can have sources that are also sinks WTF.
   There can be sources that are also sinks at the same time;
   example a web server module.

4. Do super modules have stream i/o and parameters?  The i/o and parameter
   for super modules are aliases to the i/o and parameters in the simple
   modules that are in them.  They to not have i/o and parameters of there
   own.

5. Is there always a top super module?

6. How is runner different from super module?

7. All list data classes are embedded in the struct like c-rbtree.

8. Study module resource/interface name spaces.  Map strings to pointers.

9. Can we have all the inter-module communication methods be inter-thread and inter-process
   without changing the API?

10.  Should module configuration API exist?  Can parameter replace it?

11. Extend builder API so we do not need the builder code to look at libquickstream code.

12. Is Graph the runner.

13. Think through the qsAddConfig() idea.  Should it just be callable by the containing
    super module.   If the containing super module wishes to it may make a wrapper alias
    (marshalling) to it.

14. Think through the runner as a simple module.  Can graphs have runner simple module
    to do the running stuff?   Should there is just one runner per graph?



# On Differential Equations to Difference Equations and Vice Versa

1. Can we make a change of variables that makes difference
   equations act like they would if the sample rate varied?  A sample rate
   control parameter.  This may be easy for Linear Systems.  In this way
   we could make difference equations act like they had non-constant
   sample rates to in effect have varying sample rates for system of
   difference equations that when iterated are used like difference
   equations that really have constant sample rates, were by making the
   streaming software that connect this "shit" together assume that sample
   rates are constant for all "parts".  It may be that it can only work
   for linear time-invariant difference equations.

