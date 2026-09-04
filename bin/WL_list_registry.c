
#include <stdint.h>
#include <stdio.h>
#include <wayland-client.h>

#include "../include/debug.h"


static void
registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
    printf("interface: %s  version: %" PRIu32 "  name: %" PRIu32 "\n",
            interface, version, name);
}

#if 0 // Why do I need this?  Shouldn't zeroing it out be the same.
static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
    // This space deliberately left blank
}
#endif

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    //.global_remove = registry_handle_global_remove
    .global_remove = 0
};

int main(int argc, char *argv[]) {

    struct wl_display *display = wl_display_connect(NULL);
    ASSERT(display);
    struct wl_registry *registry = wl_display_get_registry(display);
    ASSERT(registry);
    ASSERT(0 == wl_registry_add_listener(registry, &registry_listener, NULL));
    ASSERT(wl_display_roundtrip(display));
    return 0;
}
