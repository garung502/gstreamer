static void on_pad_added (GstElement *element, GstPad *pad, gpointer data)
{
GstPad *sinkpad;
GstElement *decoder = (GstElement *) data;

/* We can now link this pad with the vorbis-decoder sink pad */
g_print ("Dynamic pad created, linking demuxer/decoder\n");

sinkpad = gst_element_get_static_pad (decoder, "sink");

gst_pad_link (pad, sinkpad);

gst_object_unref (sinkpad);
}

/* Main method for the bus monitoring code */
-(void) app_function
{
GstBus *bus;
GSource *bus_source;

GstElement *source1, *clrspace1, *clrspace2, *clrspace, *videobox1, *sink, *source2, *mpegenc, *avimux;
GstElement *videomixer;
GstElement *decodebin1, *decodebin2;
GstElement *matroskademux1, *vp8dec1;

GST_DEBUG ("Creating pipeline");

/* Create our own GLib Main Context and make it the default one */
context = g_main_context_new ();
g_main_context_push_thread_default(context);

/* Build pipeline */
pipeline = gst_pipeline_new("pipelinecomposer");
source1 = gst_element_factory_make("filesrc", "source1");
source2 = gst_element_factory_make("filesrc", "source2");
videobox1 = gst_element_factory_make("videobox", "videobox");
videomixer = gst_element_factory_make("videomixer", "videomixer");
clrspace1 = gst_element_factory_make("videoconvert", "clrspace1");
clrspace2 = gst_element_factory_make("videoconvert", "clrspace2");
clrspace = gst_element_factory_make("videoconvert", "clrspace");
sink = gst_element_factory_make("filesink", "sink");
mpegenc = gst_element_factory_make("x264enc", "mpegenc");
avimux = gst_element_factory_make("mpegtsmux", "avimux");
decodebin1 = gst_element_factory_make("decodebin", "decodebin1");
decodebin2 = gst_element_factory_make("decodebin", "decodebin2");
matroskademux1 = gst_element_factory_make("matroskademux", "matroskademux1");
vp8dec1 = gst_element_factory_make("vp8dec", "vp8dec1");


NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
NSString *documentsDirectory = [paths objectAtIndex:0];
NSString *filePath = [NSString stringWithFormat:@"%@/%@", documentsDirectory, @"1.avi"];

NSString *filePath_1 = [NSString stringWithFormat:@"%@", @"/Users/johanbasore1/Movies/1.webm"];
NSString *filePath_2 = [NSString stringWithFormat:@"%@", @"/Users/johanbasore1/Movies/2.webm"];

g_object_set (G_OBJECT (source1), "location", [filePath_1 UTF8String], NULL);
g_object_set (G_OBJECT (source2), "location", [filePath_2 UTF8String], NULL);

g_object_set (G_OBJECT (sink), "location", [filePath UTF8String], NULL);

g_object_set(videobox1,"border-alpha",0,"top",0,"left",0,NULL);
g_signal_connect(matroskademux1, "pad-added", G_CALLBACK(on_pad_added), vp8dec1);

gst_bin_add_many(GST_BIN (pipeline), source1, matroskademux1, vp8dec1 , sink, clrspace1, mpegenc, avimux, nil);
gst_element_link(source1, matroskademux1);
gst_element_link_many(vp8dec1, clrspace1, mpegenc, avimux, sink, nil);
}
