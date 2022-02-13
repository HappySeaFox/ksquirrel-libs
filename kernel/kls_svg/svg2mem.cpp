/*
    This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2005 Dmitry Baryshev <ksquirrel@tut.by>

    Based on svg2png by Carl Worth
    ------------------------------

    Render SVG image to memory buffer
*/

/* svg2png - Render an SVG image to a PNG image (using cairo)
 *
 * Copyright © 2002 USC/Information Sciences Institute
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Information Sciences Institute not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.  Information Sciences Institute
 * makes no representations about the suitability of this software for
 * any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * INFORMATION SCIENCES INSTITUTE DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL INFORMATION SCIENCES
 * INSTITUTE BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Carl Worth <cworth@isi.edu>
 */

#include <cstdio>
#include <cstdlib>
#include <cerrno>

#include "svg2mem.h"

#ifndef MIN
#define MIN(a, b)     (((a) < (b)) ? (a) : (b))
#endif

svg_cairo_status_t render_to_mem(FILE *svg_file, unsigned char **buf, int *w, int *h, double scale)
{
    unsigned int svg_width, svg_height;

    svg_cairo_status_t	status;
    cairo_t 		*cr;
    svg_cairo_t		*svgc;
    cairo_surface_t 	*surface;
    double 		dx = 0, dy = 0;
    unsigned int	width, height;

    status = svg_cairo_create(&svgc);

    if(status)
    {
	fprintf(stderr, "Failed to create svg_cairo_t. Exiting.\n");
	return SVG_CAIRO_STATUS_NO_MEMORY;
    }

    status = svg_cairo_parse_file(svgc, svg_file);

    if(status)
	return status;

    svg_cairo_get_size(svgc, &svg_width, &svg_height);

    width = (unsigned int)(scale * svg_width + 0.5);
    height = (unsigned int)(scale * svg_height + 0.5);

    if(!width) width = 1;
    if(!height) height = 1;

    *w = int(width);
    *h = int(height);

    *buf = (unsigned char *)malloc(width * height * 4);
    
    if(!*buf)
	return SVG_CAIRO_STATUS_NO_MEMORY;

    surface = cairo_image_surface_create_for_data(*buf, CAIRO_FORMAT_ARGB32, width, height, width * 4);

    cr = cairo_create(surface);
    
    cairo_save(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_restore(cr);

    cairo_translate(cr, dx, dy);
    cairo_scale(cr, scale, scale);

    /* XXX: This probably doesn't need to be here (eventually) */
    cairo_set_source_rgb(cr, 1, 1, 1);

    status = svg_cairo_render(svgc, cr);

    cairo_surface_destroy(surface);
    cairo_destroy(cr);

    if(status)
	return status;

    svg_cairo_destroy(svgc);

    return status;
}
