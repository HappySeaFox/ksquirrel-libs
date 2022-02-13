/* svg_image.c: Data structures for SVG image elements
 
   Copyright © 2002 USC/Information Sciences Institute
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Carl Worth <cworth@isi.edu>
*/

#include <string.h>
#include <setjmp.h>

#include "svgint.h"

svg_status_t
_svg_image_init (svg_image_t *image)
{
    _svg_length_init_unit (&image->x, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&image->y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&image->width, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&image->height, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    image->url = NULL;

    image->data = NULL;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_image_init_copy (svg_image_t *image,
		      svg_image_t *other)
{
    *image = *other;
    if (other->url)
	image->url = strdup (other->url);
    else
	image->url = NULL;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_image_deinit (svg_image_t *image)
{
    if (image->url) {
	free (image->url);
	image->url = NULL;
    }

    if (image->data) {
	free (image->data);
	image->data = NULL;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_image_apply_attributes (svg_image_t	*image,
			     const char		**attributes)
{
    const char *aspect, *href;

    _svg_attribute_get_length (attributes, "x", &image->x, "0");
    _svg_attribute_get_length (attributes, "y", &image->y, "0");
    _svg_attribute_get_length (attributes, "width", &image->width, "0");
    _svg_attribute_get_length (attributes, "height", &image->height, "0");
    /* XXX: I'm not doing anything with preserveAspectRatio yet */
    _svg_attribute_get_string (attributes,
			       "preserveAspectRatio",
			       &aspect,
			       "xMidyMid meet");
    /* XXX: This is 100% bogus with respect to the XML namespaces spec. */
    _svg_attribute_get_string (attributes, "xlink:href", &href, "");

    if (image->width.value < 0 || image->height.value < 0)
	return SVG_STATUS_PARSE_ERROR;

    /* XXX: We really need to do something like this to resolve
       relative URLs. It involves linking the tree up in the other
       direction. Or, another approach would be to simply throw out
       the SAX parser and use the tree interface of libxml2 which
       takes care of things like xml:base for us.

    image->url = _svg_element_resolve_uri_alloc (image->element, href);

       For now, the bogus code below will let me test the rest of the
       image support:
    */

    image->url = strdup ((char*)href);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_image_render (svg_image_t		*image,
		   svg_render_engine_t	*engine,
		   void			*closure)
{
    svg_status_t status;

    if (image->width.value == 0 || image->height.value == 0)
	return SVG_STATUS_SUCCESS;

    status = (engine->render_image) (closure,
				     (unsigned char*) image->data,
				     image->data_width,
				     image->data_height,
				     &image->x,
				     &image->y,
				     &image->width,
				     &image->height);
    if (status)
	return status;

    return SVG_STATUS_SUCCESS;
}
