#ifndef _READ_WRITE_IMAGE_jpg
#define _READ_WRITE_IMAGE_jpg

#include "../defs.h"
#include "../err.h"

#include <setjmp.h>

struct my_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

#endif
