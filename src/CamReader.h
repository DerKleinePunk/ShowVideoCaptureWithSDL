#pragma once
#ifndef SRC_CAMREADER
#define SRC_CAMREADER

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <functional>
#include <thread>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

/* This Class based on V4L2 video picture grabber from Mauro Carvalho Chehab <mchehab@infradead.org>
   see https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/v4l2grab.c.html  
 */

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define MAX_RETTRY_COUNT 250

struct Framebuffer;
typedef std::function<void(void* camPixels, int size)> NewCamImageDelegate;

class CamReader {
    std::string  _deviceName;
    int          _deviceHandle;
    Framebuffer* _buffers;
    int _videoWidth;
    int _videoHeight;
    int _bufferCount;
    enum v4l2_buf_type _type;
    uint _videoPixelFormat;
    NewCamImageDelegate _callbackNewCamImage;
    std::thread _worker;
    bool _stopWorker;
    void ReqestAndInitFrameBuffer();
    static std::string GetPixelFormatText(unsigned int format);
    int WorkerMain();
    int GetNextFrame();
public:
	explicit CamReader(std::string deviceName);

    int Init(int width, int height);
    void GetPictureSize(int& width, int& height);
    void StartStream(NewCamImageDelegate callbackNewCamImage);
    void StopStream();
    uint GetPixelFormat() const;
    void Deinit();
};  

#endif //SRC_CAMREADER