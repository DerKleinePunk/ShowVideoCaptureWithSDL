#include "CamReader.h"

struct Framebuffer {
    void* start;
    size_t length;
};

static int xioctl(int fh, int request, void *arg)
{
    int r;
    auto retryCount = MAX_RETTRY_COUNT;

    do {
        r = v4l2_ioctl(fh, request, arg);
        retryCount--;

        if(retryCount == 0) {
            break;
        }
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

    if (r == -1) {
        fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
    }

    return r;
}

void CamReader::ReqestAndInitFrameBuffer() {
    v4l2_requestbuffers req;
    v4l2_buffer         buf;
    CLEAR(req);

    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    xioctl(_deviceHandle, VIDIOC_REQBUFS, &req);

    _buffers = new Framebuffer[req.count];
    for (_bufferCount = 0; _bufferCount < req.count; ++_bufferCount) {
        CLEAR(buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = _bufferCount;

        xioctl(_deviceHandle, VIDIOC_QUERYBUF, &buf);

        _buffers[_bufferCount].length = buf.length;
        _buffers[_bufferCount].start = v4l2_mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, _deviceHandle, buf.m.offset);

        if (MAP_FAILED == _buffers[_bufferCount].start) {
                perror("mmap");
                exit(EXIT_FAILURE);
        }
        memset(_buffers[_bufferCount].start, 0, buf.length);
    }

    for (auto i = 0; i < _bufferCount; ++i) {
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        xioctl(_deviceHandle, VIDIOC_QBUF, &buf);
    }
}

std::string CamReader::GetPixelFormatText(unsigned int format) {
    switch (format)
    {
        case V4L2_PIX_FMT_MJPEG:
            return "V4L2_PIX_FMT_MJPEG";
            break;
        case V4L2_PIX_FMT_JPEG:
            return "V4L2_PIX_FMT_JPEG";
            break;
        case V4L2_META_FMT_UVC:
            return "V4L2_META_FMT_UVC";
            break;
        case V4L2_PIX_FMT_YUYV:
            return "V4L2_PIX_FMT_YUYV";
            break;
         case V4L2_PIX_FMT_UYVY:
            return "V4L2_PIX_FMT_UYVY";
            break;
    default:
        return "not found";
    }
}

int CamReader::WorkerMain() {
    while (!_stopWorker)
    {
        if(GetNextFrame() <= 0) {
            return -1;
        }
    }
    printf("WorkerMain Stopped\n");
    return 0;
}

CamReader::CamReader(std::string deviceName) {
    _deviceName = deviceName;
    _videoWidth = -1;
    _videoHeight = -1;
    _videoPixelFormat = V4L2_META_FMT_UVC;
}

int CamReader::Init(int width, int height) {
   
    _deviceHandle = v4l2_open(_deviceName.c_str(), O_RDWR | O_NONBLOCK, 0);
    if (_deviceHandle < 0) {
        perror("Cannot open device");
        return -1;
    }

    v4l2_fmtdesc fmtdesc;
    CLEAR(fmtdesc);
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    //Todo Check Size ca be used
    while (xioctl(_deviceHandle, VIDIOC_ENUM_FMT, &fmtdesc) >= 0) {
        v4l2_frmsizeenum frmsize;
        CLEAR(frmsize);
        printf("Pixelformat %s is supported\n",GetPixelFormatText(fmtdesc.pixelformat).c_str()); 
        frmsize.pixel_format = fmtdesc.pixelformat;
        frmsize.index = 0;
        while (xioctl(_deviceHandle, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
            if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                printf("%dx%d\n", 
                            frmsize.discrete.width,
                            frmsize.discrete.height);
            } else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
                printf("%dx%d\n", 
                            frmsize.stepwise.max_width,
                            frmsize.stepwise.max_height);
            }
            frmsize.index++;
        }
        fmtdesc.index++;
    }
    
    //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG; -> USB Webcam
    //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    //V4L2_PIX_FMT_UYVY -> Grabber

    v4l2_format  fmt;
    uint32_t wantFormat = V4L2_PIX_FMT_YUYV;
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;

    fmt.fmt.pix.pixelformat = wantFormat;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    xioctl(_deviceHandle, VIDIOC_S_FMT, &fmt);
    if (fmt.fmt.pix.pixelformat != wantFormat) {
        printf("Libv4l didn't V4L2_PIX_FMT_YUYV support on this device\n");
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
        xioctl(_deviceHandle, VIDIOC_S_FMT, &fmt);
        if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_UYVY) {
            //Todo Implement jpeg Frames
            printf("Libv4l didn't V4L2_PIX_FMT_UYVY support on this device \n");
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
            xioctl(_deviceHandle, VIDIOC_S_FMT, &fmt);
            if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_JPEG) {
                printf("Libv4l didn't V4L2_PIX_FMT_JPEG\nNo supportet Format found\n");

                exit(EXIT_FAILURE);
            }
        }
    }
    
    _videoPixelFormat = fmt.fmt.pix.pixelformat;
    _videoWidth = fmt.fmt.pix.width;
    _videoHeight = fmt.fmt.pix.height;

    ReqestAndInitFrameBuffer();

    return 0;
}

void CamReader::GetPictureSize(int& width, int& height) {
    width = _videoWidth;
    height = _videoHeight;
}

void CamReader::StartStream(NewCamImageDelegate callbackNewCamImage) {
    _callbackNewCamImage = callbackNewCamImage;

    _type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(xioctl(_deviceHandle, VIDIOC_STREAMON, &_type) == 0) {
        printf("Stream Started\n");
        _stopWorker = false;
        _worker = std::thread(&CamReader::WorkerMain, this);
    }
}

void CamReader::StopStream() {
    _stopWorker = true;

    if(_worker.joinable()){
        _worker.join();
    }

    xioctl(_deviceHandle, VIDIOC_STREAMON, &_type);
}

uint CamReader::GetPixelFormat() const {
    return _videoPixelFormat;
}   

int CamReader::GetNextFrame() {
    int result = -1;
    fd_set  fds;
    timeval tv;

    do {
        FD_ZERO(&fds);
        FD_SET(_deviceHandle, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        result = select(_deviceHandle + 1, &fds, NULL, NULL, &tv);

    } while ((result == -1 && (errno = EINTR)));
    if (result == -1) {
        perror("select");
        return -1;
    }

    v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if(xioctl(_deviceHandle, VIDIOC_DQBUF, &buf) == -1) {
        printf("VIDIOC_DQBUF failed\n");
        return -1;
    } 
    
    result = _buffers[buf.index].length;
    if(_callbackNewCamImage != nullptr) {
        //printf("send %d buffer to callback\n", buf.index);
        _callbackNewCamImage(_buffers[buf.index].start, result);
    }

    xioctl(_deviceHandle, VIDIOC_QBUF, &buf);

    return result;
}

void CamReader::Deinit() {
    for (auto i = 0; i < _bufferCount; ++i)
        v4l2_munmap(_buffers[i].start, _buffers[i].length);
    v4l2_close(_deviceHandle);
}