#include <string.h>
#include <math.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gst/gst.h>

typedef struct _HttpSrc HttpSrc;
struct _HttpSrc {
	GstElement *pipe;
	GstElement *src;
	GstElement *queue1;
	GstElement *queue2;
	GstElement *decoder;
	GstElement *sink;
	GstBus *bus;
};

HttpSrc src;
GMainLoop *loop;

#define URL_X3M "http://stream2.yle.mobi:8000/livex3m256.mp3"

static void
on_pad_added_cb(GstElement *element, GstPad *pad, gpointer data)
{
GstElement *e=GST_ELEMENT(data);
GstPad *sinkpad;

sinkpad=gst_element_get_pad(e, "sink");
gst_pad_link(pad, sinkpad);
gst_object_unref(sinkpad);
}

static void
on_decoder_pad_added_cb (GstElement *decodebin, GstPad *pad, gboolean last, gpointer data)
{
GstElement *e=GST_ELEMENT(data);
GstPad *sinkpad;

sinkpad=gst_element_get_pad(e, "sink");
if (GST_PAD_IS_LINKED(sinkpad)) {
	g_object_unref(sinkpad);
	return;
}
gst_pad_link(pad, sinkpad);
gst_object_unref(sinkpad);
}

static void
display_tag(const GstTagList *list, const gchar *tag, gpointer data)
{
gchar *v;
guint i;
gboolean b;
GType t;

t=gst_tag_get_type(tag);
switch (t) {
        case G_TYPE_STRING:
        if (gst_tag_list_get_string(list, tag, &v))
                g_debug("Tag: %s = %s", tag, v);
        break;
        case G_TYPE_BOOLEAN:
        if (gst_tag_list_get_boolean(list, tag, &b))
        	g_debug("Tag: %s = %d", tag, b);
        break;
        case G_TYPE_UINT:
        if (gst_tag_list_get_uint(list, tag, &i))
	        g_debug("Tag: %s = %d", tag, i);
        break;
        default:
                g_debug("Tag-type: %d: %s", (guint)t, g_type_name(t));
        break;
}

}

static gboolean
m3x_bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
gchar *debug;
GstState newstate;
GstState oldstate;
GstState pending;
GError *err=NULL;
gint buffp;
gint ain, aout;
gint64 bleft;
GstBufferingMode bmode;

g_debug("\n---\nBUS CALL FROM: '%s' TYPE '%s'", GST_MESSAGE_SRC_NAME(msg), gst_message_type_get_name(GST_MESSAGE_TYPE(msg)));

switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_EOS:
		g_debug("EOS: %s, NULLIFY pipeline", GST_MESSAGE_SRC_NAME(msg));
		gst_element_set_state(src.pipe, GST_STATE_NULL);
		g_debug("NULLED");
	break;
	case GST_MESSAGE_ERROR:
		gst_message_parse_error(msg, &err, &debug);
		g_debug("ERROR: %s", err->message);
		g_free(debug);
		g_error_free(err);
		gst_element_set_state(src.pipe, GST_STATE_NULL);
		g_debug("NULLED");
	break;
	case GST_MESSAGE_WARNING:
		gst_message_parse_warning(msg, &err, &debug);
		g_debug("WARNING: %s", err->message);
		g_free(debug);
		g_error_free(err);
		/* gst_element_set_state(src.pipe, GST_STATE_NULL); */
	break;
	case GST_MESSAGE_STATE_CHANGED:
		gst_message_parse_state_changed(msg, &oldstate, &newstate, &pending);

#if 0
		if (GST_MESSAGE_SRC(msg)!=GST_OBJECT(p->ge.pipeline))
			return TRUE;
#endif

		g_debug("GST: %s state changed (o=%d->n=%d => p=%d)", GST_MESSAGE_SRC_NAME(msg), oldstate, newstate, pending);
	break;
	case GST_MESSAGE_BUFFERING:
		gst_message_parse_buffering(msg, &buffp);
		gst_message_parse_buffering_stats(msg, &bmode, &ain, &aout, &bleft);
		g_debug("BUFFERING: %d%% (M: %d %d/%d)", buffp, bmode, ain, aout);
	break;
	case GST_MESSAGE_TAG: {
		GstTagList *tags=NULL;

		gst_message_parse_tag (msg, &tags);
		g_print ("Got tags from element %s\n", GST_OBJECT_NAME (msg->src));
		gst_tag_list_foreach(tags, (GstTagForeachFunc)display_tag, NULL);
		gst_tag_list_free (tags);
	}
	break;
	default:
		g_debug("GST: From %s -> %s", GST_MESSAGE_SRC_NAME(msg), gst_message_type_get_name(GST_MESSAGE_TYPE(msg)));
	break;
	}
return TRUE;
}

/**
 * Create pipeline, currently:
 * httpsrc -|> queue2(q1) -> decodebin2 -|> queue2 > alsasink
 */
static gboolean
http_src()
{
src.pipe=gst_pipeline_new("pipeline");

src.queue1=gst_element_factory_make("queue2", "srcq1");
g_object_set(src.queue1, "use-buffering", TRUE, "low-percent", 50, NULL);

src.queue2=gst_element_factory_make("queue2", "sinkq2");
g_object_set(src.queue2, "use-buffering", TRUE, "low-percent", 20, NULL);

src.decoder=gst_element_factory_make("decodebin2", "decoder");
g_signal_connect(src.decoder, "new-decoded-pad", G_CALLBACK(on_decoder_pad_added_cb), src.queue2);
g_object_set(src.decoder, "use-buffering", TRUE, NULL);

src.src=gst_element_factory_make("souphttpsrc", "http");
g_object_set(src.src,"location", URL_X3M, NULL);

src.sink=gst_element_factory_make("alsasink", "sink");

gst_bin_add_many(GST_BIN(src.pipe), src.src, src.queue1, src.queue2, src.decoder, src.sink, NULL);

g_assert(gst_element_link(src.src, src.queue1));
/* g_assert(gst_element_link(src.decoder, src.queue)); */
g_assert(gst_element_link(src.queue1, src.decoder));
g_assert(gst_element_link(src.queue2, src.sink));

src.bus=gst_pipeline_get_bus(GST_PIPELINE(src.pipe));
gst_bus_add_watch(src.bus, m3x_bus_call, NULL);

return TRUE;
}

static gboolean
start_play(gpointer data)
{
gst_element_set_state(src.pipe, GST_STATE_PLAYING);

return FALSE;
}

gint
main(gint argc, gchar **argv)
{
g_type_init();
gst_init(&argc, &argv);

http_src();

g_idle_add(start_play, NULL);

loop=g_main_loop_new(NULL, TRUE);
g_main_loop_run(loop);

gst_element_set_state(src.pipe, GST_STATE_NULL);
gst_object_unref(GST_OBJECT(src.pipe));

return 0;
}
