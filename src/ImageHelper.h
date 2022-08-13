#pragma once
#include <stddef.h>

/*case V4L2_PIX_FMT_PJPG:
//            /*FALLTHROUGH*/
//        case V4L2_PIX_FMT_JPEG:
//            /*FALLTHROUGH*/
//        case V4L2_PIX_FMT_MJPEG:
//            return vid_mjpegtoyuv420p(map, the_buffer->ptr, width, height
//                                      ,the_buffer->content_length);*/

int mjpegtoyuv420p (unsigned char*& camPixelsOut, unsigned char *camPixelsIn, int width, int height, size_t& size);

/*some funktions get from Motion-Project
 * jpegutils.h: Some Utility programs for dealing with
 *               JPEG encoded images
 *
 *  Copyright (C) 1999 Rainer Johanni <Rainer@Johanni.de>
 *  Copyright (C) 2001 pHilipp Zabel  <pzabel@gmx.de>
 *  Copyright (C) 2008 Angel Carpintero <motiondevelop@gmail.com>
 *
 */
int jpgutl_decode_jpeg (unsigned char* jpeg_data_in, int jpeg_data_len, unsigned int width, unsigned int height, unsigned char*& img_out, size_t& size);
