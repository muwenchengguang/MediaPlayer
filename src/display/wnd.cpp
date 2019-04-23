#include "wnd.h"
#include <string.h>
#include <assert.h>

#define LOG_TAG "WND"
#include <log.h>

namespace peng {

#define MAIN_WND_WIDTH_DEFAULT (720)
#define MAIN_WND_HEIGHT_DEFAULT (540)
#define DRAW_AREA_WIDTH_DEFAULT (MAIN_WND_WIDTH_DEFAULT)
#define DRAW_AREA_HEIGHT_DEFAULT (MAIN_WND_HEIGHT_DEFAULT)

const int PIXEL_SIZE = sizeof(unsigned int);


static void zoom(const unsigned char* src, int sw, int sh, unsigned char* dst, int scale) {
    for (int j = 0; j < sh; j++) {
        int dw = sw*scale;
        unsigned char* srcW = (unsigned char*)(src + j*sw*4);

        for (int q = 0; q < scale; q++) {
            unsigned char* dstW = dst + (j*scale*dw*4) + dw*4*q;
            for (int i = 0; i < sw; i++) {
                for (int p = 0; p < scale; p++) {
                    dstW[i*4*scale + p*4] = srcW[i*4];
                    dstW[i*4*scale + 1 + p*4] = srcW[i*4 + 1];
                    dstW[i*4*scale + 2 + p*4] = srcW[i*4 + 2];
                    dstW[i*4*scale + 3 + p*4] = srcW[i*4 + 3];
                }
            }
        }

    }
}

static void rotate45(const unsigned char* src, int sw, int sh, unsigned char* dst) {
    for (int j = 0; j < sh; j++) {
        for (int i = 0; i < sw; i++) {
            int dw = sh;
            int dh = sw;
            dst[(dh-i-1)*dw*4 + j*4] = src[j*sw*4 + i*4];
            dst[(dh-i-1)*dw*4 + j*4 + 1] = src[j*sw*4 + i*4 + 1];
            dst[(dh-i-1)*dw*4 + j*4 + 2] = src[j*sw*4 + i*4 + 2];
            dst[(dh-i-1)*dw*4 + j*4 + 3] = src[j*sw*4 + i*4 + 3];
        }
    }
}

static void YUV2RGB(unsigned char* pYUV, unsigned char* pRGB) {
    double Y, U, V;
    double R, G, B;

    Y = pYUV[0];
    U = pYUV[1];
    V = pYUV[2];

    R = Y + 1.402*(V-128);
    G = Y - 0.34414*(U-128) - 0.71414*(V-128);
    B = Y + 1.772*(U-128);

 // B = 1.164*(Y - 16) + 2.018*(U - 128);

 // G = 1.164*(Y - 16) - 0.813*(V - 128) - 0.391*(U - 128);
 // R = 1.164*(Y - 16) + 1.596*(V - 128);

    if (R > 255)R = 255;
    if (G > 255)G = 255;
    if (B > 255)B = 255;
    if (R < 0)R = 0;
    if (G < 0)G = 0;
    if (B < 0)B = 0;
    pRGB[0] = (int)R;
    pRGB[1] = (int)G;
    pRGB[2] = (int)B;
    pRGB[3] = 0xff;
}

void convertYUV2RGB(const unsigned char* yuv, int w, int h, unsigned char* out) {
    int y_len = w*h;
    int u_len = w*h/4;
    int v_len = u_len;
    const unsigned char* Y = yuv;
    const unsigned char* U = Y + y_len;
    const unsigned char* V = U + u_len;
    const unsigned char* pY = Y, *pU = U, *pV = V;
    unsigned char YUV[3] = {0};
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            YUV[0] = *pY;
            YUV[1] = *pU;
            YUV[2] = *pV;
            YUV2RGB(YUV, out);
            out += 4;
            pY++;
            if (j%2 == 1) {
                pU++;
                pV++;
            }
        }
        if (i%2 == 1) {
            pU = pU - w/2;
            pV = pV - w/2;
        }
    }
}

void copyFromYUV2RGB32(const unsigned char* y, int y_ls,
                    const unsigned char* u, int u_ls,
                    const unsigned char* v, int v_ls,
                    int w, int h,
                    unsigned char* out) {
    const unsigned char* Y = y;
    const unsigned char* U = u;
    const unsigned char* V = v;
    const unsigned char* pY = Y, *pU = U, *pV = V;
    unsigned char YUV[3] = {0};
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            YUV[0] = pY[j];
            YUV[1] = pU[j/2];
            YUV[2] = pV[j/2];
            YUV2RGB(YUV, out);
            out += 4;
        }
        if (i%2 == 1) {
            pU = pU + u_ls;
            pV = pV + v_ls;
        }
        pY = pY + y_ls;
    }
}


static gboolean Redraw(gpointer data) {
  GtkDrawArea* wnd = reinterpret_cast<GtkDrawArea*>(data);
  wnd->OnRedraw();
  return false;
}


GtkMainWnd::GtkMainWnd():draw_area_(DRAW_AREA_WIDTH_DEFAULT, DRAW_AREA_HEIGHT_DEFAULT) {
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window_), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window_), MAIN_WND_WIDTH_DEFAULT, MAIN_WND_HEIGHT_DEFAULT);
    gtk_window_set_title(GTK_WINDOW(window_), "mixer demo");
    
    gtk_container_add(GTK_CONTAINER(window_), draw_area_.GetWidget());
    
    gtk_widget_show_all(window_);
}


GtkMainWnd::~GtkMainWnd() {
}

void GtkMainWnd::draw(const unsigned char* pixbuf, int width, int height) {
    draw_area_.draw(pixbuf, width, height);
}

unsigned char g_buff[1920*1080*4];

void GtkMainWnd::render(MediaBuffer* buffer, sp<MetaData> metadata) {
    int32_t width;
    int32_t height;
    int ret = metadata->findInt32(kKeyWidth, width);
    assert(ret == 0);
    ret = metadata->findInt32(kKeyHeight, height);
    assert(ret == 0);
    LOGI("w:%d, h:%d", width, height);
    memcpy(g_buff, buffer->data(), width*height*4);
    draw(g_buff, width, height);
}


//
// Implementation of draw area class GtkDrawArea.
//
GtkDrawArea::GtkDrawArea(int width, int height):input_buff_(NULL) {
    draw_area_ = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area_, width, height);
    width_ = width;
    height_ = height;


    draw_buffer_size_ = width_ * height_ * PIXEL_SIZE;
    draw_buffer_ = new unsigned char[draw_buffer_size_];
}
GtkDrawArea::~GtkDrawArea() {
    gtk_widget_destroy(draw_area_);
    delete draw_buffer_;
    delete []input_buff_;
}

void GtkDrawArea::draw(const unsigned char* buff, int w, int h) {
    //assert(input_buff_ == NULL);
    gdk_threads_enter();
    if (input_buff_ == NULL) {
        input_buff_ = new unsigned char[w*h*4];
    }
    memcpy((void*)input_buff_, (const void*)buff, w*h*4);
    input_w_ = w; 
    input_h_ = h;
    gdk_threads_leave();
    g_idle_add(Redraw, this);
}

void GtkDrawArea::OnRedraw() {
    gdk_threads_enter();
    DrawPixBuf(input_buff_, input_w_, input_h_);
    gdk_threads_leave();
}


void GtkDrawArea::DrawPixBuf(const unsigned char* pixbuf, int width, int height) {
    assert(draw_area_);
    float width_scale;
    float height_scale;

    /*const unsigned int* image = reinterpret_cast<const unsigned int*>(pixbuf);
    width_scale = 1.0f * width_ / width;
    height_scale = 1.0f * height_ / height;

    unsigned int* scaled = (unsigned int*)draw_buffer_;

    for (int r = 0; r < height; r ++) {
        int r_scaled = r * height_scale;
        for (int c = 0; c < width; c ++) {
            int c_scaled = c * width_scale;
            if (r_scaled > height_ || c_scaled > width_)
                continue;

            scaled[c_scaled + r_scaled * width_] = image[c + r * width];
        }
    }*/
    gdk_draw_rgb_32_image(draw_area_->window, draw_area_->style->fg_gc[GTK_STATE_NORMAL],  0,  0,  width_,  height_, GDK_RGB_DITHER_MAX,  pixbuf,  width_ * PIXEL_SIZE);
}

}
