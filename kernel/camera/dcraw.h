#ifdef HAVE_JPEGLIB_H
#include <jpeglib.h>
#endif

#include <unistd.h>
#include <netinet/in.h>
typedef long long INT64;

#ifdef LJPEG_DECODE
#error Please compile dcraw.c by itself.
#error Do not link it with ljpeg_decode.
#endif

#ifndef LONG_BIT
#define LONG_BIT (8 * sizeof (long))
#endif

#define ushort UshORt
typedef unsigned char uchar;
typedef unsigned short ushort;

/*
   All global variables are defined here, and all functions that
   access them are prefixed with "CLASS".  Note that a thread-safe
   C++ class cannot have non-const static local variables.
 */
FILE     *ifp;
short    order;
char     *ifname, make[64], model[64], model2[64];
time_t   timestamp;
int      data_offset, curve_offset, curve_length;
int      tiff_data_compression, kodak_data_compression;
int      raw_height, raw_width, top_margin, left_margin;
int      height, width, colors, black, rgb_max;
int      iheight, iwidth, shrink;
int      is_canon, is_cmy, is_foveon, use_coeff, trim, flip, xmag, ymag;
int      zero_after_ff;
unsigned filters;
ushort (*image)[4], white[8][8];
void    (*load_raw)();
float   gamma_val=0.6, bright=1.0, red_scale=1.0, blue_scale=1.0;
int     four_color_rgb=0, document_mode=0, quick_interpolate=0;
int     verbose=0, use_auto_wb=0, use_camera_wb=0, use_secondary=0;
float   camera_red, camera_blue;
float   pre_mul[4], coeff[3][4];
int     histogram[0x2000];

void write_rawrgb(FILE *);

void (*write_fun)(FILE *) = write_rawrgb;
jmp_buf failure;

struct decode {
  struct decode *branch[2];
  int leaf;
} first_decode[2048], *second_decode, *free_decode;

#define CLASS

/*
   In order to inline this calculation, I make the risky
   assumption that all filter patterns can be described
   by a repeating pattern of eight rows and two columns

   Return values are either 0/1/2/3 = G/M/C/Y or 0/1/2/3 = R/G1/B/G2
 */
#define FC(row,col) \
	(filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)

#define BAYER(row,col) \
	image[((row) >> shrink)*iwidth + ((col) >> shrink)][FC(row,col)]

/*
   PowerShot 600 uses 0xe1e4e1e4:

	  0 1 2 3 4 5
	0 G M G M G M
	1 C Y C Y C Y
	2 M G M G M G
	3 C Y C Y C Y

   PowerShot A5 uses 0x1e4e1e4e:

	  0 1 2 3 4 5
	0 C Y C Y C Y
	1 G M G M G M
	2 C Y C Y C Y
	3 M G M G M G

   PowerShot A50 uses 0x1b4e4b1e:

	  0 1 2 3 4 5
	0 C Y C Y C Y
	1 M G M G M G
	2 Y C Y C Y C
	3 G M G M G M
	4 C Y C Y C Y
	5 G M G M G M
	6 Y C Y C Y C
	7 M G M G M G

   PowerShot Pro70 uses 0x1e4b4e1b:

	  0 1 2 3 4 5
	0 Y C Y C Y C
	1 M G M G M G
	2 C Y C Y C Y
	3 G M G M G M
	4 Y C Y C Y C
	5 G M G M G M
	6 C Y C Y C Y
	7 M G M G M G

   PowerShots Pro90 and G1 use 0xb4b4b4b4:

	  0 1 2 3 4 5
	0 G M G M G M
	1 Y C Y C Y C

   All RGB cameras use one of these Bayer grids:

	0x16161616:	0x61616161:	0x49494949:	0x94949494:

	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5
	0 B G B G B G	0 G R G R G R	0 G B G B G B	0 R G R G R G
	1 G R G R G R	1 B G B G B G	1 R G R G R G	1 G B G B G B
	2 B G B G B G	2 G R G R G R	2 G B G B G B	2 R G R G R G
	3 G R G R G R	3 B G B G B G	3 R G R G R G	3 G B G B G B

 */

void CLASS merror (void *ptr, const char *where);
ushort CLASS fget2 (FILE *f);
int CLASS fget4 (FILE *f);
void CLASS canon_600_load_raw(void);
void CLASS canon_a5_load_raw(void);
unsigned CLASS getbits (int nbits);
void CLASS init_decoder(void);
uchar * CLASS make_decoder (const uchar *source, int level);
void CLASS crw_init_tables (unsigned table);
int CLASS canon_has_lowbits(void);

void CLASS canon_compressed_load_raw(void);
void CLASS kodak_curve (ushort *curve);
void CLASS lossless_jpeg_load_raw(void);
void CLASS nikon_compressed_load_raw(void);
void CLASS nikon_load_raw(void);
int CLASS nikon_is_compressed(void);
int CLASS nikon_e990(void);
int CLASS nikon_e2100(void);
int CLASS minolta_z2(void);
void CLASS nikon_e2100_load_raw(void);
void CLASS nikon_e950_load_raw(void);
void CLASS fuji_s2_load_raw(void);
void CLASS fuji_common_load_raw(int, int, int);
void CLASS fuji_s5000_load_raw(void);
void CLASS fuji_s7000_load_raw(void);
void CLASS fuji_f700_load_raw(void);
void CLASS rollei_load_raw(void);
void CLASS phase_one_load_raw(void);
void CLASS ixpress_load_raw(void);
void CLASS packed_12_load_raw(void);
void CLASS unpacked_load_raw(int, int);
void CLASS be_16_load_raw(void);
void CLASS be_high_12_load_raw(void);
void CLASS be_low_12_load_raw(void);
void CLASS be_low_10_load_raw(void);
void CLASS le_high_12_load_raw(void);
void CLASS le_low_12_load_raw(void);
void CLASS olympus_cseries_load_raw(void);
void CLASS eight_bit_load_raw(void);
void CLASS casio_qv5700_load_raw(void);
void CLASS nucore_load_raw(void);
const int * CLASS make_decoder_int (const int *, int);
int CLASS radc_token (int);
void CLASS kodak_radc_load_raw(void);
void CLASS kodak_dc120_load_raw(void);
void CLASS kodak_dc20_coeff (float);
void CLASS kodak_easy_load_raw(void);
void CLASS kodak_compressed_load_raw(void);
void CLASS kodak_yuv_load_raw(void);
void CLASS sony_decrypt (unsigned *, int, int, int);
void CLASS sony_load_raw(void);
void CLASS sony_rgbe_coeff(void);
void CLASS foveon_decoder (unsigned huff[1024], unsigned);
void CLASS foveon_load_raw(void);
int CLASS apply_curve (int, const int *);
void CLASS foveon_interpolate(void);
void CLASS bad_pixels(void);
void CLASS scale_colors(void);
void CLASS vng_interpolate(void);
void CLASS tiff_parse_subifd (int);
void CLASS parse_makernote(void);
void CLASS get_timestamp(void);
void CLASS parse_exif (int);
void CLASS parse_tiff (int);
void CLASS parse_minolta(void);
void CLASS ciff_block_1030(void);
void CLASS parse_ciff (int, int);
void CLASS parse_rollei(void);
void CLASS parse_foveon(void);
void CLASS foveon_coeff(void);
void CLASS canon_rgb_coeff (float);
void CLASS nikon_e950_coeff(void);
void CLASS gmcy_coeff(void);
int CLASS identify(void);
void CLASS convert_to_rgb(void);
void CLASS flip_image(void);
int CLASS sqcall(int, char **);
