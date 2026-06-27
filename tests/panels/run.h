
static inline void Run(struct PnWidget *win) {
#ifdef RUN
    // Run the main loop until the GUI user causes it to stop.
    while(pnDisplay_dispatch());
#else
    // Run the main loop until it appears to have drawn a window.  For a
    // non-interactive test that shows windows very briefly.  It looks
    // really cool.  Not a thing, so much, you'd want to do as an API
    // user.
    pnWindow_isDrawnReset(win);
    while(!pnWindow_isDrawn(win))
        ASSERT(pnDisplay_dispatch());
#endif
}
