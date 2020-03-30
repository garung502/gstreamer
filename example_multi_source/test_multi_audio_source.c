/* GStreamer
 * Copyright (C) 2003 Julien Moutte <julien@moutte.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <gst/gst.h>

static GMainLoop *loop = NULL;

static gboolean my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
  g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *err;
      gchar *debug;

      gst_message_parse_error (message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);

      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_EOS:
      /* end-of-stream */
      g_main_loop_quit (loop);
      break;
    default:
      /* unhandled message */
      break;
  }

  /* we want to be notified again the next time there is a message
   * on the bus, so returning TRUE (FALSE means we want to stop watching
   * for messages on the bus and our callback should not be called again)
   */
  return TRUE;
}



static gboolean switch_timer (GstElement * audio_switch)
{
  gint nb_sources;
  GstPad *active_pad, *new_pad;
  gchar *active_name;

  g_message ("switching");
  g_object_get (G_OBJECT (audio_switch), "n-pads", &nb_sources, NULL);
  g_object_get (G_OBJECT (audio_switch), "active-pad", &active_pad, NULL);

  active_name = gst_pad_get_name (active_pad);
  if (strcmp (active_name, "sink_0") == 0) {
    new_pad = gst_element_get_static_pad (audio_switch, "sink_1");
  } else {
    new_pad = gst_element_get_static_pad (audio_switch, "sink_0");
  }
  g_object_set (G_OBJECT (audio_switch), "active-pad", new_pad, NULL);
  g_free (active_name);
  gst_object_unref (new_pad);

  g_message ("current number of sources : %d, active source %s",
      nb_sources, gst_pad_get_name (active_pad));

  return (GST_STATE (GST_ELEMENT (audio_switch)) == GST_STATE_PLAYING);
}

/* Process keyboard input */
static gboolean handle_keyboard (GIOChannel * source, GIOCondition cond, GstElement * audio_switch)
{
  gchar *str = NULL;
  gint nb_sources;
  GstPad *active_pad, *new_pad;
  gchar *active_name;

  g_message ("switching");
  g_object_get (G_OBJECT (audio_switch), "n-pads", &nb_sources, NULL);
  g_object_get (G_OBJECT (audio_switch), "active-pad", &active_pad, NULL);

  active_name = gst_pad_get_name (active_pad);


  if (g_io_channel_read_line (source, &str, NULL, NULL,
          NULL) != G_IO_STATUS_NORMAL) {
    return TRUE;
  }
  
  switch (g_ascii_tolower (str[0])) {
    case '1':
      new_pad = gst_element_get_static_pad (audio_switch, "sink_0");
      break;
    case '2':
      new_pad = gst_element_get_static_pad (audio_switch, "sink_1");
      break;
    default:
      break;
  }

  g_object_set (G_OBJECT (audio_switch), "active-pad", new_pad, NULL);

  g_free (active_name);
  gst_object_unref (new_pad);
  g_free (str);

  g_message ("current number of sources : %d, active source %s", nb_sources, gst_pad_get_name (active_pad));
  g_print ("KET THUC XU LI TU BAN PHIM :\n");

  return (GST_STATE (GST_ELEMENT (audio_switch)) == GST_STATE_PLAYING);

}


static void last_message_received (GObject * segment)
{
  gchar *last_message;

  g_object_get (segment, "last_message", &last_message, NULL);
  g_print ("last-message: %s\n", last_message);
  g_free (last_message);
}

/* This function will be called by the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *new_pad, GstElement *data) {
  gchar *name;
  GstPad *sink_pad = NULL;
  g_object_get (G_OBJECT (src), "name", &name, NULL);
  if (!strcmp(name, "source_1")) {
    sink_pad = gst_element_get_static_pad (data, "sink");
    g_print ("SELECT SOURCE 1.\n");
  }
  else if (!strcmp(name, "source_2")) {
    sink_pad = gst_element_get_static_pad (data, "sink");
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


int main (int argc, char *argv[])
{
  GstElement *pipeline, *src1, *src2, *audio_switch, *audio_sink, *segment;
  GstElement *convert;
  GstElement *resample;

  GstElement *sink1_sync, *sink2_sync;/*, *capsfilter;*/
  GstBus *bus;
  GIOChannel *io_stdin;

  /* Initing GStreamer library */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  pipeline = gst_pipeline_new ("pipeline");

  //src1 = gst_element_factory_make ("audiotestsrc", "src1");
  //g_object_set (src1, "freq", 150.0f, NULL);

  //src2 = gst_element_factory_make ("audiotestsrc", "src2");
  //g_object_set (src2, "freq", 750.0f, NULL);

  src1 = gst_element_factory_make ("uridecodebin", "source_1");
  g_object_set (src1, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/ghita.flac", NULL);

  src2 = gst_element_factory_make ("uridecodebin", "source_2");
  g_object_set (src2, "uri", "file:///home/truongtv/bt/gstreamer/file_audio/Awake.flac", NULL);

  /*
  capsfilter = gst_element_factory_make ("capsfilter", "caps0");
  g_object_set (G_OBJECT (capsfilter), "caps", gst_caps_from_string ("video/x-raw,width=640,height=480"), NULL);
 */
  audio_switch = gst_element_factory_make ("input-selector", "audio_switch");

  segment = gst_element_factory_make ("identity", "identity-segment");
  g_object_set (G_OBJECT (segment), "silent", TRUE, NULL);
  g_signal_connect (G_OBJECT (segment), "notify::last-message", G_CALLBACK (last_message_received), segment);
  g_object_set (G_OBJECT (segment), "single-segment", TRUE, NULL);

  convert = gst_element_factory_make ("audioconvert", "convert");
  resample = gst_element_factory_make ("audioresample", "resample");

  audio_sink = gst_element_factory_make ("autoaudiosink", "audio_sink");

  sink1_sync = gst_element_factory_make ("identity", "sink0_sync");
  g_object_set (G_OBJECT (sink1_sync), "sync", TRUE, NULL);

  sink2_sync = gst_element_factory_make ("identity", "sink1_sync");
  g_object_set (G_OBJECT (sink2_sync), "sync", TRUE, NULL);

  gst_bin_add_many (GST_BIN (pipeline), src1, src2, sink1_sync, sink2_sync, audio_switch, segment, convert, resample, audio_sink, NULL);

  gst_element_link (src1, sink1_sync);
  gst_element_link (sink1_sync, audio_switch);

  //gst_element_link (src2, capsfilter);
  gst_element_link (src2, sink2_sync);
  gst_element_link (sink2_sync, audio_switch);
  gst_element_link (audio_switch, segment);

  gst_element_link (segment, convert);
  gst_element_link (convert, resample);
  gst_element_link (resample,audio_sink);

  //gst_element_link (segment,    /*scaler);
  // gst_element_link (scaler, */ audio_sink);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, my_bus_callback, NULL);
  gst_object_unref (bus);

  g_signal_connect (src2, "pad-added", G_CALLBACK (pad_added_handler), sink2_sync);
  g_signal_connect (src1, "pad-added", G_CALLBACK (pad_added_handler), sink1_sync);
  //g_timeout_add (5000, (GSourceFunc) switch_timer, audio_switch);

   #ifdef G_OS_WIN32
    io_stdin = g_io_channel_win32_new_fd (fileno (stdin));
   #else
    io_stdin = g_io_channel_unix_new (fileno (stdin));
   #endif
    g_io_add_watch (io_stdin, G_IO_IN, (GIOFunc) handle_keyboard, audio_switch);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_main_loop_run (loop);

  //gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_READY);

  /* unref */
  gst_object_unref (GST_OBJECT (pipeline));

  exit (0);
}
