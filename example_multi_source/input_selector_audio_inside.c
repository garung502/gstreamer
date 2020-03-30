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


#include <gst/gst.h>

static GMainLoop *loop = NULL;

static gboolean
my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
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



static gboolean
switch_timer (GstElement * audio_switch)
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

static void
last_message_received (GObject * segment)
{
  gchar *last_message;

  g_object_get (segment, "last_message", &last_message, NULL);
  g_print ("last-message: %s\n", last_message);
  g_free (last_message);
}

int
main (int argc, char *argv[])
{
  GstElement *pipeline, *src1, *src2, *audio_switch, *audio_sink, *segment;
  GstElement *convert;
  GstElement *resample;

  GstElement *sink1_sync, *sink2_sync;/*, *capsfilter;*/
  GstBus *bus;

  /* Initing GStreamer library */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  pipeline = gst_pipeline_new ("pipeline");

  src1 = gst_element_factory_make ("audiotestsrc", "src1");
  g_object_set (src1, "freq", 150.0f, NULL);

  src2 = gst_element_factory_make ("audiotestsrc", "src2");
  g_object_set (src2, "freq", 750.0f, NULL);

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
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_timeout_add (5000, (GSourceFunc) switch_timer, audio_switch);

  g_main_loop_run (loop);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_READY);

  /* unref */
  gst_object_unref (GST_OBJECT (pipeline));

  exit (0);
}
