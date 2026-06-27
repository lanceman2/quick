#include <math.h>

#include <panels.h>
#include <debug.h>


static int i_max = 730;

static double beta, phi;

static double alpha2, // 2 * alpha
              betaSq; // beta^2

static double x_0, x_1;


// Next x (X) as function of current x.
//
// Note this is iterating a map, or difference equation.
static inline double X() {

    double x_2 = + alpha2 * x_1 - betaSq * x_0;
    x_0 = x_1;
    x_1 = x_2;

    return x_2;
}

// This gets called at every graph redraw.
//
static bool StaticPlotterX(struct PnWidget *g, struct PnPlot *p,
        void *userData) {

    // Initial conditions
    x_0 = 1.0;
    x_1 = x_0 * beta * cos(phi);

    pnPlot_drawPoint(p, 0, x_0);
    pnPlot_drawPoint(p, 1, x_1);

    for(int i=2; i<=i_max; ++i)
        pnPlot_drawPoint(p, i, X());

    return false;
}

static void StaticPlotX(struct PnWidget *g) {

    struct PnPlot *p = pnStaticPlot_create(g, StaticPlotterX, 0);
    ASSERT(p);
    // This plot, p, is owned by the graph, w.
    pnPlot_setLineColor(p, 0xFFFF0000);
    pnPlot_setPointColor(p, 0xFF00FFFF);
    pnPlot_setLineWidth(p, 3.2);
    pnPlot_setPointSize(p, 4.5);

    ////////////////////////////////////////////////
    // Difference Equation Parameters
    ////////////////////////////////////////////////
    beta = 1.0 - 0.006;
    betaSq = beta * beta;

    double period = 60.0;
    phi = 2.0 * M_PI / period;

    double alpha = beta * cos(phi);
    alpha2 = 2.0 * alpha;

    fprintf(stderr, "beta=%lg period=%lg\n", beta, period);
}



static double y_0;

// Next x (X) as function of current x.
//
// Note this is iterating a map, or difference equation.
static inline double Y() {

    double y_1 = beta * y_0;
    y_0 = y_1;

    return y_1;
}

// This gets called at every graph redraw.
//
static bool StaticPlotterY(struct PnWidget *g, struct PnPlot *p,
        void *userData) {

    // Initial conditions
    y_0 = 1.0;
    pnPlot_drawPoint(p, 0, y_0);

    for(int i=1; i<=i_max; ++i)
        pnPlot_drawPoint(p, i, Y());

    return false;
}

static void StaticPlotY(struct PnWidget *g) {

    struct PnPlot *p = pnStaticPlot_create(g, StaticPlotterY, 0);
    ASSERT(p);
    // This plot, p, is owned by the graph, w.
    pnPlot_setLineColor(p, 0xFFF00000);
    pnPlot_setPointColor(p, 0xFF000FFF);
    pnPlot_setLineWidth(p, 3.2);
    pnPlot_setPointSize(p, 4.5);
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
    //pnGraph_setView(g, -6.05, 105.0, -6.0, 800);
    pnGraph_setView(g, - i_max * 0.05, i_max * 1.05, -1.3, 1.3);

    StaticPlotX(g);
    StaticPlotY(g);
}

