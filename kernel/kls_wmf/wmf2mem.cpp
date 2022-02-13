/* libwmf (convert/wmf2gd.c): library for wmf conversion
   Copyright (C) 2000 - various; see CREDITS, ChangeLog, and sources

   The libwmf Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The libwmf Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the libwmf Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fmt_defs.h"

#include <libwmf/fund.h>
#include <libwmf/types.h>
#include <libwmf/api.h>
#include <libwmf/ipa.h>
#include <libwmf/font.h>
#include <libwmf/color.h>
#include <libwmf/macro.h>

#include <libwmf/gd.h>

typedef struct
{	int    argc;
	char** argv;

	char*  wmf_filename;
	char*  gd_filename;

	wmf_gd_t options;

	unsigned int max_width;
	unsigned int max_height;

	unsigned long max_flags;

} PlotData;

#define WMF2GD_MAXPECT (1 << 0)
#define WMF2GD_MAXSIZE (1 << 1)

int  wmf2gd_draw (PlotData*, unsigned char **buf, int *w, int *h);

void wmf2gd_init (PlotData*,int,char**);
int  wmf2gd_args (PlotData*);
int  wmf2gd_file (PlotData*, unsigned char **buf, int *w, int *h);

int call(int argc,char** argv, unsigned char **buf, int *w, int *h);

int  explicit_wmf_error(wmf_error_t);

int wmf2gd_draw (PlotData* pdata, unsigned char **buf, int *www, int *hhh)
{
	int status = 0;

	float wmf_width;
	float wmf_height;

	float ratio_wmf;
	float ratio_bounds;

	unsigned int disp_width  = 0;
	unsigned int disp_height = 0;

	unsigned long flags;
	unsigned long max_flags;

	wmf_error_t err;

	wmf_gd_t* ddata = 0;

	wmfAPI* API = 0;

	wmfAPI_Options api_options;

	flags = 0;

	flags |= WMF_OPT_FUNCTION;
	api_options.function = wmf_gd_function;

	flags |= WMF_OPT_ARGS;
	api_options.argc = pdata->argc;
	api_options.argv = pdata->argv;
#ifndef DEBUG
	flags |= WMF_OPT_IGNORE_NONFATAL;
#endif
	err = wmf_api_create (&API,flags,&api_options);
	status = explicit_wmf_error (err);

	if (status)
	{	if (API) wmf_api_destroy (API);
		return (status);
	}

	ddata = WMF_GD_GetData (API);

	err = wmf_file_open(API, pdata->wmf_filename);
	status = explicit_wmf_error (err);

	if (status)
	{
		wmf_api_destroy (API);
		return (status);
	}

	err = wmf_scan (API, 0, &(pdata->options.bbox));
	status = explicit_wmf_error (err);

	if (status)
	{	
		wmf_api_destroy (API);
		return (status);
	}

/* Okay, got this far, everything seems cool.
 */
	ddata->type = pdata->options.type;

	ddata->flags |= WMF_GD_OUTPUT_MEMORY;
	ddata->file = pdata->options.file;

	ddata->bbox = pdata->options.bbox;

	wmf_display_size (API, &disp_width, &disp_height, 72, 72);

	wmf_width  = (float) disp_width;
	wmf_height = (float) disp_height;

	if ((wmf_width <= 0) || (wmf_height <= 0))
	{
		status = 1;
		wmf_api_destroy (API);
		return (status);
	}

	max_flags = pdata->max_flags;

	if ((wmf_width  > (float) pdata->max_width ) || (wmf_height > (float) pdata->max_height))
		if (max_flags == 0)
		    max_flags = WMF2GD_MAXPECT;

	if (max_flags == WMF2GD_MAXPECT) /* scale the image */
	{	ratio_wmf = wmf_height / wmf_width;
		ratio_bounds = (float) pdata->max_height / (float) pdata->max_width;

		if (ratio_wmf > ratio_bounds)
		{	ddata->height = pdata->max_height;
			ddata->width  = (unsigned int) ((float) ddata->height / ratio_wmf);
		}
		else
		{	ddata->width  = pdata->max_width;
			ddata->height = (unsigned int) ((float) ddata->width  * ratio_wmf);
		}
	}
	else if (max_flags == WMF2GD_MAXSIZE) /* bizarre option, really */
	{	ddata->width  = pdata->max_width;
		ddata->height = pdata->max_height;
	}
	else
	{	ddata->width  = (unsigned int) ceil ((double) wmf_width );
		ddata->height = (unsigned int) ceil ((double) wmf_height);
	}

	if (status == 0)
	{
		err = wmf_play(API, 0, &(pdata->options.bbox));
		status = explicit_wmf_error (err);
	}

	wmf_api_destroy(API);

	int *p = wmf_gd_image_pixels(ddata->gd_image);
	unsigned int w, h;

	unsigned int pixel;
	unsigned char r, g, b, a;

	*buf = new u8 [ddata->height * ddata->width * sizeof(RGBA)];

	if(!*buf)
	    return 1;
	    
	unsigned char *pss = *buf;

        for (h = 0; h < ddata->height; h++)
            for (w = 0; w < ddata->width; w++)
            {
                pixel = (unsigned int) (*p++);

                b = (unsigned char) (pixel & 0xff);
                pixel >>= 8;
                g = (unsigned char) (pixel & 0xff);
                pixel >>= 8;
                r = (unsigned char) (pixel & 0xff);
                pixel >>= 7;
                a = (unsigned char) (pixel & 0xfe);
                a ^= 0xff;

                *pss++ = r;
                *pss++ = g;
                *pss++ = b;
                *pss++ = a;
            }

	*www = ddata->width;
	*hhh = ddata->height;

	free(ddata->gd_image);
	
	if(ddata->memory)
	    free(ddata->memory);

	return status;
}

void wmf2gd_init(PlotData *pdata, int argc, char **argv)
{
	pdata->argc = argc;
	pdata->argv = argv;

	pdata->wmf_filename = 0;
	pdata->gd_filename = 0;

	pdata->options.type = wmf_gd_png;

	pdata->options.file = 0;

	pdata->options.width  = 0;
	pdata->options.height = 0;

	pdata->options.flags = 0;

	pdata->max_width  = 768;
	pdata->max_height = 512;

	pdata->max_flags = 0;
}

int wmf2gd_args (PlotData* pdata)
{
	int status = 0;
	int arg = 0;

	int    argc = pdata->argc;
	char** argv = pdata->argv;

	while ((++arg) < argc)
	{	
		if (strcmp (argv[arg],"-o") == 0)
		{	if ((++arg) < argc)
			{	pdata->gd_filename = argv[arg];
				continue;
			}
			fprintf (stderr,"usage: `wmf2gd -o <file.gd> <file.wmf>'.\n");
			fprintf (stderr,"Try `%s --help' for more information.\n",argv[0]);
			status = arg;
			break;
		}

		if (argv[arg][0] != '-')
		{	pdata->wmf_filename = argv[arg];
			continue;
		}

		fprintf (stderr,"option `%s' not recognized.\n",argv[arg]);
		fprintf (stderr,"Try `%s --help' for more information.\n",argv[0]);
		status = arg;

		break;
	}

	if (status == 0)
	{	if(pdata->wmf_filename == 0)
		{	
			fprintf (stderr,"No input file specified!\n");
			fprintf (stderr,"Try `%s --help' for more information.\n",argv[0]);
			status = argc;
		}
	}

	pdata->options.type = wmf_gd_image;

	return (status);
}

int wmf2gd_file(PlotData* pdata, unsigned char **buf, int *w, int *h)
{
	int status = 0;

	pdata->options.file = stdout;

	status = wmf2gd_draw(pdata, buf, w, h);

	if(pdata->options.file != stdout)
	    fclose(pdata->options.file);

	return status;
}

int call(int argc, char **argv, unsigned char **buf, int *w, int *h)
{	
	int status = 0;

	PlotData PData;

	wmf2gd_init(&PData, argc, argv);

	status = wmf2gd_args(&PData);

	if(status)
	    return status;

	status = wmf2gd_file(&PData, buf, w, h);

	return status;
}

int explicit_wmf_error (wmf_error_t err)
{
    int status = 0;

    switch (err)
    {
        case wmf_E_None:
	    status = 0;
	break;

        case wmf_E_InsMem:
        case wmf_E_BadFile:
        case wmf_E_BadFormat:
        case wmf_E_EOF:
        case wmf_E_DeviceError:
        case wmf_E_Glitch:
        case wmf_E_Assert:
	    status = 1;
        break;

	default:
	    status = 1;
    }

    return status;
}
