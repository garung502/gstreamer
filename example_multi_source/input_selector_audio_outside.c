#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <stdlib.h>

#include <gst/gst.h>

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct CustomData {
  GstElement *pipeline;

  GstElement *source_1;
  GstElement *source_2;
  
  GstElement *sink1_sync;
  GstElement *sink2_sync;

  GstElement *audio_switch;
  GstElement *segment;

  GstElement *convert;
  GstElement *resample;
  GstElement *sink;

  GMainLoop *loop;
} CustomData;

/* Handler for the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);
static void last_message_received (GObject * segment);
static gboolean switch_timer (CustomData *data);

static void check(gboolean status) {
    if (!status) {
        g_print("XXXXXXXXXXXXXXXXXXX Element_link fail !\n");
    }
}

int main(int argc, char *argv[]) {
  CustomData data;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  gboolean terminate = FALSE;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);
  data.loop = g_main_loop_new (NULL, FALSE);

  /* Create the elements */
  data.source_1 = gst_element_factory_make ("uridecodebin", "source_1");
  g_object_set (data.source_1, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/ghita.flac", NULL);
  data.source_2 = gst_element_factory_make ("uridecodebin", "source_2");
  g_object_set (data.source_2, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/Awake.flac", NULL);

  data.sink1_sync = gst_element_factory_make ("identity", "sink0_sync");
  g_object_set (G_OBJECT (data.sink1_sync), "sync", TRUE, NULL);

  data.sink2_sync = gst_element_factory_make ("identity", "sink1_sync");
  g_object_set (G_OBJECT (data.sink2_sync), "sync", TRUE, NULL);

  data.audio_switch = gst_element_factory_make ("input-selector", "audio_switch");

  data.segment = gst_element_factory_make ("identity", "identity-segment");
  g_object_set (G_OBJECT (data.segment), "silent", FALSE, NULL);
  //g_signal_connect (G_OBJECT (data.segment), "notify::last-message", G_CALLBACK (last_message_received), data.segment);
  g_object_set (G_OBJECT (data.segment), "single-segment", TRUE, NULL);
 

  data.convert = gst_element_factory_make ("audioconvert", "convert");
  data.resample = gst_element_factory_make ("audioresample", "resample");
  data.sink = gst_element_factory_make ("autoaudiosink", "sink");

  /* Create the empty pipeline */
  data.pipeline = gst_pipeline_new ("test-pipeline");

  if (!data.pipeline || !data.source_1 || !data.source_2 || !data.sink1_sync || !data.sink2_sync || 
      !data.audio_switch || !data.segment || !data.convert || !data.resample || !data.sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Build the pipeline. Note that we are NOT linking the source at this
   * point. We will do it later. */
  gst_bin_add_many (GST_BIN (data.pipeline), data.source_1, data.source_2, data.sink1_sync, data.sink2_sync, 
                    data.audio_switch, data.segment, data.convert, data.resample, data.sink, NULL);
  
  gboolean status;
  //status = gst_element_link (data.source_1, data.sink1_sync);
  //check(status);
  status = gst_element_link (data.sink1_sync, data.audio_switch);
  check(status);
  //status = gst_element_link (data.source_2, data.sink2_sync);
  //check(status);
  status = gst_element_link (data.sink2_sync, data.audio_switch);
  check(status);
  status = gst_element_link (data.audio_switch, data.segment);
  check(status);
  status = gst_element_link (data.segment, data.convert);
  check(status);
  status = gst_element_link (data.convert, data.resample);
  check(status);
  status = gst_element_link (data.resample, data.sink);
  check(status);

  /*
  if (!gst_element_link_many (data.convert, data.resample, data.sink, NULL)) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }
  */

  /* Set the URI to play */
  //g_object_set (data.source, "filesrc", "//home//truongtv//Desktop//sintel_trailer-480p.webm", NULL);

  //g_object_set (data.source, playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm",NULL );
  //g_object_set (data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);
  //g_object_set (data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);
  //g_object_set (data.source, "uri", "rtsp://cwp:123456@172.36.101.21:8554/test/ghita.flac", NULL);
  //g_object_set (data.source, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/ghita.flac", NULL);

  /* Connect to the pad-added signal */
  //g_signal_connect (data.source, "pad-added", G_CALLBACK (pad_added_handler), &data);
 
  //g_signal_connect (data.source_1, "pad-added", G_CALLBACK (pad_added_handler), &data);
  g_signal_connect (data.source_2, "pad-added", G_CALLBACK (pad_added_handler), &data);
  g_signal_connect (data.source_1, "pad-added", G_CALLBACK (pad_added_handler), &data);
  //g_timeout_add (5000, (GSourceFunc) switch_timer, &data);
  /* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

  g_timeout_add (5000, (GSourceFunc) switch_timer, &data);
  /* Listen to the bus */
  bus = gst_element_get_bus (data.pipeline);
  do {
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    /* Parse message */
    if (msg != NULL) {
      GError *err;
      gchar *debug_info;

      switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR:
          gst_message_parse_error (msg, &err, &debug_info);
          g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
          g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
          g_clear_error (&err);
          g_free (debug_info);
          terminate = TRUE;
          break;
        case GST_MESSAGE_EOS:
          g_print ("End-Of-Stream reached.\n");
          terminate = TRUE;
          break;
        case GST_MESSAGE_STATE_CHANGED:
          /* We are only interested in state-changed messages from the pipeline */
          if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data.pipeline)) {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
            g_print ("Pipeline state changed from %s to %s:\n",
                gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
          }
          break;
        default:
          /* We should not reach here */
          g_printerr ("Unexpected message received.\n");
          break;
      }
      gst_message_unref (msg);
    }
  } while (!terminate);

  //loop
  g_main_loop_run (data.loop);
  gst_element_set_state (GST_ELEMENT (data.pipeline), GST_STATE_READY);

  /* Free resources */
  // debug 
  GST_DEBUG_BIN_TO_DOT_FILE(data.pipeline, GST_DEBUG_GRAPH_SHOW_ALL, "tut_3");

  gst_object_unref (bus);
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  return 0;
}

/* This function will be called by the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
  gchar *name;
  GstPad *sink_pad = NULL;
  g_object_get (G_OBJECT (src), "name", &name, NULL);
  if (!strcmp(name, "source_1")) {
    sink_pad = gst_element_get_static_pad (data->sink1_sync, "sink");
    g_print ("SELECT SOURCE 1.\n");
  }
  else if (!strcmp(name, "source_2")) {
    sink_pad = gst_element_get_static_pad (data->sink2_sync, "sink");
    g_print ("SELECT SOURCE 2.\n");
  }
  else {
    // do nothing
  }
  //GstPad *sink_pad = gst_element_get_static_pad (data->convert, "sink");
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;

  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

  /* If our converter is already linked, we have nothing to do here */
  if (gst_pad_is_linked (sink_pad)) {
    g_print ("We are already linked. Ignoring.\n");
    goto exit;
  }

  /* Check the new pad's type */
  new_pad_caps = gst_pad_get_current_caps (new_pad);
  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  new_pad_type = gst_structure_get_name (new_pad_struct);
  
  g_print ("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ It has type '%s' .\n", new_pad_type);


  if (!g_str_has_prefix (new_pad_type, "audio/x-raw")) {
    g_print ("It has type '%s' which is not raw audio. Ignoring.\n", new_pad_type);
    goto exit;
  }

  /* Attempt the link */
  ret = gst_pad_link (new_pad, sink_pad);
  if (GST_PAD_LINK_FAILED (ret)) {
    g_print ("Type is '%s' but link failed.\n", new_pad_type);
  } else {
    g_print ("Link succeeded (type '%s').\n", new_pad_type);
  }

exit:
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);

  /* Unreference the sink pad */
  gst_object_unref (sink_pad);

  g_free (name);
}

static void last_message_received (GObject * segment)
{
  gchar *last_message;

  g_object_get (segment, "last_message", &last_message, NULL);
  g_print ("last-message: %s\n", last_message);
  g_free (last_message);
}

static gboolean switch_timer (CustomData *data)
{
  gint nb_sources;
  GstPad *active_pad, *new_pad;
  gchar *active_name;

  g_message ("switching");
  g_object_get (G_OBJECT (data->audio_switch), "n-pads", &nb_sources, NULL);
  g_object_get (G_OBJECT (data->audio_switch), "active-pad", &active_pad, NULL);

  active_name = gst_pad_get_name (active_pad);
  if (strcmp (active_name, "sink_0") == 0) {
    new_pad = gst_element_get_static_pad (data->audio_switch, "sink_1");

     // NOTE: Must put this signal on function main
    //g_signal_connect (data->source_2, "pad-added", G_CALLBACK (pad_added_handler), data);
  }
 else {
    new_pad = gst_element_get_static_pad (data->audio_switch, "sink_0");
    //g_signal_connect (data->source_1, "pad-added", G_CALLBACK (pad_added_handler), data);
  }
  g_object_set (G_OBJECT (data->audio_switch), "active-pad", new_pad, NULL);
  g_free (active_name);
  gst_object_unref (new_pad);

  g_message ("current number of sources : %d, active source %s",
      nb_sources, gst_pad_get_name (active_pad));

  return (GST_STATE (GST_ELEMENT (data->audio_switch)) == GST_STATE_PLAYING);
}

