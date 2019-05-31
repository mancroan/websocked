#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#ifdef _MSC_VER

#else
#include <unistd.h>
#include <signal.h>
#endif

static int destroy_flag = 0;

#ifndef _MSC_VER
	static void INT_HANDLER(int signo) {
		destroy_flag = 1;
	}
#endif

/* *
 * websocket_write_back: write the string data to the destination wsi.
 */
int websocket_write_back(struct lws *wsi_in, char *str, int str_size_in) 
{
    if (str == NULL || wsi_in == NULL)
        return -1;
 
    int n;
    int len;
    unsigned char *out = NULL;
 
    if (str_size_in < 1) 
        len = strlen(str);
    else
        len = str_size_in;
 
    out = (unsigned char *)malloc(sizeof(unsigned char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));
    //* setup the buffer*/
    memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );
    //* write out*/
    n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);
    //* free the buffer*/
    free(out);
 
    return n;
}

static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	switch( reason )
	{
		case LWS_CALLBACK_HTTP:
			lws_serve_http_file( wsi, "example.html", "text/html", NULL, 0 );
			break;
		default:
			break;
	}

	return 0;
}

#define EXAMPLE_RX_BUFFER_BYTES (1024)

static int callback_example( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	switch( reason )
	{
		case LWS_CALLBACK_ESTABLISHED:
			break;

		case LWS_CALLBACK_RECEIVE:
			//* echo back to client*/
			websocket_write_back(wsi ,(char *)in, -1);
			break;

		case LWS_CALLBACK_CLOSED:
			break;

		default:
			break;
	}

	return 0;
}

enum protocols
{
	PROTOCOL_HTTP = 0,
	PROTOCOL_EXAMPLE,
	PROTOCOL_COUNT
};

static struct lws_protocols protocols[] =
{
	/* The first protocol must always be the HTTP handler */
	{
		"http-only",   /* name */
		callback_http, /* callback */
		0,             /* No per session data. */
		0,             /* max frame size / rx buffer */
	},
	{
		"example-protocol",
		callback_example,
		0,
		EXAMPLE_RX_BUFFER_BYTES,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

int main( int argc, char *argv[] )
{

#ifndef _MSC_VER
	//* register the signal SIGINT handler */
	struct sigaction act;
	act.sa_handler = INT_HANDLER;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction( SIGINT, &act, 0);
#endif

	struct lws_context_creation_info info;
	memset( &info, 0, sizeof(info) );

	info.port = 8000;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	struct lws_context *context = lws_create_context( &info );

	while( !destroy_flag )
	{
		lws_service( context, /* timeout_ms = */ 1000 );
	}

	lws_context_destroy( context );

	return 0;
}
