#ifndef WND_H
#define WND_H

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <RefBase.h>
#include <MediaBuffer.h>
#include <MetaData.h>

namespace peng {

typedef struct _GtkWidget GtkWidget;

class GtkDrawArea {
public:
    GtkDrawArea(int width, int height);
    ~GtkDrawArea();
    void DrawPixBuf(const unsigned char* pixbuf, int width, int height);
    GtkWidget* GetWidget() { return draw_area_; }
    void draw(const unsigned char* buff, int w, int h);
    void OnRedraw();
protected:
    GtkWidget*    draw_area_;
    int           width_;
    int           height_;
    unsigned char* draw_buffer_;
    int           draw_buffer_size_;
    const unsigned char  *input_buff_;
    int input_w_;
    int input_h_;
};


class GtkMainWnd: public RefBase {
public:
    GtkMainWnd();
    ~GtkMainWnd();
    
    void render(MediaBuffer* buffer, sp<MetaData> metadata);

protected:
    void draw(const unsigned char* pixbuf, int width, int height);
    
    
private:
    
    GtkWidget* window_;  // Our main window.
    GtkDrawArea draw_area_;
    
};

}

#endif
