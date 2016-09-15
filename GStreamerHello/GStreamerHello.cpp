// GStreamerHello.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <gst/gst.h>

static GMainLoop *loop;

static gboolean
my_bus_callback(GstBus     *bus,
	GstMessage *message,
	gpointer    data)
{
	g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME(message));

	switch (GST_MESSAGE_TYPE(message)) {
	case GST_MESSAGE_ERROR: {
		GError *err;
		gchar *debug;

		gst_message_parse_error(message, &err, &debug);
		g_print("Error: %s\n", err->message);
		g_error_free(err);
		g_free(debug);

		g_main_loop_quit(loop);
		break;
	}
	case GST_MESSAGE_EOS:
		/* end-of-stream */
		g_main_loop_quit(loop);
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

int main(int   argc,
	char *argv[])
{
	gboolean link_ok;
	GstElement *bin, *pipeline, *source, *sink, *filter;
	GstBus *bus;
	guint bus_watch_id;
	GstCaps *caps;


	caps = gst_caps_new_simple("video/x-raw",
		"width", G_TYPE_INT, 640,
		"height", G_TYPE_INT, 480,
		NULL);

	/* Initialize GStreamer */
	gst_init(&argc, &argv);

	/* Build the pipeline */
	pipeline = gst_pipeline_new("my_pipeline");

	/* adds a watch for new message on our pipeline's message bus to
	* the default GLib main context, which is the main context that our
	* GLib main loop is attached to below
	*/
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	bus_watch_id = gst_bus_add_watch(bus, my_bus_callback, NULL);
	gst_object_unref(bus);

	bin = gst_bin_new("my_bin");
	source = gst_element_factory_make("videotestsrc", "source");
	filter = gst_element_factory_make("capsfilter", "capsfilter");

	g_object_set(G_OBJECT(filter), "caps", caps, NULL);
	g_object_set(G_OBJECT(source), "pattern", 18, NULL);
	sink = gst_element_factory_make("fpsdisplaysink", "sink");
	
	/* First add the elements to the bin */
	gst_bin_add_many(GST_BIN(bin), source, filter, sink, NULL);
	/* add the bin to the pipeline */
	gst_bin_add(GST_BIN(pipeline), bin);

	/* link the elements */
	link_ok = gst_element_link(source, sink);
	gst_caps_unref(caps);

	if (!link_ok) {
		g_warning("Failed to link element1 and element2!");
	}

	/* Start playing */
	gst_element_set_state(pipeline, GST_STATE_PLAYING);
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	/* cleanup */
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	g_source_remove(bus_watch_id);
	g_main_loop_unref(loop);

	return 0;
}

