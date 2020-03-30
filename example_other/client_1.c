#include <gst/gst.h>
#include <glib.h>
#include <string.h>


static GstElement *source, *demuxer, *vdqueue, *adqueue, *vdsink,
*adsink, *decvd, *decad;

void on_pad_added (GstElement *element, GstPad *pad)
{
        g_debug ("Signal: pad-added");
        GstCaps *caps;
        GstStructure *str;

        caps = gst_pad_get_current_caps (pad);
        g_assert (caps != NULL);
        str = gst_caps_get_structure (caps, 0);
        g_assert (str != NULL);

        if (g_strrstr (gst_structure_get_name (str), "video")) {
                g_debug ("Linking video pad to dec_vd");
                // Link it actually
                GstPad *targetsink = gst_element_get_static_pad (decvd, "sink");
                g_assert (targetsink != NULL);
                gst_pad_link (pad, targetsink);
                gst_object_unref (targetsink);
        }

        if (g_strrstr (gst_structure_get_name (str), "audio")) {
                g_debug ("Linking audio pad to dec_ad");
                // Link it actually
                GstPad *targetsink = gst_element_get_static_pad (decad, "sink");
                g_assert (targetsink != NULL);
                gst_pad_link (pad, targetsink);
                gst_object_unref (targetsink);
        }

        gst_caps_unref (caps);
}

static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
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

int
main (int   argc,
      char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline;
  GstBus *bus;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);


  /* Check input arguments */
  if (argc != 2) {
    g_printerr ("Usage: %s <AVI (h264+mp3) filename>\n", argv[0]);
    return -1;
  }


  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("media-player");
  source = gst_element_factory_make ("filesrc", "file-source");
  demuxer = gst_element_factory_make ("avidemux", "avi-demuxer");
  decvd = gst_element_factory_make ("ffdec_h264", "h264-decoder");
  decad = gst_element_factory_make ("mad", "mp3-decoder");
  vdsink = gst_element_factory_make ("autovideosink", "video-sink");
  vdqueue = gst_element_factory_make ("queue", "video-queue");
  adqueue = gst_element_factory_make ("queue", "audio-queue");
  adsink = gst_element_factory_make ("alsasink", "audio-sink");

  if (!pipeline || !source || !demuxer || !decvd || !decad || !vdsink
|| !vdqueue || !adqueue || !adsink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */

  /* we set the input filename to the source element */
  g_object_set (G_OBJECT (source), "location", argv[1], NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, demuxer, decvd, decad, adqueue, vdqueue,
vdsink, adsink,  NULL);
  //gst_bin_add_many (GST_BIN (pipeline),
  //                  source, demuxer, decvd, vdqueue, vdsink,  NULL);

  /* we link the elements together */
  /* file-source -> demuxer ~> {video-decoder -> video-output} ~>
{audio-decoder -> alsa-output} */
  gst_element_link (source, demuxer);
  gst_element_link (decvd, vdqueue);
  gst_element_link (vdqueue, vdsink);
  gst_element_link (decad, adqueue);
  gst_element_link (adqueue, adsink);

  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), NULL);

  /* note that the demuxer will be linked to the decoder dynamically.
     The reason is that Media may contain various streams (for example
     audio and video). The source pad(s) will be created at run time,
     by the demuxer when it detects the amount and nature of streams.
     Therefore we connect a callback function which will be executed
     when the "pad-added" is emitted.*/

  /* Set the pipeline to "playing" state*/
  g_print ("Now playing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}
