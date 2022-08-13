#include "ImageHelper.h"
#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#include <cstdint> 
//#include <turbojpeg.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <jpeglib.h>
#include <jerror.h>
#ifdef __cplusplus
}
#endif

//Hints
//https://stackoverflow.com/questions/5616216/need-help-in-reading-jpeg-file-using-libjpeg
//https://code.i-harness.com/en/q/8ac623
//https://packages.debian.org/source/jessie/libjpeg-turbo

static const uint8_t EOI_data[2] = { 0xFF, 0xD9 };

struct jpgutl_error_mgr {
    struct jpeg_error_mgr pub;   /* "public" fields */
    jmp_buf setjmp_buffer;       /* For return to caller */

    /* Original emit_message method. */
    JMETHOD(void, original_emit_message, (j_common_ptr cinfo, int msg_level));
    /* Was a corrupt-data warning seen. */
    int warning_seen;
};

unsigned char* searchMem(unsigned char* haystack, size_t haystacklen, const char* needle, size_t needlelen)
{
  // Sanity check
  if (needlelen > haystacklen) return nullptr;

  // We'll stop searching at the last possible position for a match, 
  // which is haystack[ haystacklen - needlelen + 1 ]
  haystacklen -= needlelen - 1;

  while (haystacklen)
  {
    // Find the first byte in a potential match
    auto z = (unsigned char*)memchr( haystack, needle[0], haystacklen );
    if (!z) return nullptr;

    // Is there enough space for there to actually be a match?
    ptrdiff_t delta = z - haystack;
    ptrdiff_t remaining = (ptrdiff_t)haystacklen - delta;
    if (remaining < 1) return nullptr;

    // Advance our pointer and update the amount of haystack remaining
    haystacklen -= delta;
    haystack = z;

    // Did we find a match?
    if (!memcmp( haystack, needle, needlelen )) return haystack;
    
    // Ready for next loop
    haystack = haystack + 1;
    haystacklen -= 1;
  }
  return nullptr;
}

/*      some funktions get from video_common.c  Motion-Project 
 *
 *      Video stream functions for motion.
 *      Copyright 2000 by Jeroen Vreeken (pe1rxq@amsat.org)
 *                2006 by Krzysztof Blaszkowski (kb@sysmikro.com.pl)
 *                2007 by Angel Carpintero (motiondevelop@gmail.com)
 *      This software is distributed under the GNU public license version 2
 *      See also the file 'COPYING'.
 *      remove the memmem function because c++
 */

/**
 * mjpegtoyuv420p
 *
 * Return values
 *  -1 on fatal error
 *  0  on success
 *  2  if jpeg lib threw a "corrupt jpeg data" warning.
 *     in this case, "a damaged output image is likely."
 */
int mjpegtoyuv420p (unsigned char *&camPixelsOut, unsigned char *camPixelsIn, int width, int height, size_t& size) {
    unsigned char *ptr_buffer;
    size_t soi_pos = 0;
    int ret = 0;

    auto buffer = new unsigned char [size];
    memcpy(buffer, camPixelsIn, size);

    ptr_buffer = searchMem(buffer, size, "\xff\xd8", 2);
    if (ptr_buffer == NULL) {
        //MOTION_LOG(CRT, TYPE_VIDEO, NO_ERRNO,_("Corrupt image ... continue"));
        return 1;
    }
    /**
     Some cameras are sending multiple SOIs in the buffer.
     Move the pointer to the last SOI in the buffer and proceed.
    */
    while (ptr_buffer != NULL && ((size - soi_pos - 1) > 2) ){
        soi_pos = ptr_buffer - buffer;
        ptr_buffer = searchMem(buffer + soi_pos + 1, size - soi_pos - 1, "\xff\xd8", 2);
    }

    if (soi_pos != 0){
        //MOTION_LOG(INF, TYPE_VIDEO, NO_ERRNO,_("SOI position adjusted by %d bytes."), soi_pos);
    }
    
    memmove(buffer, buffer + soi_pos, size - soi_pos);
    size -= soi_pos;

    ret = jpgutl_decode_jpeg(buffer, size, width, height, camPixelsOut, size);

    delete [] buffer;

    if (ret == -1) {
        //MOTION_LOG(CRT, TYPE_VIDEO, NO_ERRNO,_("Corrupt image ... continue"));
        ret = 1;
    }
    return ret;
}

/**
 * jpgutl_error_exit
 *  Purpose:
 *    Exit routine for errors thrown by JPEG library.
 *  Parameters:
 *    cinfo      The jpeg library compression/decompression information
 *  Return values:
 *    None
 */
static void jpgutl_error_exit(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];

    /* cinfo->err really points to a jpgutl_error_mgr struct, so coerce pointer. */
    struct jpgutl_error_mgr *myerr = (struct jpgutl_error_mgr *) cinfo->err;

    /*
     * Always display the message.
     * We could postpone this until after returning, if we chose.
     */
    (*cinfo->err->format_message) (cinfo, buffer);

    //MOTION_LOG(ERR, TYPE_ALL, NO_ERRNO, "%s", buffer);

    /* Return control to the setjmp point. */
    longjmp (myerr->setjmp_buffer, 1);
}

/**
 * jpgutl_emit_message
 *  Purpose:
 *    Process the messages thrown by the JPEG library
 *  Parameters:
 *    cinfo      The jpeg library compression/decompression information
 *    msg_level  Integer indicating the severity of the message.
 *  Return values:
 *    None
 */
static void jpgutl_emit_message(j_common_ptr cinfo, int msg_level)
{
    char buffer[JMSG_LENGTH_MAX];
    /* cinfo->err really points to a jpgutl_error_mgr struct, so coerce pointer. */
    struct jpgutl_error_mgr *myerr = (struct jpgutl_error_mgr *) cinfo->err;
    /*
     *  The JWRN_EXTRANEOUS_DATA is sent a lot without any particular negative effect.
     *  There are some messages above zero but they are just informational and not something
     *  that we are interested in.
    */
    if ((cinfo->err->msg_code != JWRN_EXTRANEOUS_DATA) && (msg_level < 0) ) {
        myerr->warning_seen++ ;
        (*cinfo->err->format_message) (cinfo, buffer);
            //MOTION_LOG(DBG, TYPE_VIDEO, NO_ERRNO, "msg_level: %d, %s", msg_level, buffer);
    }

}

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
static void jpgutl_init_source(j_decompress_ptr cinfo)
{
    /* No work necessary here */
}

/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 */
static void jpgutl_term_source(j_decompress_ptr cinfo)
{
    /* No work necessary here */
}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * Should never be called since all data should be already provided.
 * Is nevertheless sometimes called - sets the input buffer to data
 * which is the JPEG EOI marker;
 *
 */
static boolean jpgutl_fill_input_buffer(j_decompress_ptr cinfo)
{
    cinfo->src->next_input_byte = EOI_data;
    cinfo->src->bytes_in_buffer = 2;
    return TRUE;
}

/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 */
static void jpgutl_skip_data(j_decompress_ptr cinfo, long num_bytes)
{
    if (num_bytes > 0) {
        if (num_bytes > (long) cinfo->src->bytes_in_buffer)
            num_bytes = (long) cinfo->src->bytes_in_buffer;
        cinfo->src->next_input_byte += (size_t) num_bytes;
        cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
    }
}

/*
 * The source object and input buffer are made permanent so that a series
 * of JPEG images can be read from the same buffer by calling jpgutl_buffer_src
 * only before the first one.  (If we discarded the buffer at the end of
 * one image, we'd likely lose the start of the next one.)
 * This makes it unsafe to use this manager and a different source
 * manager serially with the same JPEG object.  Caveat programmer.
 */
/**
 * jpgutl_buffer_src
 *  Purpose:
 *    Establish the input buffer source for the JPEG libary and associated helper functions.
 *  Parameters:
 *    cinfo      The jpeg library compression/decompression information
 *    buffer     The buffer of JPEG data to decompress.
 *    buffer_len The length of the buffer.
 *  Return values:
 *    None
 */
static void jpgutl_buffer_src(j_decompress_ptr cinfo, unsigned char *buffer, long buffer_len)
{

    if (cinfo->src == NULL) {    /* First time for this JPEG object? */
        cinfo->src = (jpeg_source_mgr *)cinfo->mem->alloc_small((j_common_ptr) cinfo, JPOOL_PERMANENT,sizeof (jpeg_source_mgr));
     }

    cinfo->src->init_source = jpgutl_init_source;
    cinfo->src->fill_input_buffer = jpgutl_fill_input_buffer;
    cinfo->src->skip_input_data = jpgutl_skip_data;
    cinfo->src->resync_to_restart = jpeg_resync_to_restart;    /* Use default method */
    cinfo->src->term_source = jpgutl_term_source;
    cinfo->src->bytes_in_buffer = buffer_len;
    cinfo->src->next_input_byte = (JOCTET *) buffer;


}


/**
 * jpgutl_decode_jpeg
 *  Purpose:  Decompress the jpeg data_in into the img_out buffer.
 *
 *  Parameters:
 *  jpeg_data_in     The jpeg data sent in
 *  jpeg_data_len    The length of the jpeg data
 *  width            The width of the image
 *  height           The height of the image
 *  img_out          Pointer to the image output
 *
 *  Return Values
 *    Success 0, Failure -1
 */
int jpgutl_decode_jpeg (unsigned char *jpeg_data_in, int jpeg_data_len, unsigned int width, unsigned int height, unsigned char*& img_out, size_t& size )
{
    JSAMPARRAY      line;           /* Array of decomp data lines */
    unsigned char  *wline;          /* Will point to line[0] */
    unsigned int    i;
    unsigned char  *img_y, *img_cb, *img_cr;
    unsigned char   offset_y;

    jpeg_decompress_struct dinfo;
    jpgutl_error_mgr jerr;

    /* We set up the normal JPEG error routines, then override error_exit. */
    dinfo.err = jpeg_std_error (&jerr.pub);
    jerr.pub.error_exit = jpgutl_error_exit;
    /* Also hook the emit_message routine to note corrupt-data warnings. */
    jerr.original_emit_message = jerr.pub.emit_message;
    jerr.pub.emit_message = jpgutl_emit_message;
    jerr.warning_seen = 0;

    jpeg_create_decompress (&dinfo);

    /* Establish the setjmp return context for jpgutl_error_exit to use. */
    if (setjmp (jerr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error. */
        jpeg_destroy_decompress (&dinfo);
        return -1;
    }

    jpgutl_buffer_src (&dinfo, jpeg_data_in, jpeg_data_len);

    jpeg_read_header (&dinfo, TRUE);

    //420 sampling is the default for YCbCr so no need to override.
    dinfo.out_color_space = JCS_YCbCr;
    dinfo.dct_method = JDCT_DEFAULT;
    //guarantee_huff_tables(&dinfo);  /* Required by older versions of the jpeg libs */
    jpeg_start_decompress (&dinfo);

    if ((dinfo.output_width == 0) || (dinfo.output_height == 0)) {
        //MOTION_LOG(WRN, TYPE_VIDEO, NO_ERRNO,_("Invalid JPEG image dimensions"));
        jpeg_destroy_decompress(&dinfo);
        return -1;
    }

    if ((dinfo.output_width != width) || (dinfo.output_height != height)) {
        //MOTION_LOG(WRN, TYPE_VIDEO, NO_ERRNO,_("JPEG image size %dx%d, JPEG was %dx%d"),width, height, dinfo.output_width, dinfo.output_height);
        jpeg_destroy_decompress(&dinfo);
        return -1;
    }

    size = dinfo.output_width * dinfo.output_height * dinfo.output_components;
    img_out = new unsigned char [size];
    img_y  = img_out;
    img_cb = img_y + dinfo.output_width * dinfo.output_height;
    img_cr = img_cb + (dinfo.output_width * dinfo.output_height) / 4;

    /* Allocate space for one line. */
    line = (*dinfo.mem->alloc_sarray)((j_common_ptr) &dinfo, JPOOL_IMAGE,
                                       dinfo.output_width * dinfo.output_components, 1);

    wline = line[0];
    offset_y = 0;

    while (dinfo.output_scanline < dinfo.output_height) {
        jpeg_read_scanlines(&dinfo, line, 1);

        for (i = 0; i < (dinfo.output_width * 3); i += 3) {
            img_y[i / 3] = wline[i];
            if (i & 1) {
                img_cb[(i / 3) / 2] = wline[i + 1];
                img_cr[(i / 3) / 2] = wline[i + 2];
            }
        }

        img_y += dinfo.output_width;

        if (offset_y++ & 1) {
            img_cb += dinfo.output_width / 2;
            img_cr += dinfo.output_width / 2;
        }
    }

    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);

    /*
     * If there are too many warnings, this means that
     * only a partial image could be returned which would
     * trigger many false positive motion detections
    */
    if (jerr.warning_seen > 2) return -1;

    return 0;

}
