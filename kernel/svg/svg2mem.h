#include <cairo/cairo.h>
#include <svg-cairo.h>

extern "C" {
    svg_cairo_status_t render_to_mem(FILE *svg_file, unsigned char **buf, int *w, int *h);
}
