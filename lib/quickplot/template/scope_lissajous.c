#include <inttypes.h>
#include <stdio.h>
#include <math.h>

#include <panels.h>
#include <debug.h>


static double t = 0.0;
static const double dt = 0.1;
static const uint32_t num = 50;

static bool Plot(struct PnWidget *g, struct PnPlot *p, void *userData) {

    for( uint32_t n = num; n; t += dt, n--) {
        double a = cos(0.34 + t/(540.2 * M_PI));
        pnPlot_drawPoint(p, a * cos(t), a * sin(2.01*t));
        t += 0.1;
    }
    // We'll redraw some:
    //t -= (num/2) * dt;

    pnWidget_queueDraw(g, 0);
    return false;
}


void qp_graph(struct PnWidget *parent) {

    struct PnWidget *w = pnGraph_create(
            parent,
            90/*width*/, 70/*height*/, 0/*align*/,
            PnExpand_HV/*expand*/);
    ASSERT(w);
    //                  Color Bytes:  A R G B
    pnWidget_setBackgroundColor(w, 0xA0101010, 0);

    struct PnPlot *p = pnScopePlot_create(w, Plot, 0);
    ASSERT(p);
    // This plot, p, is owned by the graph, w.
    pnPlot_setLineColor(p, 0xFFFF0000);
    pnPlot_setPointColor(p, 0xFF00FFFF);
    pnPlot_setLineWidth(p, 3.2);
    pnPlot_setPointSize(p, 4.5);

    pnGraph_setView(w, -1.05, 1.05, -1.05, 1.05);
}
