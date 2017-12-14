# riemann-c-client

Native C client and library for [Riemann] licensed with Apache License 2.0.

[Riemann]: http://riemann.io
 
Chekout the [wiki]!
[wiki]: https://github.com/gkos/riemann-c-client/wiki

You may want to take a look at [Algenon's riemann-c-client] it supports
everthing that this version supports plus TLS. Also it has a simpler API.
I'm still willing to merge pull requests if anyone has interest.
[Algenon's riemann-c-client]: https://github.com/algernon/riemann-c-client 

## Notes

- 2016-10-02: I'm re-releasing this as Apache License 2.0.

## Features

- TCP and UDP support for sending events
- TCP support for quering
- Timeout support, enabling blocking and non-blocking calls on same
  function. (Can be disabled at compile time
  --disable-timeout. Enabled by default.)
- Support for tags and attributes
- Support for sending multiple events in one shot

## Dependencies

- Google [protobuf-c] library
- libtoolize

On OS X, this can be installed with:

    brew protobuf-c libtool

Use `--with-protobuf-c-dir=/usr/local/Cellar/protobuf-c/0.15`
as arguments to `./configure` below.

[protobuf-c]: http://code.google.com/p/protobuf-c/

## Install

WARNING: This is in really early stages, and was not fully tested.

1. Clone

```
# git clone 'https://github.com/gkos/riemann-c-client.git'
# cd riemann-c-client
```

2. Build

```
# ./build.sh
# ./configure --with-protobuf-c-dir=/usr/include/protobuf-c
# make
```

3. Install

```
# sudo make install
```

Checkout the [wiki] for distro specific instructions
[wiki]: https://github.com/gkos/riemann-c-client/wiki   

## Usage

Include the headers
```C
#include <riemann/event.h>
#include <riemann/message.h>
#include <riemann/client.h>
```

Create messages. Queries and Events are send attached in messages.
Also create a client, this will be your connection to riemann
server. riemann_client_connect() returns non-zero on failure.

```C
int error;
riemann_message_t msg = RIEMANN_MSG_INIT;
riemann_client_t cli;
```

If you'll send events create the events and alloc them. There no
simpler way to send a sigle event. Check the returned pointers. NULL
means that malloc failed. Then set the event fields.

```C
riemann_event_t **events;
events = riemann_event_alloc_events(1);
events[0] = riemann_event_alloc_event();
riemann_event_set_host(events[0], "batcave");
riemann_event_set_service(events[0], "batservice");
riemann_event_set_metric_f(events[0], 1);
riemann_event_set_state(events[0], "ok");
```

Attach events to message:

```C
riemann_message_set_events(&msg, events, 1);
```

Send the message! 3th parameter is flags to be passed to send() call,
4th parameter is a timeout struct(struct timeval). Passing 0 on both
fields yield a non-blocking call, otherwise the call waits until
timeout. If NULL is passed the call blocks undefinately.

```C
error = riemann_client_init(&cli);
error = riemann_client_connect(&cli, RIEMANN_TCP,
                               "riemann.batmansion.org, 
                               5555);
error = riemann_client_send_message(&cli, &msg, 0, NULL);
```

If you want to make sure that server received your events, ask it for
a response:

```C
riemann_message_t *resp = NULL;
resp = riemann_client_recv_message(&cli, 0, NULL);
if (resp->ok)
   puts("SUCCESS"); /* your message was received with success */
```

Responses need to be freed. Also events and clients.

```C
riemann_message_free(resp);
riemann_events_free(events, n_events);
riemann_client_free(&cli);
```

When compiling your code, link it against libriemann_client:

```
# cc -o your_program your_code.c -lriemann_c_client -L/usr/local/lib -I/usr/local/include
```


For complete examples look at src/examples directory.
