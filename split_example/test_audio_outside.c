#include <gst/gst.h>

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
  GstElement *pipeline;
  GstElement *source;
  GstElement *convert;
  GstElement *resample;
  GstElement *sink;
  GMainLoop *loop
} CustomData;

/* Handler for the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);

/* Handle position media streamer */
static gboolean cb_print_position (CustomData *data);

/* Handle message on bus of pipeline */
static gboolean bus_call (GstBus *bus, GstMessage *msg, CustomData *data);

int main(int argc, char *argv[]) {
  CustomData data;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  gboolean terminate = FALSE;
  guint bus_watch_id;

  //GstEvent *seek_event;
  gboolean result = FALSE;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  data.source = gst_element_factory_make ("uridecodebin", "source");
  data.convert = gst_element_factory_make ("audioconvert", "convert");
  data.resample = gst_element_factory_make ("audioresample", "resample");
  data.sink = gst_element_factory_make ("autoaudiosink", "sink");

  /* Create the empty pipeline */
  data.pipeline = gst_pipeline_new ("test-pipeline");

  //seek_event = gst_event_new_seek (1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET, 60 * GST_SECOND, GST_SEEK_TYPE_SET, 0);

    //seek_event = gst_event_new_seek (8.0, GST_FORMAT_TIME, 0, GST_SEEK_TYPE_NONE, -1, GST_SEEK_TYPE_NONE, -1); 

  if (!data.pipeline || !data.source || !data.convert || !data.resample || !data.sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Build the pipeline. Note that we are NOT linking the source at this
   * point. We will do it later. */
  gst_bin_add_many (GST_BIN (data.pipeline), data.source, data.convert, data.resample, data.sink, NULL);
  if (!gst_element_link_many (data.convert, data.resample, data.sink, NULL)) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

  /* create event seek playback backward */
   // seek_event = gst_event_new_seek (-8.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, 0);  

    
   //seek_event = gst_event_new_seek (1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET, 60 * GST_SECOND, GST_SEEK_TYPE_SET, 30 * GST_SECOND);  

  /* Set the URI to play */
  //g_object_set (data.source, "filesrc", "//home//truongtv//Desktop//sintel_trailer-480p.webm", NULL);

  //g_object_set (data.source, playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm",NULL );
  //g_object_set (data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);
//g_object_set (data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);
//g_object_set (data.source, "uri", "rtsp://cwp:123456@172.36.101.21:8554/test/ghita.flac", NULL);
 g_object_set (data.source, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/ctmc.wav", NULL);

 data.loop = g_main_loop_new (NULL, FALSE);
  /* Connect to the pad-added signal */
  g_signal_connect (data.source, "pad-added", G_CALLBACK (pad_added_handler), &data);
  
  /* signal timeout 200ms will connect to function  cb_print_position */
  g_timeout_add (200, (GSourceFunc) cb_print_position, &data);

  /* Send the event */

  /*
  ret = gst_element_set_state (data.pipeline, GST_STATE_PAUSED);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("XXXXXXXXXXXX Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }
  

  result = gst_element_send_event (data.sink, seek_event);
  if(!result) {
    g_print("ZZZZZZZZZZZZZZZZ Event seek fail ");
  }
  */

  /* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

  /*
  // Send the event
 
  result = gst_element_send_event (data.sink, seek_event);
  if(!result) {
    g_print("ZZZZZZZZZZZZZZZZ Event seek fail ");
  }
  */  

  /* Listen to the bus */
  bus = gst_element_get_bus (data.pipeline);
  //bus_watch_id = gst_bus_add_watch (bus, (GstBusFunc)bus_call, &data);
  
  g_main_loop_run (data.loop);


  do {
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS |GST_MESSAGE_APPLICATION );

    // Parse message
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
          // We are only interested in state-changed messages from the pipeline
          if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data.pipeline)) {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
            g_print ("Pipeline state changed from %s to %s:\n",
                gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
          }
          break;
        case GST_MESSAGE_APPLICATION:
            g_print ("ZZZZZZZZZZZZZZZZ Pipeline is state GST_MESSAGE_APPLICATION ");
            //gst_element_send_event (data.pipeline, seek_event);
            break;
        default:
          // We should not reach here 
          g_printerr ("Unexpected message received.\n");
          break;
      }
      gst_message_unref (msg);
    }
  } while (!terminate);
  

  /* Free resources */
  // debug 
  //GST_DEBUG_BIN_TO_DOT_FILE(data.pipeline, GST_DEBUG_GRAPH_SHOW_ALL, "tut_3");

  gst_object_unref (bus);
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  return 0;
}

/* This function will be called by the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
  GstPad *sink_pad = gst_element_get_static_pad (data->convert, "sink");
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;

  GstEvent *seek_event;
  gboolean result;

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

  seek_event = gst_event_new_seek (1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET, 40 * GST_SECOND, GST_SEEK_TYPE_SET, -1);

  // Send the event
 
  result = gst_element_send_event (data->pipeline, seek_event);
  if(!result) {
    g_print("ZZZZZZZZZZZZZZZZ Event seek fail ");
  }


   
exit:
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);

  /* Unreference the sink pad */
  gst_object_unref (sink_pad);
}

/* This function called timeout 200 */
static gboolean cb_print_position (CustomData *data)
{
  gint64 pos, len;

 if (gst_element_query_position (data->pipeline, GST_FORMAT_TIME, &pos) && gst_element_query_duration (data->pipeline, GST_FORMAT_TIME, &len))  {
    g_print ("Time: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r", GST_TIME_ARGS (pos), GST_TIME_ARGS (len));
 }

  /* call me again */
  return TRUE;
}

/* This function handle message on bus of pipeline */
static gboolean bus_call (GstBus *bus, GstMessage *msg, CustomData *data)
{
    GMainLoop *loop = (GMainLoop *) data->loop;

    switch (GST_MESSAGE_TYPE (msg)) {

        case GST_MESSAGE_EOS:
            g_print ("End of stream\n");
            g_main_loop_quit (loop);
            break;

        case GST_MESSAGE_ERROR:
            {
                gchar  *debug;
                GError *error;
                gst_message_parse_error (msg, &error, &debug);
                g_free (debug);

                g_printerr ("Error: %s\n", error->message);
                g_error_free (error);

                g_main_loop_quit (loop);
                break;
            }
        default:
            break;
    }
    return TRUE;
}

