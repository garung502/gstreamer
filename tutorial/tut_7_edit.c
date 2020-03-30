#include <string.h>
#include <stdio.h>
#include <gst/gst.h>
#include <gst/gstmessage.h>

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
  GstElement *pipeline, *audio_source, *tee, *audio_queue, *audio_convert, *audio_resample, *audio_sink;
  GstElement *video_queue, *visual, *video_convert, *video_sink;
  GMainLoop *loop;
  int index_source;
  gboolean playing;
} CustomData;

/* Handler for the pad-added signal */
static void src_pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);

/* Process keyboard input */
static gboolean handle_keyboard (GIOChannel * source, GIOCondition cond, CustomData * data);

int main(int argc, char *argv[]) {
  //GstElement *pipeline, *audio_source, *tee, *audio_queue, *audio_convert, *audio_resample, *audio_sink;
  //GstElement *video_queue, *visual, *video_convert, *video_sink;
  CustomData data;

  GstBus *bus;
  GstMessage *msg;
  GstPad *tee_audio_pad, *tee_video_pad;
  GstPad *queue_audio_pad, *queue_video_pad;

  GIOChannel *io_stdin;
  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  //data.audio_source = gst_element_factory_make ("audiotestsrc", "audio_source");
  data.audio_source = gst_element_factory_make ("uridecodebin", "source");
  data.tee = gst_element_factory_make ("tee", "tee");
  data.audio_queue = gst_element_factory_make ("queue", "audio_queue");
  data.audio_convert = gst_element_factory_make ("audioconvert", "audio_convert");
  data.audio_resample = gst_element_factory_make ("audioresample", "audio_resample");
  data.audio_sink = gst_element_factory_make ("autoaudiosink", "audio_sink");
  data.video_queue = gst_element_factory_make ("queue", "video_queue");
  data.visual = gst_element_factory_make ("wavescope", "visual");
  data.video_convert = gst_element_factory_make ("videoconvert", "csp");
  data.video_sink = gst_element_factory_make ("autovideosink", "video_sink");

  /* Create the empty pipeline */
  data.pipeline = gst_pipeline_new ("test-pipeline");

  if (!data.pipeline || !data.audio_source || !data.tee || !data.audio_queue || !data.audio_convert || !data.audio_resample || !data.audio_sink || !data.video_queue || !data.visual || !data.video_convert || !data.video_sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Configure elements */
  //g_object_set (data.audio_source, "freq", 215.0f, NULL);
  g_object_set (data.audio_source, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/ghita.flac", NULL);
  //g_object_set (data.audio_source, "uri", "file:///home/truongtv/bt/gstreamer/file_example_MOV_480_700kB.mov", NULL);
  //g_object_set (data.visual, "shader", 0, "style", 1, NULL);

  /* Link all elements that can be automatically linked because they have "Always" pads */
  gst_bin_add_many (GST_BIN (data.pipeline), data.audio_source, data.tee, data.audio_queue, data.audio_convert, data.audio_resample, data.audio_sink, data.video_queue, data.visual, data.video_convert, data.video_sink, NULL);
  if (/*gst_element_link_many (data.audio_source, data.tee, NULL) != TRUE ||*/
      gst_element_link_many (data.audio_queue, data.audio_convert, data.audio_resample, data.audio_sink, NULL) != TRUE ||
      gst_element_link_many (data.video_queue, data.visual, data.video_convert, data.video_sink, NULL) != TRUE) {
            g_printerr ("Elements could not be linked.\n");
            gst_object_unref (data.pipeline);
            return -1;
  }

  /* Manually link the Tee, which has "Request" pads */
  
  tee_audio_pad = gst_element_get_request_pad (data.tee, "src_%u");
  g_print ("Obtained request pad %s for audio branch.\n", gst_pad_get_name (tee_audio_pad));
  queue_audio_pad = gst_element_get_static_pad (data.audio_queue, "sink");
  tee_video_pad = gst_element_get_request_pad (data.tee, "src_%u");
  g_print ("Obtained request pad %s for video branch.\n", gst_pad_get_name (tee_video_pad));
  queue_video_pad = gst_element_get_static_pad (data.video_queue, "sink");
  if (gst_pad_link (tee_audio_pad, queue_audio_pad) != GST_PAD_LINK_OK ||
      gst_pad_link (tee_video_pad, queue_video_pad) != GST_PAD_LINK_OK) {
    g_printerr ("Tee could not be linked.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }
  gst_object_unref (queue_audio_pad);
  gst_object_unref (queue_video_pad);

  //g_signal_connect (data.audio_source, "pad-added", G_CALLBACK (src_pad_added_handler), &data);


 /* Add a keyboard watch so we get notified of keystrokes */
 #ifdef G_OS_WIN32
    io_stdin = g_io_channel_win32_new_fd (fileno (stdin));
 #else
    io_stdin = g_io_channel_unix_new (fileno (stdin));
 #endif
    g_io_add_watch (io_stdin, G_IO_IN, (GIOFunc) handle_keyboard, &data);

  /* Start playing the pipeline */
  gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  //gst_element_set_state (data.pipeline, GST_STATE_PAUSED);
  data.index_source = -1;
  data.playing = FALSE;

  /* Wait until error or EOS */
  bus = gst_element_get_bus (data.pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  /* Release the request pads from the Tee, and unref them */

  gst_element_release_request_pad (data.tee, tee_audio_pad);
  gst_element_release_request_pad (data.tee, tee_video_pad);
  gst_object_unref (tee_audio_pad);
  gst_object_unref (tee_video_pad);

  /* Create a GLib Main Loop and set it to run */
  data.loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (data.loop);

  /* Free resources */
  if (msg != NULL)
    gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (data.pipeline, GST_STATE_NULL);

  gst_object_unref (data.pipeline);
  return 0;
}

/* This function will be called by the pad-added signal of source */
static void src_pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
  //GstPad *sink_pad = gst_element_get_static_pad (data->tee, "sink");
  GstPad *sink_pad = gst_element_get_compatible_pad (data->tee, new_pad, NULL);

  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;

  g_print ("Linking on Source and Tee \n");
  
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

  /*
  if (!g_str_has_prefix (new_pad_type, "audio/x-raw")) {
    g_print ("It has type '%s' which is not raw audio. Ignoring.\n", new_pad_type);
    goto exit;
  } 
  */

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
} 

/* Process keyboard input */
static gboolean handle_keyboard (GIOChannel * source, GIOCondition cond, CustomData * data)
{
  gchar *str = NULL;
  GstBus *bus;

  if (g_io_channel_read_line (source, &str, NULL, NULL,
          NULL) != G_IO_STATUS_NORMAL) {
    return TRUE;
  }
  
  bus = gst_element_get_bus (data->pipeline);
  /*
  if (data->playing) {
     gst_bus_post (bus, gst_message_new_eos());
  }
  */
  switch (g_ascii_tolower (str[0])) {
    case '1':
      //gst_element_set_state (data->pipeline, GST_STATE_NULL);
      //gst_element_set_state (data->pipeline, GST_STATE_READY);
      g_object_set (data->audio_source, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/ghita.flac", NULL);
      data->index_source = 1;
      g_print ("CASE_1:\n");

      //data->playing = !data->playing;
      //gst_element_set_state (data->pipeline,
          //data->playing ? GST_STATE_PLAYING : GST_STATE_PAUSED);
      //g_print ("Setting state to %s\n", data->playing ? "PLAYING" : "PAUSE");
      break;
    case '2':
      //gst_element_set_state (data->pipeline, GST_STATE_NULL);
      //gst_element_set_state (data->pipeline, GST_STATE_READY);
      g_object_set (data->audio_source, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/Awake.flac", NULL);
      data->index_source = 2;
      g_print ("CASE_2:\n");
      break;
    case '3':
      //gst_element_set_state (data->pipeline, GST_STATE_NULL);
      //gst_element_set_state (data->pipeline, GST_STATE_READY);
      g_object_set (data->audio_source, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/Dancing.flac", NULL);
      data->index_source = 3;
      g_print ("CASE_3:\n");
      break;
    default:
      break;
  }

  if(data->index_source != -1) {
    g_signal_connect (data->audio_source, "pad-added", G_CALLBACK (src_pad_added_handler), data);
    gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
    data->playing = TRUE;
    g_print ("SET SOURCE AUDIO :\n");
  }
  g_print ("KET THUC XU LI TU BAN PHIM :\n");
  g_free (str);

  return TRUE;
}

