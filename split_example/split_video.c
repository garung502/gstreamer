#include <gst/gst.h>
#include <unistd.h>
int main (int argc, char **argv)
{
 GstElement *pipeline;
 GError *error = NULL;

 gst_init (&argc, &argv);

 //pipeline = gst_parse_launch ("rtspsrc location=rtsp://127.0.0.1:8554/test ! rtph264depay ! h264parse ! splitmuxsink max-size-time=2 location=video%05d.mp4", &error);


 pipeline = gst_parse_launch ("playbin uri=file:////home/truongtv/bt/gstreamer/split_example/videoplayback.mp4 ! rtph264depay ! h264parse ! splitmuxsink max-size-time=6000000 location=video%05d.mp4", &error);

 if (!pipeline)
 g_error ("Error: %s", error->message);
 gst_element_set_state(pipeline, GST_STATE_PLAYING);
 //pause();
 gst_element_set_state(pipeline, GST_STATE_NULL);
 return 0;
}
