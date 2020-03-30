#include <gst/gst.h>
#include <glib.h>


/////////////////////////////////////////////// ---------- The Begin ------------------------ /////////////////////////////////////////
/*
typedef struct myDataTag {
GstElement *pipeline;
GstElement *rtspsrc;
GstElement *depayloader;
GstElement *decoder;
*sink;
} myData_t;

myData_t appData;

appData->pipeline  = gst_pipeline_new ("videoclient");
appData->rtspsrc = gst_element_factory_make ("rtspsrc", "rtspsrc");
g_object_set (G_OBJECT (appData->rtspsrc), "location", "rtsp://192.168.1.10:554/myStreamPath", NULL);
appData->depayloader = gst_element_factory_make ("rtph264depay","depayloader");
appData->decoder = gst_element_factory_make ("h264dec", "decoder");
appData->sink = gst_element_factory_make ("autovideosink", "sink");

//then add all elements together
gst_bin_add_many (GST_BIN (appData->pipeline), appData->rtspsrc, appData->depayloader, appData->decoder, appData->sink, NULL);

//link everythink after source
gst_element_link_many (appData->depayloader, appData->decoder, appData->sink, NULL);


 // Connect to the pad-added signal for the rtpbin.  This allows us to link
 // the dynamic RTP source pad to the depayloader when it is created.
g_signal_connect (appData->rtspsrc, "pad-added", G_CALLBACK (pad_added_handler), &appData);

// Set the pipeline to "playing" state
gst_element_set_state (appData->pipeline, GST_STATE_PLAYING);

// pad added handler 
static void pad_added_handler (GstElement *src, GstPad *new_pad, myData_t *pThis) {
GstPad *sink_pad = gst_element_get_static_pad (pThis->depayloader, "sink");
GstPadLinkReturn ret;
GstCaps *new_pad_caps = NULL;
GstStructure *new_pad_struct = NULL;
const gchar *new_pad_type = NULL;

g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

// Check the new pad's name 
if (!g_str_has_prefix (GST_PAD_NAME (new_pad), "recv_rtp_src_")) {
    g_print ("  It is not the right pad.  Need recv_rtp_src_. Ignoring.\n");
    goto exit;
}

// If our converter is already linked, we have nothing to do here
if (gst_pad_is_linked (sink_pad)) {
    g_print (" Sink pad from %s already linked. Ignoring.\n", GST_ELEMENT_NAME (src));
    goto exit;
}

// Check the new pad's type
new_pad_caps = gst_pad_get_caps (new_pad);
new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
new_pad_type = gst_structure_get_name (new_pad_struct);

// Attempt the link 
ret = gst_pad_link (new_pad, sink_pad);
if (GST_PAD_LINK_FAILED (ret)) {
    g_print ("  Type is '%s' but link failed.\n", new_pad_type);
} else {
    g_print ("  Link succeeded (type '%s').\n", new_pad_type);
}

exit:
// Unreference the new pad's caps, if we got them //
if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);

// Unreference the sink pad 
gst_object_unref (sink_pad);
}

*/
/////////////////////////////////////////////// ---------- The End ------------------------ /////////////////////////////////////////


/*
static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
 GMainLoop *loop = (GMainLoop *) data;
 switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
             g_print ("End of stream\n");
             g_main_loop_quit (loop);
             break;
        case GST_MESSAGE_ERROR: {
             gchar *debug;
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
int main (int argc, char *argv[])
{
GMainLoop *loop;
GstElement *pipeline, *source, *sink;
GstBus *bus;
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    if (argc != 2) {
             return -1;
       }
    pipeline = gst_pipeline_new ("network-player");
    source = gst_element_factory_make ("rtspsrc","file-source");
    sink = gst_element_factory_make ("fakesink","fake");

    if (!pipeline || !source || !sink) {
                  g_printerr ("One element could not be created. Exiting.\n");
                  return -1;
       }
    g_object_set (G_OBJECT (source), "location", argv[1], NULL);
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    gst_bin_add_many (GST_BIN (pipeline),source, sink, NULL);
    gst_element_link_many (source, sink, NULL);

    // Set the pipeline to "playing" state 
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    // Iterate 
    g_print ("Running...\n");
    g_main_loop_run (loop);

    // Out of the main loop, clean up nicely
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);

    g_print ("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));
    return 0;
    }

*/


//#include <gst/gst.h>
//#include <glib.h>

static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ERROR:{
      gchar *debug;
      GError *error;
      gst_message_parse_error (msg, &error, &debug);
      g_printerr ("ERROR from element %s: %s\n",
          GST_OBJECT_NAME (msg->src), error->message);
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
main (int argc, char *argv[])
{
  GMainLoop *loop = NULL;
  GstElement *pipeline = NULL, 
             *source = NULL, 
             *rtpdepay = NULL,
             *vidqueue = NULL,
             *h264parser = NULL,
             *decoder = NULL, 
             *vidconvert = NULL, 
             *vidscale = NULL, 
             *sink = NULL;

  GstBus *bus = NULL;
  guint bus_watch_id;
  GstCaps *caps1 = NULL, *caps2 = NULL;
  gulong osd_probe_id = 0;
  GstPad *osd_sink_pad = NULL;

  /* GStreamer initialization */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("pipeline");
  source = gst_element_factory_make ("rtspsrc", "file-source");
  rtpdepay = gst_element_factory_make ("rtph264depay", "rtpdepay");
  vidqueue = gst_element_factory_make ("queue", "vidqueue");
  h264parser = gst_element_factory_make ("h264parse", "h264parser");
  decoder = gst_element_factory_make ("avdec_h264", "avh264decoder");
  vidconvert = gst_element_factory_make ("videoconvert", "vidconvert");
  vidscale = gst_element_factory_make ("videoscale", "vidscale");
  sink = gst_element_factory_make ("autovideosink", "sink");

  /* Check elements creation */
  if (!pipeline   || 
      !source     || 
      !rtpdepay   || 
      !vidqueue   ||
      !h264parser || 
      !decoder    || 
      !vidconvert || 
      !vidscale   || 
      !sink) {
    g_printerr ("One or more element could not be created. Exiting.\n");
    return -1;
  }

  /* Set input location to the source element */
  g_object_set (G_OBJECT (source), "location", argv[1], NULL);

  /* Add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* Set up the pipeline */
  /* Add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, 
                    rtpdepay, 
                    vidqueue,  
                    h264parser, 
                    decoder, 
                    vidconvert, 
                    vidscale, 
                    sink, 
                    NULL);

  /* Link the elements together */
  gst_element_link_many (source, 
                         rtpdepay, 
                         vidqueue,  
                         h264parser, 
                         decoder, 
                         vidconvert, 
                         vidscale, 
                         sink, 
                         NULL);

  /* Set the pipeline to "playing" state */
  g_print ("Now playing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* Wait till pipeline encounters an error or EOS */
  g_print ("Running...\n");
  g_main_loop_run (loop);

  /* Out of the main loop */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);
  return 0;
}
