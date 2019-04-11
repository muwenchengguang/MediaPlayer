#ifndef MAIN_WND_H
#define MAIN_WND_H

#include <vector>
#include "play_pcm.h"
#include "SingleThread.h"
#include "Semaphore.h"
#include <list>

// Forward declarations.
typedef struct _GtkWidget GtkWidget;
typedef union _GdkEvent GdkEvent;
typedef struct _GdkEventKey GdkEventKey;
typedef struct _GtkTreeView GtkTreeView;
typedef struct _GtkTreePath GtkTreePath;
typedef struct _GtkTreeViewColumn GtkTreeViewColumn;
typedef struct _GtkListStore GtkListStore;

namespace peng {


class SoundPlayer: public SingleThread::ThreadProc {
public:
    SoundPlayer():player_(16000, 1), sem_(0) {
        exit_ = false;
        sr_ = 16000;
        ch_ = 1;
        thread_play_sound_.registerThreadProc(*this);
        thread_play_sound_.start();
    }
    ~SoundPlayer() {
        exit_ = true;
        sem_.post();
        thread_play_sound_.stop();
    }


    void post(const unsigned char* buff, int len, int sr, int ch) {
        //memcpy(buff_ , buff, len);
        unsigned char* temp = new unsigned char[len];
        memcpy(temp, buff, len);
        len_ = len;
        sr_ = sr;
        ch_ = ch;
        buffs_.push_back(temp);
        sem_.post();
    }

protected:
    virtual bool process(int id) {
        sem_.wait();
        //PRINT("process");
        if (exit_) {
            return false;
        }


        player_.play((short*)buffs_.front(), len_/2, sr_, ch_);
        delete buffs_.front();
        buffs_.pop_front();
        return true;
    }

private:
    Semaphore sem_;
    unsigned char buff_[1024*1024];
    int len_;
    bool exit_;
    PCMPlayer player_;
    int sr_;
    int ch_;
    std::list<unsigned char*>buffs_;
    SingleThread thread_play_sound_;
};



class GtkPeerListBox;
class GtkMainWnd: public mixer::Sender::StreamStateObserver {
public:
class GtkDrawArea;

class Stream: public ISourceReceiver {
public:
    Stream() {
    }
    virtual ~Stream() {
    }
    
    virtual GtkDrawArea* getDrawArea() = 0;
};

class VStream: public Stream {
public:
    VStream(int id, GtkDrawArea* da);
    ~VStream();

    GtkDrawArea* getDrawArea() {
        return da_;
    }

protected:
    virtual int onNotify(MetaData& meta);
private:
    int id_;
    GtkDrawArea* da_;
    FFMPEGVideoDecoder* decoder_;
};

class AStream: public ISourceReceiver, public Stream {
public:
    AStream(int id);
    ~AStream();
    
    GtkDrawArea* getDrawArea() {
        return NULL;
    }

protected:
    virtual int onNotify(MetaData& meta);
private:
    int id_;
    FFMpegAudioDecoder* decoder_;
    unsigned char buff_[1024*100];
    SoundPlayer player_;
};

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
    const unsigned char  input_buff_[1280*720*4];
    int input_w_;
    int input_h_;
};

    GtkMainWnd();
    virtual ~GtkMainWnd();

    void OnRowActivated(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column);

protected:
    void createRemoteViews();
    virtual void onStreamStart(MetaData& meta);
    virtual void onStreamStop(MetaData& meta);
    void removeStreamByMetaData(MetaData& meta);
    int getFreeDisplay();
    void freeDisplay(GtkDrawArea* da);
    void MessageBox(const char* caption, const char* text, bool is_error);
private:
    GtkWidget* window_;  // Our main window.
    GtkWidget* chatting_table_; // Container for the Chatting UI.
    std::vector<GtkDrawArea*>remote_draw_areas_; // The drawing surface for remote rendering video streams.
    std::vector<bool> is_draw_area_using_;
    std::list<Stream*> streams_;
    static int stream_id_;
    GtkPeerListBox* lb_;
    std::vector<bool>connected_;
    std::map<int, mixer::Session*>sessions_;
};

class GtkPeerListBox {
public:
    GtkPeerListBox(GtkMainWnd* main_wnd);
    ~GtkPeerListBox();

    void Create();
    void Destroy();

    void AppendItem(const char* str, int value);
    void InsertItem(const char* str, int value, int position);
    void ClearItems();

    GtkWidget* GetWidget() { return peer_list_; }

protected:
    GtkMainWnd*   main_wnd_;
    GtkWidget*    peer_list_;
};

}
#endif
