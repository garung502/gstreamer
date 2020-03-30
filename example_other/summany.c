#include <stdio.h>
#include <gst/gst.h>
#include <string.h>

typedef struct _CustomData {
  gboolean is_live;
  GstElement *pipeline;
  GMainLoop *loop;

  GstElement *video_sink;
  gboolean playing;        /* Playing or Paused */ /* Are we in the PLAYING state? */
  gdouble rate;            /* Current playback rate (can be negative) */ 

  GstElement *playbin;  /* Our one and only element */
  gboolean terminate;    /* Should we terminate execution? */
  gboolean seek_enabled; /* Is seeking enabled for this media? */
  gboolean seek_done;    /* Have we performed the seek already? */
  gint64 duration;       /* How long does this media last, in nanoseconds */
 
} CustomData;

static void cb_message (GstBus *bus, GstMessage *msg, CustomData *data) {

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR: {
      GError *err;
      gchar *debug;

      gst_message_parse_error (msg, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);

      gst_element_set_state (data->pipeline, GST_STATE_READY);
      g_main_loop_quit (data->loop);
      break;
    }
    case GST_MESSAGE_EOS:
      /* end-of-stream */
      gst_element_set_state (data->pipeline, GST_STATE_READY);
      g_main_loop_quit (data->loop);
      break;
    /*Case here lien quan den hien tuong mang cham hay chap chon --> phai nhan du buffer moi thuc hien playback*/
    case GST_MESSAGE_BUFFERING: {
      gint percent = 0;

      /* If the stream is live, we do not care about buffering. */
      if (data->is_live) break;

      gst_message_parse_buffering (msg, &percent);
      g_print ("Buffering (%3d%%)\r", percent);
      /* Wait until buffering is complete before start/resume playing */
      if (percent < 100)
        gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
      else
        gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
      break;
    }
    /*Case here lien quan den su co mang nhu mat mang*/
    case GST_MESSAGE_CLOCK_LOST:
      /* Get a new clock */
      gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
      gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
      break;
    default:
      /* Unhandled message */
      break;
    }
}

/* Send seek event to change rate */
static void send_seek_event (CustomData * data)
{
  gint64 position;
  GstEvent *seek_event;

  /* Obtain the current position, needed for the seek event */
  if (!gst_element_query_position (data->pipeline, GST_FORMAT_TIME, &position)) {
    g_printerr ("Unable to retrieve current position.\n");
    return;
  }
  
  /* Create the seek event */
  if (data->rate > 0) {
    seek_event =
        gst_event_new_seek (data->rate, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
        position, GST_SEEK_TYPE_END, 0);
  } else {
    seek_event =
        gst_event_new_seek (data->rate, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET, 0,
        GST_SEEK_TYPE_SET, position);
  }

  if (data->video_sink == NULL) {
    /* If we have not done so, obtain the sink through which we will send the seek events */
    g_object_get (data->pipeline, "video-sink", &data->video_sink, NULL);
  }

  /* Send the event */
  gst_element_send_event (data->video_sink, seek_event);

  g_print ("Current rate: %g\n", data->rate);
}

/* Process keyboard input */
static gboolean handle_keyboard (GIOChannel * source, GIOCondition cond, CustomData * data)
{
  gchar *str = NULL;

  if (g_io_channel_read_line (source, &str, NULL, NULL,
          NULL) != G_IO_STATUS_NORMAL) {
    return TRUE;
  }

  switch (g_ascii_tolower (str[0])) {
    case 'p':
      data->playing = !data->playing;
      gst_element_set_state (data->pipeline,
          data->playing ? GST_STATE_PLAYING : GST_STATE_PAUSED);
      g_print ("Setting state to %s\n", data->playing ? "PLAYING" : "PAUSE");
      break;
    case 's':
      if (g_ascii_isupper (str[0])) {
        data->rate *= 2.0;
      } else {
        data->rate /= 2.0;
      }
      send_seek_event (data);
      break;
    case 'd':
      data->rate *= -1.0;
      send_seek_event (data);
      break;
    case 'n':
      if (data->video_sink == NULL) {
        /* If we have not done so, obtain the sink through which we will send the step events */
        g_object_get (data->pipeline, "video-sink", &data->video_sink, NULL);
      }

      gst_element_send_event (data->video_sink,
          gst_event_new_step (GST_FORMAT_BUFFERS, 100, ABS (data->rate), TRUE,
              FALSE));
      g_print ("Stepping one hundred frame\n");
      break;
    case 'q':
      g_main_loop_quit (data->loop);
      break;
    default:
      break;
  }

  g_free (str);

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static void handle_message (CustomData *data, GstMessage *msg) {
  GError *err;
  gchar *debug_info;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error (msg, &err, &debug_info);
      g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
      g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
      g_clear_error (&err);
      g_free (debug_info);
      data->terminate = TRUE;
      break;
    case GST_MESSAGE_EOS:
      g_print ("End-Of-Stream reached.\n");
      data->terminate = TRUE;
      break;
    case GST_MESSAGE_DURATION:
      /* The duration has changed, mark the current one as invalid */
      data->duration = GST_CLOCK_TIME_NONE;
      break;
    case GST_MESSAGE_STATE_CHANGED: {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
      if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->playbin)) {
        g_print ("Pipeline state changed from %s to %s:\n",
            gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));

        /* Remember whether we are in the PLAYING state or not */
        data->playing = (new_state == GST_STATE_PLAYING);

        if (data->playing) {
          /* We just moved to PLAYING. Check if seeking is possible */
          GstQuery *query;
          gint64 start, end;
          query = gst_query_new_seeking (GST_FORMAT_TIME);
          if (gst_element_query (data->playbin, query)) {
            gst_query_parse_seeking (query, NULL, &data->seek_enabled, &start, &end);
            if (data->seek_enabled) {
              g_print ("Seeking is ENABLED from %" GST_TIME_FORMAT " to %" GST_TIME_FORMAT "\n",
                  GST_TIME_ARGS (start), GST_TIME_ARGS (end));
            } else {
              g_print ("Seeking is DISABLED for this stream.\n");
            }
          }
          else {
            g_printerr ("Seeking query failed.");
          }
          gst_query_unref (query);
        }
      }
    } break;
    default:
      /* We should not reach here */
      g_printerr ("Unexpected message received.\n");
      break;
  }
  gst_message_unref (msg);
}
/////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  GstElement *pipeline;
  GstBus *bus;
  GstStateChangeReturn ret;
  GMainLoop *main_loop;
  CustomData data;

  GIOChannel *io_stdin;

  GstMessage *msg;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Initialize our data structure */
  memset (&data, 0, sizeof (data));

  data.playing = FALSE;
  data.terminate = FALSE;
  data.seek_enabled = FALSE;
  data.seek_done = FALSE;
  data.duration = GST_CLOCK_TIME_NONE;


  /* Print usage map */
    /*
  g_print ("USAGE: Choose one of the following options, then press enter:\n"
      " 'P' to toggle between PAUSE and PLAY\n"
      " 'S' to increase playback speed, 's' to decrease playback speed\n"
      " 'D' to toggle playback direction\n"
      " 'N' to move to next frame (in the current direction, better in PAUSE)\n"
      " 'Q' to quit\n");
    */
  /* Build the pipeline */
  //pipeline = gst_parse_launch ("playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);
  pipeline = gst_parse_launch ("playbin uri=rtsp://127.0.0.1:8554/test", NULL);

  g_object_set (data.playbin, "uri", "rtsp://127.0.0.1:8554/test", NULL);
  //pipeline = gst_parse_launch (data.playbin, NULL);

  /* listen bus on pipeline */
  bus = gst_element_get_bus (pipeline);
  ///////////////////////////////////////////////////////////////////////////////////////////
      do {
    msg = gst_bus_timed_pop_filtered (bus, 100 * GST_MSECOND,
        GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_DURATION);

    /* Parse message */
    if (msg != NULL) {
      handle_message (&data, msg);
    } else {
      /* We got no message, this means the timeout expired */
      if (data.playing) {
        gint64 current = -1;

        /* Query the current position of the stream */
        if (!gst_element_query_position (data.playbin, GST_FORMAT_TIME, &current)) {
          g_printerr ("Could not query current position.\n");
        }

        /* If we didn't know it yet, query the stream duration */
        if (!GST_CLOCK_TIME_IS_VALID (data.duration)) {
          if (!gst_element_query_duration (data.playbin, GST_FORMAT_TIME, &data.duration)) {
            g_printerr ("Could not query current duration.\n");
          }
        }

        /* Print current position and total duration */
        g_print ("Position %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
            GST_TIME_ARGS (current), GST_TIME_ARGS (data.duration));

        /* If seeking is enabled, we have not done it yet, and the time is right, seek */
        if (data.seek_enabled && !data.seek_done && current > 10 * GST_SECOND) {
          g_print ("\nReached 10s, performing seek...\n");
          gst_element_seek_simple (data.playbin, GST_FORMAT_TIME,
              GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, 30 * GST_SECOND);
          data.seek_done = TRUE;
        }
      }
    }
  } while (!data.terminate);
 
  //////////////////////////////////////////////////////////////////////////////////////////


  /* Add a keyboard watch so we get notified of keystrokes */
  #ifdef G_OS_WIN32
    io_stdin = g_io_channel_win32_new_fd (fileno (stdin));
  #else
    io_stdin = g_io_channel_unix_new (fileno (stdin));
  #endif
  g_io_add_watch (io_stdin, G_IO_IN, (GIOFunc) handle_keyboard, &data);

  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  } else if (ret == GST_STATE_CHANGE_NO_PREROLL) {
    data.is_live = TRUE;
  }
  data.playing = TRUE;
  data.rate = 1.0;

  /* Create a GLib Main Loop and set it to run */
  main_loop = g_main_loop_new (NULL, FALSE);
  data.loop = main_loop;
  data.pipeline = pipeline;

  gst_bus_add_signal_watch (bus);
  //g_signal_connect (bus, "message", G_CALLBACK (cb_message), &data);  // message

  g_main_loop_run (main_loop);

  /* Free resources */
  g_main_loop_unref (main_loop);
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

  g_io_channel_unref (io_stdin);
  if (data.video_sink != NULL) {
    gst_object_unref (data.video_sink);
  }
  gst_object_unref (data.pipeline);
  
  return 0;
}
