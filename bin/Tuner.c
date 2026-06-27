#define TUNER_VERSION  "0.0.1"

#define _GNU_SOURCE
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include "../include/debug.h"
#include "../include/panels.h"
#include "../lib/opts.h"

#include "TunerOptions.h"
#include "cmdlopt.h"
#include "Spawn.h"


#define HELPER "/Tuner/misc/TunerHelp"

// STR(X) turns any CPP macro number into a string by using two macros.
#define STR(s) XSTR(s)
#define XSTR(s) #s



#define SAMPLE_RATE  384000  // samples per second feed to program arecord
#define YMIN ((double) INT_MIN)
#define YMAX ((double) INT_MAX)
#define SAMPLE_BYTES  (4)
#define ARECORD_FMT   "S32_LE"

static_assert((SAMPLE_BYTES) == sizeof(int32_t));


// FIXME: Maybe we should be using the libasound API
// Like in
// https://raw.githubusercontent.com/bear24rw/alsa-utils/refs/heads/master/aplay/aplay.c


// example:
//   arecord -r 384000 -f S32_LE -t raw -c1 -d 5 -B20000

// -f FORMAT -c numChannels -r Hz
// -B microseconds (buffer length)  1s/60 = 0.01666...seconds.
// 1 micro second is second/1000000 
static const char command[] =
        "arecord -r " STR(SAMPLE_RATE) " -f " ARECORD_FMT " -c1 -t raw -B80000";



// Returns program error status, and can set exitOnError flag that is
// passed to it.  exitOnError is only set here, but it's not used here.
//
// c is the command to switch on.
//
// command is the command as a long option string like "help" for --help.
//
// argc is the number of arg strings and argv[] is the strings.
//
int RunCommand(int c, int argc, const char *command,
        const char * const * argv, bool *exitOnError) {
    DASSERT(argc >= 1);
    DASSERT(command);
    DASSERT(c);
    DASSERT(argv);
    DASSERT(argv[0]);
    DASSERT(exitOnError);

#ifdef DEBUG
    fprintf(stderr, "Got command line option: "
            "\"%s[%c:%d]\":  ", command, c, c);
    for(int i=0; i<argc; ++i)
        fprintf(stderr, " %s", argv[i]);
    fprintf(stderr, "\n");
#endif

    switch(c) {

        case HELP:
            help(HELPER); // Does not return.
        case USAGE:
            usage(HELPER); // Does not return.
        case VERSION:
            printf("%s\n", TUNER_VERSION);
            exit(0);
        default:
    }

    return 0; // 0 --> success
}


static void catcher(int signum) {
    ASSERT(0, "Caught signal %d\n", signum);
}



static
double beta = 1.0 - 0.0056,/*damping beta = 1 is not damped*/
       A = 0.0001;/*driver amplitude multiplier*/

double alpha2, // 2 * alpha
       betaSq; // beta^2

double x_0, x_1, x_2;


#define LEN  (1024 * 2)
static int32_t buf[LEN];

// Stored oscillator x positions.
double x[LEN];





// Iterate once, a difference equation that is a good model for a driven
// damped harmonic oscillator.  The time between iterations is:
// 1 / SAMPLE_RATE, which is a sample period in seconds.
//
void Iterate(double driver) {

    // x_2 is the position of a driven damped harmonic oscillator at this
    // iteration index.
    x_2 = + alpha2 * x_1 - betaSq * x_0 + A * driver;
    // x_0 is the position before the previous position.
    x_0 = x_1;
    // x_1 is the previous position.
    x_1 = x_2;
}


void Init(double freq) {

    // freq is the resonate frequency of the x_2, x_1, x_0 oscillator in
    // Hz (1/seconds).  It's the tone we are trying to tune to.  For
    // example: freq = 97.999 is 97.999 Hz, is a musical G 2 tone.

    // SAMPLE_RATE is the sample rate that we read the mic with in Hz.
    ASSERT(freq < (double) SAMPLE_RATE/ 2.0);

    betaSq = beta * beta;
    double phi = 2.0 * M_PI * freq / ((double) SAMPLE_RATE);

    /* alpha = beta * cos(phi) */
    double alpha = beta * cos(phi);
    alpha2 = 2.0 * alpha;

    x_0 = 0.0;
    x_1 = 0.0;
    x_2 = 0.0;
}


#define LEN  (1024 * 2)
static int32_t buf[LEN];


static int ReadSound(int fd, void *userData) {

    struct PnWidget *g = userData;
    DASSERT(g);

    DASSERT(fd >= 0);

    size_t count = LEN *sizeof(uint32_t);
    uint8_t *buff = (uint8_t *) buf;

    errno = 0;
    while(count) {
        ssize_t rd = read(fd, buff, count);
        if(rd == 0 || errno) break;
        DASSERT(rd <= count);
        count -= rd;
        buff += rd;
    }

    if(count) {
        INFO("read() failed");
        return 1;
    }

    for(size_t i = 0; i < LEN; ++i) {
        Iterate(((double) buf[i]));
        x[i] = x_2;
    }


    pnWidget_queueDraw(g, 0);

    //fprintf(stderr, "%zu ", LEN);

    return 0;
}

double t = 0.0;

bool Plot(struct PnWidget *g, struct PnPlot *p, void *userData) {

    for(uint32_t n = 100; n; t += 0.1, n--) {
        double a = cos(0.34 + t/(540.2 * M_PI));
        pnPlot_drawPoint(p, a * cos(t), a * sin(2.01*t));
        t += 0.1;
    }

    for(uint32_t i = 0; i < LEN; ++i)
        pnPlot_drawPoint(p, i/((double)LEN) , ((double) buf[i])/YMAX);
    for(uint32_t i = 0; i < LEN; ++i)
        pnPlot_drawPoint(p, i/((double)LEN) , x[i]/YMAX);


    return false;
}


int main(int argc, const char * const *argv) {

    // Hang the program for debugging, if we segfault.
    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    int exitStatus = 0;
    bool exitOnError = true;
    int i = 1;


    while(i < argc) {

        const char *command = 0;

        //
        // Returns &command as the long opt from in options[].
        // Returns the short opt as an int:
        //
        int c = getOpt(argc, argv, i, options, &command);

        if(!c) {
            fprintf(stderr, "Error: unknown option at argv[%d]: %s\n",
                    i, argv[i]);
            if(exitOnError) {
                exitStatus = 1;
                break;
            }
            ++i;
            continue;
        }
        DASSERT(command);

        // GetNumArgs() is more particular to this program
        // than getOpt(), so that's why getOpt() does not set numArgs.
        //
        // Now we know that we have an option argument from the struct
        // opts option[] thingy, though more particular testing will be
        // done before we can run part of this program.
        //
        int numArgs = GetNumArgs(i, argc, argv, command);

        if((exitStatus = RunCommand(c, numArgs+1,
                        command, argv + i, &exitOnError))) {
            if(exitStatus && exitOnError)
                break;
            // else keep going.
        }

        ++i;
        i += numArgs;
    }

    // If we made it to here than here goes:

    int fd = -1;

    pid_t pid = Spawn(command, &fd, false/*nonblocking*/);
    DASSERT(fd >= 0);

    struct PnWidget *win = pnWindow_create(0, 10, 10,
            0/*x*/, 0/*y*/, PnLayout_LR/*layout*/, 0,
            PnExpand_HV);
    ASSERT(win);
    pnWindow_setPreferredSize(win, 1100, 900);

    // The auto 2D plotter grid (graph)
    struct PnWidget *w = pnGraph_create(
            win/*parent*/,
            90/*width*/, 70/*height*/, 0/*align*/,
            PnExpand_HV/*expand*/);
    ASSERT(w);
    //                  Color Bytes:  A R G B
    pnWidget_setBackgroundColor(w, 0xA0101010, 0);

    ASSERT(pnDisplay_addReader(fd, 0/*edge_trigger*/,
            ReadSound, w) == false);


    struct PnPlot *p = pnScopePlot_create(w, Plot, 0);
    ASSERT(p);
    // This plot, p, is owned by the graph, w.
    pnPlot_setLineColor(p, 0xFFFF0000);
    pnPlot_setPointColor(p, 0xFF00FFFF);
    pnPlot_setLineWidth(p, 3.2);
    pnPlot_setPointSize(p, 4.5);

    pnGraph_setView(w, -1.05, 1.05, -1.05, 1.05);

    Init(97.999);

    pnWindow_show(win);

    pnDisplay_run();


    if(pid) {
        printf("Cleaning up children processes\n");
        // Cleanup children processes.
        ASSERT(kill(pid, SIGTERM) == 0);
        ASSERT(waitpid(pid, 0, 0) == pid);
    }

    return exitStatus;
}
