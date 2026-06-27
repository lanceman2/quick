#include <math.h>

#include <panels.h>
#include <debug.h>


static const double amp = 1.0;

// Difference Equation Parameters
/////////////////////////////////////////////////////
static const double period = 100.0; // in iteration steps
// phase = n * phi   
//
// period * phi = 2 * Pi => phi = 2 * pi / period
static const double phi = 2.0 * M_PI / period;
// cos(phi) = alpha  => 2 * alpha = 2 * cos(phi)
static double alpha_2; // 2 * alpha

// Initial conditions that give a solution that is a cosine.
static double x_0;
static double x_1;


// Plotting parameters:  note iMin, iMax are iteration number
static const double iMin = - 0.1 * period;
static const double iMax = 3.01 * period;


// Next x_2 as function of current x_0, x_1.
//
// Note this is iterating a map, or difference equation.
static inline double X(void) {

    double x_2 = alpha_2 * x_1 - x_0;
    x_0 = x_1;
    x_1 = x_2;
    return x_2;
}


// This gets called at every graph redraw.
//
static bool mapPlotter(struct PnWidget *g, struct PnPlot *p,
        void *userData) {

    // Initial conditions that give a solution that is a cosine.
    x_0 = amp; // cos(0)
    x_1 = amp*cos(phi);
    pnPlot_drawPoint(p, 0, x_0);
    pnPlot_drawPoint(p, 1, x_1);

    for(int i=2; i < iMax; ++i)
        pnPlot_drawPoint(p, i, X());

    return false;
}

// This gets called at every graph redraw.
//
static bool cosPlotter(struct PnWidget *g, struct PnPlot *p,
        void *userData) {

    for(int i=0; i < iMax; ++i) {
        pnPlot_drawPoint(p, i, amp * cos(phi*i));
        pnPlot_drawPoint(p, i+0.5, amp * cos(phi*(i+0.5)));
    } 

    return false;
}



static void StaticPlot(struct PnWidget *g) {

    alpha_2 = 2.0 * cos(phi);

    struct PnPlot *p = pnStaticPlot_create(g, mapPlotter, 0);
    ASSERT(p);
    // This plot, p, is owned by the graph, g.
    pnPlot_setLineColor(p, 0xFFFF0000);
    pnPlot_setPointColor(p, 0xFF00FFFF);
    pnPlot_setLineWidth(p, 3.2);
    pnPlot_setPointSize(p, 4.5);

    p = pnStaticPlot_create(g, cosPlotter, 0);
    ASSERT(p);
    // This plot, p, is owned by the graph, g.
    pnPlot_setLineColor(p, 0xFF00FF09);
    pnPlot_setPointColor(p, 0xFF9900FF);
    pnPlot_setLineWidth(p, 1.2);
    pnPlot_setPointSize(p, 2.5);

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
    pnGraph_setView(g, iMin, iMax, -1.1*amp/*yMin*/, 1.1*amp/*yMax*/);

    StaticPlot(g);
}

