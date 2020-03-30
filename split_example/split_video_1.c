#include <gst/gst.h>
#include<stdio.h>
#include<stdlib.h>

static void cb_new_rtspsrc_pad(GstElement *element,GstPad*pad,gpointer  data)
{
gchar *name;
GstElement *p_rtph264depay;

name = gst_pad_get_name(pad);
g_print("A new pad %s was created\n", name);
p_rtph264depay = GST_ELEMENT(data);

if(!gst_element_link_pads(element, name, p_rtph264depay, "sink"))
{
    printf("Failed to link elements 3\n");
}
g_free(name);
}

static void pts_analysis_cb(GstElement *splitmuxsink, guint id , GstSample *sample,  gpointer user_data)   
  {
  GstBuffer *buffer=gst_sample_get_buffer(sample); 
  GstClockTime pts1 = GST_BUFFER_PTS(buffer);
  printf("lff - *%ld*   \n  ",pts1);
  g_print("Hello from above");
  }

static void pts_analysis_call(GstElement *identity,  GstBuffer *buffer, gpointer user_data) 
    {
   GstClockTime pts = GST_BUFFER_PTS(buffer);
    printf("id- *%ld* ",pts);
}

  int main(int argc, char *argv[]) {
  GstElement *pipeline,*rtspsrc,*depay,*h264,*splitmux,*id;
  GstBus *bus;
  GstMessage *msg;
  gst_init (&argc, &argv);

  pipeline=gst_pipeline_new("pipeline");

  //rtspsrc=gst_element_factory_make("rtspsrc","src");
  //g_object_set(G_OBJECT(rtspsrc),"location","rtsp://127.0.0.1:8554/test",NULL);
  
  rtspsrc=gst_element_factory_make("uridecodebin", "source");
  g_object_set(G_OBJECT(rtspsrc), "uri", "file:////home/truongtv/bt/gstreamer/split_example/videoplayback.mp4", NULL);

 depay=gst_element_factory_make("rtph264depay","depay");
 id=gst_element_factory_make("identity","id");
  h264=gst_element_factory_make("h264parse","h264");

  splitmux=gst_element_factory_make("splitmuxsink","mux");
  g_object_set(G_OBJECT(splitmux),"location","/home/fti/gstpatch/video%02d.mp4","max-size-time","6000000000",NULL);

  bus = gst_element_get_bus (pipeline);

  gst_bin_add_many(GST_BIN(pipeline),rtspsrc,depay,NULL);
  g_signal_connect(rtspsrc,"pad-added",G_CALLBACK(cb_new_rtspsrc_pad),depay);
  gst_bin_add_many(GST_BIN(pipeline),id,NULL);
  printf("*%d*",gst_element_link(depay,id));
    gst_bin_add_many(GST_BIN(pipeline),h264,NULL);
  printf("*%d*",gst_element_link(id,h264));

    g_signal_connect_data(id, "handoff",G_CALLBACK(pts_analysis_call), NULL, NULL, G_CONNECT_AFTER);

  gst_bin_add_many(GST_BIN(pipeline),splitmux,NULL);
  if(!gst_element_link(h264,splitmux))
   { printf("Failed");}

  g_signal_connect_data(splitmux,"format-location-full", G_CALLBACK(pts_analysis_cb),NULL,NULL,G_CONNECT_AFTER);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  if (msg != NULL)
  {
  gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  }
  gst_object_unref (pipeline);
  return 0;
  }
