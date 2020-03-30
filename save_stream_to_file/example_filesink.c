#include <stdio.h>
#include <gst/gst.h>

#define GLIB_DISABLE_DEPRECATION_WARNINGS

static GstElement *pipeline;
static GstPad *queue_src_pad;
static GstElement *bins[2];
static GstPad *bin_pads[2];
static GstElement *filesink[2];
static GMainLoop *loop;
static GstElement *flacenc[2];
static size_t current_bin = 0;
static long int current_file = 0;

static GstPadProbeReturn pad_probe_cb(GstPad * pad, GstPadProbeInfo * info, gpointer user_data) {
    gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID (info));

    gst_pad_unlink(queue_src_pad, bin_pads[current_bin]);
    gst_pad_send_event(bin_pads[current_bin], gst_event_new_eos());
    gst_element_set_state(bins[current_bin], GST_STATE_NULL);
    gst_object_ref(bins[current_bin]);
    gst_bin_remove(GST_BIN(pipeline), bins[current_bin]);


    current_file++;
    current_bin = (current_file % 2);

    {
    char file_location[32];
    sprintf(file_location, "recording_%ld.flac", current_file);
    g_object_set(G_OBJECT(
            filesink[current_bin]), "location", file_location, NULL);

    printf("now writing to %s\n", file_location);
    }

    gst_bin_add(GST_BIN(pipeline), bins[current_bin]);
    gst_pad_link(queue_src_pad, bin_pads[current_bin]);
    gst_element_set_state(bins[current_bin], GST_STATE_PLAYING);
    gst_element_sync_state_with_parent(bins[current_bin]);

    return GST_PAD_PROBE_OK;
}

static gboolean timeout_cb(gpointer user_data) {
    gst_pad_add_probe(queue_src_pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
        pad_probe_cb, NULL, NULL);

    return TRUE;
}

static gboolean bus_cb(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *)data;
    g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME(msg));
    switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
        g_print ("End of stream\n");
        //g_main_loop_quit (loop);
        break;

    case GST_MESSAGE_ERROR: {
        gchar  *debug;
        GError *error;

        gst_message_parse_error(msg, &error, &debug);
        g_free(debug);

        g_printerr("Error: %s\n", error->message);
        g_error_free(error);

        g_main_loop_quit(loop);
        break;
    }
    default:
        break;
    }

    return TRUE;
}

gint main(gint argc, gchar *argv[])
{
    GstElement *audiosrc;
    GstElement *queue;
    GstBus *bus;
    GMainLoop *loop;
    pthread_t libusb_tid;
    guint bus_watch_id;

    gst_init (&argc, &argv);

    //audiosrc = gst_element_factory_make("alsasrc", "audiosrc");
   audiosrc = gst_element_factory_make("autoaudiosrc", "audio_src"); 

    queue = gst_element_factory_make("queue", "queue");

    bins[0] = gst_bin_new("bin0");
    bins[1] = gst_bin_new("bin1");

    flacenc[0] = gst_element_factory_make("flacenc", "flacenc0");
    flacenc[1] = gst_element_factory_make("flacenc", "flacenc1");

    filesink[0] = gst_element_factory_make("filesink", "filesink0");
    filesink[1] = gst_element_factory_make("filesink", "filesink1");

    pipeline = gst_pipeline_new("test-pipeline");

    if (!pipeline || !audiosrc || !queue
            || !flacenc[0] || !filesink[0]
            || !flacenc[1] || !filesink[1]
            ) {
        g_printerr ("not all elements could be created\n");
        //return -1;
    }
    gst_bin_add_many(GST_BIN(bins[0]), flacenc[0], filesink[0], NULL);
    gst_bin_add_many(GST_BIN(bins[1]), flacenc[1], filesink[1], NULL);
    gst_bin_add_many(GST_BIN(pipeline), audiosrc, queue, bins[0], NULL);

    g_assert(gst_element_link(audiosrc, queue));

    g_assert(gst_element_link_many(flacenc[0], filesink[0], NULL));
    g_assert(gst_element_link_many(flacenc[1], filesink[1], NULL));

    GstPad* pad = gst_element_get_static_pad(flacenc[0], "sink");
    gst_element_add_pad(bins[0], gst_ghost_pad_new("sink", pad));
    gst_object_unref(pad);

    GstPad* pad2 = gst_element_get_static_pad(flacenc[1], "sink");
    gst_element_add_pad(bins[1], gst_ghost_pad_new("sink", pad2));
    gst_object_unref(pad2);

    bin_pads[0] = gst_element_get_static_pad(bins[0], "sink");
    bin_pads[1] = gst_element_get_static_pad(bins[1], "sink");

    current_bin = 0;
    gst_element_link(queue, bins[current_bin]);
    g_object_set(filesink[current_bin], "location", "recording_0.flac", NULL);

    queue_src_pad = gst_element_get_static_pad(queue, "src");

    bus = gst_element_get_bus(pipeline);
    bus_watch_id = gst_bus_add_watch(bus, bus_cb, loop);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    loop = g_main_loop_new(NULL, FALSE);
    g_timeout_add_seconds(10, timeout_cb, NULL);
    g_main_loop_run (loop);

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(pipeline);
    return 0;
}
