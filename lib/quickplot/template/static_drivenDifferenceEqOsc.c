#include <math.h>

#include <panels.h>
#include <debug.h>

static long i_run = 1730;
static long i_stop = 1730 + 1000;



static double omega_min = 2.0 * M_PI/ 500.0,
              omega_max = 2.0 * M_PI/ 30.0,
              delta_omega = 2.0 * M_PI/ 2000.0;


static
double beta = 1.0 - 0.0056,/*damping 1 is not damped*/
       phi,/* alpha = beta * cos(phi) */
       A = 0.001,/*driver amplitude*/
       omega = 2.0 * M_PI/ 70.0,/* driver angular frequency */
       period = 70.0;/*natural period of oscillator*/

static double alpha2, // 2 * alpha
              betaSq; // beta^2

static double x_0, x_1;


// Next x (X) as function of current x.
//
// Note this is iterating a map, or difference equation.
static inline double X(long i) {

    double x_2 = + alpha2 * x_1 - betaSq * x_0 + A * cos(omega * i);
    x_0 = x_1;
    x_1 = x_2;

    return x_2;
}

static double Run(double omega_in) {

    // Initial conditions
    x_0 = 0.01;
    x_1 = x_0 * beta * cos(phi);

    omega = omega_in;

    long i = 2;

    for(; i<=i_run; ++i)
        X(i);

    double amp = - 1.0;

    for(; i<=i_stop; ++i) {
        double x = X(i);
        if(x > amp)
            amp = x;
    }

    return amp;
}



// This gets called at every graph redraw.
//
static bool StaticPlotterX(struct PnWidget *g, struct PnPlot *p,
        void *userData) {

    for(double o=omega_min; o <= omega_max; o += delta_omega)
        pnPlot_drawPoint(p, o, Run(o));

    return false;
}


static void StaticPlotX(struct PnWidget *g) {

    struct PnPlot *p = pnStaticPlot_create(g, StaticPlotterX, 0);
    ASSERT(p);
    // This plot, p, is owned by the graph, w.
    pnPlot_setLineColor(p, 0xFFFF0000);
    pnPlot_setPointColor(p, 0xFF00FFFF);
    pnPlot_setLineWidth(p, 6.2);
    pnPlot_setPointSize(p, 8.5);

    ////////////////////////////////////////////////
    // Set Difference Equation Parameters
    ////////////////////////////////////////////////
    betaSq = beta * beta;

    phi = 2.0 * M_PI / period;

    double alpha = beta * cos(phi);
    alpha2 = 2.0 * alpha;

    fprintf(stderr, "beta=%lg period=%lg  res angular frequency=%lg\n",
            beta, period, 2.0*M_PI/period);
}


// This is called by ../../bin/quickplot using dlsym().
//
void qp_graph(struct PnWidget *parent) {

    struct PnWidget *g = pnGraph_create(
            parent,
            90/*width*/, 70/*height*/, 0/*align*/,
            PnExpand_HV/*expand*/);
    ASSERT(g);
    //                  Color Bytes:  A R G B
    pnWidget_setBackgroundColor(g, 0xA0101010, 0);
    double delta = omega_max - omega_min;
    delta *= 0.05;
    //pnGraph_setView(g, -6.05, 105.0, -6.0, 800);
    pnGraph_setView(g, omega_min - delta, omega_max + delta , -0.1, 1.3);

    StaticPlotX(g);
}

