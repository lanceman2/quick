#include <math.h>

#include <panels.h>
#include <debug.h>
#include <quickplot_plugin.h>


static const double amp = 1.0;

#define PERIOD  (100.0)

// period * phi = 2 * Pi => phi = 2 * pi / period
static const double phi = 2.0 * M_PI / PERIOD;

// Plotting parameters:  note Min, iMax are iteration number
static const double Min = - 0.1 * PERIOD;
static const double Max = 3.01 * PERIOD;


// This gets called at every graph redraw.
//
static bool cosPlotter(struct PnWidget *g, struct PnPlot *p,
        void *userData) {

    for(int i=0; i < Max; ++i) {
        pnPlot_drawPoint(p, i, amp * cos(phi*i));
        pnPlot_drawPoint(p, i+0.5, amp * cos(phi*(i+0.5)));
    } 

    return false;
}


static void CreatePlot(struct PnWidget *g) {

    struct PnPlot *p = pnStaticPlot_create(g, cosPlotter, 0);
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
    pnGraph_setView(g, Min, Max, -1.1*amp/*yMin*/, 1.1*amp/*yMax*/);

    CreatePlot(g);
}
