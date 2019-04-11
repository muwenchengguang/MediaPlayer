#if 0
#include "main_wnd.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stddef.h>
#include <assert.h>
#include "utils/common.h"
#include <string.h>
#include "session/session_manager.h"

namespace peng {

#define MAIN_WND_WIDTH_DEFAULT (1280)
#define MAIN_WND_HEIGHT_DEFAULT (720)
#define MAIN_WND_BORDER_DEFAULT 10
#define REMOTE_DRAW_AREA_WIDTH_DEFAULT (352)
#define REMOTE_DRAW_AREA_HEIGHT_DEFAULT (288)
#define REMOTE_IMAGE_DEFAULT ""
#define MAIN_WND_SPACING_DEFAULT                5
#define PEER_LIST_WIDTH_DEFAULT                 160
#define PEER_LIST_HEIGHT_DEFAULT                320
const int PIXEL_SIZE = sizeof(unsigned int);


static void AddEntryToTable(GtkWidget* table,  GtkWidget *entry, guint left_attach, guint right_attach, guint top_attach, guint bottom_attach);
static gboolean OnDestroyedCallback(GtkWidget* widget, GdkEvent* event, gpointer data) {
  return false;
}

static gboolean Redraw(gpointer data) {
  GtkMainWnd::GtkDrawArea* wnd = reinterpret_cast<GtkMainWnd::GtkDrawArea*>(data);
  wnd->OnRedraw();
  return false;
}

// Click the Close button
static void OnCloseCallback(GtkWidget* widget, gpointer data) {
}

static void OnClickedCallback(GtkWidget* widget, gpointer data) {
}

static gboolean OnKeyPressCallback(GtkWidget* widget, GdkEventKey* key, gpointer data) {
  return false;
}


void OnRowActivatedCallback(GtkTreeView* tree_view, GtkTreePath* path,
                            GtkTreeViewColumn* column, gpointer data) {
  reinterpret_cast<GtkMainWnd*>(data)->OnRowActivated(tree_view, path, column);
}


int GtkMainWnd::stream_id_ = 0;
GtkMainWnd::GtkMainWnd():window_(NULL) {
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (window_) {
        gtk_window_set_position(GTK_WINDOW(window_), GTK_WIN_POS_CENTER);
        gtk_window_set_default_size(GTK_WINDOW(window_), MAIN_WND_WIDTH_DEFAULT, MAIN_WND_HEIGHT_DEFAULT);
        gtk_window_set_title(GTK_WINDOW(window_), "mixer demo");
        gtk_container_set_border_width(GTK_CONTAINER(window_), MAIN_WND_BORDER_DEFAULT);

        g_signal_connect(G_OBJECT(window_), "delete-event", G_CALLBACK(&OnDestroyedCallback), this);
        g_signal_connect(window_, "key-press-event", G_CALLBACK(OnKeyPressCallback), this);
    } else {
        assert(false);
    }

    chatting_table_ = gtk_table_new(2, 3, TRUE);
    gtk_container_add(GTK_CONTAINER(window_), chatting_table_);
    gtk_table_set_row_spacings(GTK_TABLE(chatting_table_), MAIN_WND_SPACING_DEFAULT);
    gtk_table_set_col_spacings(GTK_TABLE(chatting_table_), MAIN_WND_SPACING_DEFAULT);
    PRINT("GtkMainWnd created");
    createRemoteViews();
    gtk_widget_show_all(window_);

}


GtkMainWnd::~GtkMainWnd() {
}


void GtkMainWnd::createRemoteViews() {
    lb_ = new GtkPeerListBox(this);
    lb_->Create();
    lb_->AppendItem("rtsp://172.17.7.19:8554/h264.ts", 0);
    lb_->AppendItem("rtsp://10.239.146.203:8554/test3.264", 1);
    lb_->AppendItem("rtsp://10.239.146.203:8554/test.264", 2);
    lb_->AppendItem("rtsp://10.239.146.203:8554/test4.264", 3);
    lb_->AppendItem("rtsp://10.239.146.203:8554/xiayizhantianhou.aac", 4);
    lb_->AppendItem("rtsp://10.239.146.203:8554/xiayizhantianhou.mp3", 5);
    lb_->AppendItem("rtsp://10.239.146.203:8554/test.mpg", 6);
    connected_.push_back(false);
    connected_.push_back(false);
    AddEntryToTable(chatting_table_, lb_->GetWidget(), 0, 1, 0, 2);
    GtkDrawArea* da = new GtkDrawArea(REMOTE_DRAW_AREA_WIDTH_DEFAULT, REMOTE_DRAW_AREA_HEIGHT_DEFAULT);
    AddEntryToTable(chatting_table_, da->GetWidget(), 1, 2, 0, 1);
    remote_draw_areas_.push_back(da);
    is_draw_area_using_.push_back(false);

    da = new GtkDrawArea(REMOTE_DRAW_AREA_WIDTH_DEFAULT, REMOTE_DRAW_AREA_HEIGHT_DEFAULT);
    AddEntryToTable(chatting_table_, da->GetWidget(), 2, 3, 0, 1);
    remote_draw_areas_.push_back(da);
    is_draw_area_using_.push_back(false);
    
    da = new GtkDrawArea(REMOTE_DRAW_AREA_WIDTH_DEFAULT, REMOTE_DRAW_AREA_HEIGHT_DEFAULT);
    AddEntryToTable(chatting_table_, da->GetWidget(), 1, 2, 1, 2);
    remote_draw_areas_.push_back(da);
    is_draw_area_using_.push_back(false);
    
    da = new GtkDrawArea(REMOTE_DRAW_AREA_WIDTH_DEFAULT, REMOTE_DRAW_AREA_HEIGHT_DEFAULT);
    AddEntryToTable(chatting_table_, da->GetWidget(), 2, 3, 1, 2);
    remote_draw_areas_.push_back(da);
    is_draw_area_using_.push_back(false);
}


void GtkMainWnd::OnRowActivated(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column) {
    PRINT("OnRowActivated");
    GtkTreeIter iter;
    GtkTreeModel* model;
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        char* text;
        int id = -1;
        gtk_tree_model_get(model, &iter, 0, &text, 1, &id,  -1);
        if (id != -1) {
            PRINT("select [%d] = %s", id, text);
            if (!connected_[id]) {
                PRINT("createSender");
                sessions_[id] = mixer::getSessionManager()->createSession();
                MetaData file_meta;
                file_meta.setCString(MetaData::URL, text);
                sessions_[id]->setFileMetaData(file_meta);
                sessions_[id]->setStreamStateObserver(*this);
                sessions_[id]->createSender();
                connected_[id] = true;
            } else {
               PRINT("destroySender");
               sessions_[id]->destroySender();
               delete sessions_[id];
               sessions_[id] = NULL;
               connected_[id] = false;
            }
        }
        g_free(text);
    } else {
        MessageBox("Warning!", "Please select a peer to start chatting.", true);
    }
}

int GtkMainWnd::getFreeDisplay() {
    for (int i = 0; i < is_draw_area_using_.size(); i++) {
        if (!is_draw_area_using_[i])
            return i;
    }

    return -1;
}

void GtkMainWnd::freeDisplay(GtkDrawArea* da) {
    for (int i = 0; i < remote_draw_areas_.size(); i++) {
        if (remote_draw_areas_[i] == da) {
            assert(is_draw_area_using_[i]);
            is_draw_area_using_[i] = false;
            return;
        }
    }
    assert(false);
}

void GtkMainWnd::MessageBox(const char* caption, const char* text, bool is_error) {
  GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(window_),
      GTK_DIALOG_DESTROY_WITH_PARENT,
      is_error ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO,
      GTK_BUTTONS_CLOSE, "%s", text);
  gtk_window_set_title(GTK_WINDOW(dialog), caption);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}



void GtkMainWnd::onStreamStart(MetaData& meta) {
    PRINT("Live555Server::onStreamStart");

    const char* stream_type = NULL;
    CHECK_EQ(meta.findCString(MetaData::STREAM_TYPE, &stream_type), true);
    Stream* stream = NULL;
    std::string type(stream_type);
    if (type == "audio") {
        stream = new AStream(stream_id_++);
    } else if (type == "video"){
        int display = getFreeDisplay();
        if (display < 0) {
            PRINT("Discard the stream, because there are no available displays");
            return;
        }
        stream = new VStream(stream_id_++, remote_draw_areas_[display]);
        is_draw_area_using_[display] = true;
    } else {
        assert(false);
        return;
    }
    stream->onStart(meta);
    streams_.push_back(stream);
    mixer::StreamSender* ss = NULL;
    CHECK_EQ(meta.findPointer(MetaData::SOURCE_PROVIDER, &ss), true);
    ss->addSourceReceiver(*stream);
}

void GtkMainWnd::onStreamStop(MetaData& meta) {
    PRINT("Live555Server::onStreamStop");
    removeStreamByMetaData(meta);
    PRINT("Live555Server::onStreamStop finished");
}

void GtkMainWnd::removeStreamByMetaData(MetaData& meta) {
    const char* session = NULL;
    CHECK_EQ(meta.findCString(MetaData::SESSION, &session), true);
    std::string str_session(session);
    for (std::list<Stream*>::iterator it = streams_.begin(); it != streams_.end(); it++) {
        Stream* stream = *it;
        const char* tmp_session = NULL;
        CHECK_EQ(stream->getStreamMetaData().findCString(MetaData::SESSION, &tmp_session), true);
        std::string str_tmp_session(tmp_session);
        if (str_session == str_tmp_session) {
            mixer::StreamSender* ss = NULL;
            CHECK_EQ(stream->getStreamMetaData().findPointer(MetaData::SOURCE_PROVIDER, &ss), true);
            ss->removeSourceReceiver(*stream);
            streams_.erase(it);
            if (stream->getDrawArea() != NULL) {
                freeDisplay(stream->getDrawArea());
            }
            delete stream;
            break;
        }
    }
}

//
// Implementation of draw area class GtkDrawArea.
//
GtkMainWnd::GtkDrawArea::GtkDrawArea(int width, int height) {
    draw_area_ = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area_, width, height);
    width_ = width;
    height_ = height;


    draw_buffer_size_ = width_ * height_ * PIXEL_SIZE;
    draw_buffer_ = new unsigned char[draw_buffer_size_];
}
GtkMainWnd::GtkDrawArea::~GtkDrawArea() {
    gtk_widget_destroy(draw_area_);
    delete draw_buffer_;
}

void GtkMainWnd::GtkDrawArea::draw(const unsigned char* buff, int w, int h) {
    //assert(input_buff_ == NULL);
    gdk_threads_enter();
    memcpy(input_buff_, buff, w*h*4);
    input_w_ = w; 
    input_h_ = h;
    gdk_threads_leave();
    g_idle_add(Redraw, this);
}

void GtkMainWnd::GtkDrawArea::OnRedraw() {
    gdk_threads_enter();
    DrawPixBuf(input_buff_, input_w_, input_h_);
    gdk_threads_leave();
}

void GtkMainWnd::GtkDrawArea::DrawPixBuf(const unsigned char* pixbuf, int width, int height) {
    assert(draw_area_);
    float width_scale;
    float height_scale;

    const unsigned int* image = reinterpret_cast<const unsigned int*>(pixbuf);
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
    }
    gdk_draw_rgb_32_image(draw_area_->window, draw_area_->style->fg_gc[GTK_STATE_NORMAL],  0,  0,  width_,  height_, GDK_RGB_DITHER_MAX,  draw_buffer_,  width_ * PIXEL_SIZE);
}


//
// Implementation of Peer list controller class GtkPeerListBox.
//
GtkPeerListBox::GtkPeerListBox(GtkMainWnd* main_wnd)
    : main_wnd_(main_wnd), peer_list_(NULL) {
}

GtkPeerListBox::~GtkPeerListBox() {
        Destroy();
}

void GtkPeerListBox::Create() {
   GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
   GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
                "Contacts", renderer, "text", 0, NULL);
   GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

   peer_list_ = gtk_tree_view_new();

   g_signal_connect(peer_list_, "row-activated",
                  G_CALLBACK(OnRowActivatedCallback), main_wnd_);

   gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(peer_list_), FALSE);
   gtk_tree_view_append_column(GTK_TREE_VIEW(peer_list_), column);
   gtk_tree_view_set_model(GTK_TREE_VIEW(peer_list_), GTK_TREE_MODEL(store));

   gtk_widget_set_size_request(peer_list_, PEER_LIST_WIDTH_DEFAULT, PEER_LIST_HEIGHT_DEFAULT);

   g_object_unref(store);
}

void GtkPeerListBox::Destroy() {
    if (peer_list_) {
        gtk_widget_destroy(peer_list_);
        peer_list_ = NULL;
    }
}

void GtkPeerListBox::AppendItem(const char* str, int value) {
   GtkListStore* store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(peer_list_)));

   GtkTreeIter iter;
   gtk_list_store_append(store, &iter);
   gtk_list_store_set(store, &iter, 0, str, 1, value, -1);
}

void GtkPeerListBox::InsertItem(const char* str, int value, int position) {
   GtkListStore* store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(peer_list_)));

   GtkTreeIter iter;
   gtk_list_store_insert(store, &iter, position);
   gtk_list_store_set(store, &iter, 0, str, 1, value, -1);
}

void GtkPeerListBox::ClearItems() {
    GtkListStore* store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(peer_list_)));
    gtk_list_store_clear(store);
}

//=======GtkMainWnd::Stream======================

GtkMainWnd::VStream::VStream(int id, GtkMainWnd::GtkDrawArea* da):id_(id), da_(da), decoder_(NULL) {
}

GtkMainWnd::VStream::~VStream() {
    if (decoder_ != NULL)
        delete decoder_;
}

int GtkMainWnd::VStream::onNotify(MetaData& meta) {
    int len;
    const char* session = NULL;
    const char* buff = NULL;
    CODEC_TYPE type;
    CHECK_EQ(meta.findInt32(MetaData::BUFFER_LENGTH, &len), true);
    CHECK_EQ(meta.findPointer(MetaData::BUFFER_POINTER, &buff), true);
    CHECK_EQ(meta.findCString(MetaData::SESSION, &session), true);
    CHECK_EQ(meta.findInt32(MetaData::STREAM_CODEC, (int*)&type), true);
    //PRINT("main_wnd::Stream[%d]::onNotify, %d bytes, session:%s", id_, len, session);

    if (decoder_ == NULL) {
        decoder_ = new FFMPEGVideoDecoder(type);
    }
    bool bret = decoder_->decode(buff, len);
    if (bret) {
        da_->draw(decoder_->getBuffer(), decoder_->getWidth(), decoder_->getHeight());
    }
    return 0;
}


GtkMainWnd::AStream::AStream(int id):id_(id), decoder_(NULL) {
    PRINT("GtkMainWnd::AStream::AStream created");
}

GtkMainWnd::AStream::~AStream() {
    PRINT("GtkMainWnd::AStream::~AStream destroyed");
    if (decoder_ != NULL)
        delete decoder_;
}

int GtkMainWnd::AStream::onNotify(MetaData& meta) {
    int len;
    const char* session = NULL;
    const char* buff = NULL;
    CODEC_TYPE type;
    CHECK_EQ(meta.findInt32(MetaData::BUFFER_LENGTH, &len), true);
    CHECK_EQ(meta.findPointer(MetaData::BUFFER_POINTER, &buff), true);
    CHECK_EQ(meta.findCString(MetaData::SESSION, &session), true);
    CHECK_EQ(meta.findInt32(MetaData::STREAM_CODEC, (int*)&type), true);
    //PRINT("main_wnd::Stream[%d]::onNotify, %d bytes, session:%s", id_, len, session);

    if (decoder_ == NULL) {
        decoder_ = new FFMpegAudioDecoder(type);
    }
    
    int outsize, samples, samplerate;
    bool bret = false;
    if (type == MPEGA) {
        int frame_len;
        MP3Parser::FrameInfo info;
        int pos = MP3Parser::GetFrame(buff, len, frame_len, info);
        samplerate = info.samplerate;
        bret = decoder_->decode(&buff[pos], frame_len, buff_, outsize, samples);
    } else if (type == AAC) {
        AACParser::FrameInfo info;
        int frame_len;
        unsigned char aac_buff[1024*2] = {0};
        //assume that an audio payload length is less than 2000 bytes
        if (len >= 1000*2) {
            assert(false);
            return 0;
        }
        // construct ADTS header for AAC
        unsigned char ADTS[] = {0xFF, 0xF1, 0x00, 0x00, 0x00, 0x00, 0xFC};
        ADTS[2] = 0x50;
        ADTS[3] = 0x40;
        int aac_len = len - 4 + 7;
        aac_len <<= 5;
        aac_len |= 0x1F;
        ADTS[4] = aac_len>>8;
        ADTS[5] = aac_len & 0xFF;
        memcpy(aac_buff, ADTS, 7);
        memcpy(aac_buff + 7, buff + 4, len - 4);
        bret = decoder_->decode(aac_buff, len + 3, buff_, outsize, samples);
        //PRINT("outsize = %d  samples = %d", outsize, samples);
        samplerate = 44100;
    } else {
        assert(false);
        return 0;
    }
    if (bret) {
        int channels = outsize/2/samples;
        player_.post(buff_, outsize, samplerate, channels);
    }
    return 0;
}


//======================================================
void AddEntryToTable(GtkWidget* table,  GtkWidget *entry, guint left_attach, guint right_attach, guint top_attach, guint bottom_attach) {
    gtk_table_attach(GTK_TABLE(table), entry, left_attach, right_attach, top_attach, bottom_attach, GTK_FILL, GTK_SHRINK, 0, 0);
}

}
#endif
