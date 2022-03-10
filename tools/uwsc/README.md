# uwsc - Ulfius WebSocket Client

Simple command-line application to connect to websocket services

Copyright 2018-2022 Nicolas Mora <mail@babelouest.org>

This program is free software; you can redistribute it and/or modify it under the terms of the GPL3 License.

## Overview

Can connect to websocket services, both ws:// (http) or wss:// (https). When the websocket is connected, messages from the server are displayed in the terminal and messages can be sent to the service via a prompt `> `.

To quit uwsc during connection, enter the message `!q`.

## Options

Options available:

```shell
-o --output-log-file=PATH
	Print yder log messages in a file
-x --add-header=HEADER
	Add the specified header of the form 'key:value'
-b --send-binary-file=PATH
	Sends the content of a file at the beginning of the connection in a binary frame
-t --send-text-file=PATH
	Sends the content of a file at the beginning of the connection in a text frame
-i --non-interactive-send
	Do not prompt for message sending
-l --non-listening
	Do not listen for incoming messages
-f --fragmentation=
	Specify the max length of a frame and fragment the message to send if required, 0 means no fragmentation, default 0
-p --protocol=PROTOCOL
	Specify the Websocket Protocol values, default none
-e --extensions=EXTENSION
	Specify the Websocket extensions values, default none
-s --non-secure
	Do not check server certificate
-s --non-secure
	Do not check server certificate
-q --quiet
	Quiet mode, show only websocket messages
-v --version
	Print Glewlwyd's current version

-h --help
	Print this message
```

## Examples

Here are some examples on how to use uwsc options

```shell
$ # run uwsc to a websocket service
$ uwsc http://localhost:9275/websocket
$ # run uwsc to a secured websocket service
$ uwsc https://localhost:9275/websocket
$ # run uwsc with a specified protocol and extension list
$ uwsc --protocol=myProtocol --extensions=ext1;ext2
$ # run uwsc to a websocket service with additional header values
$ uwsc --add-header="Authorization: Bearer abcd.efgh.xyz" --add-header="pragma: no-cache" http://localhost:9275/websocket
$ # run uwsc to send a text file content to the service as first message, then the prompt will be available
$ uwsc --send-text-file=/path/to/file http://localhost:9275/websocket
$ # run uwsc without prompt
$ uwsc -i http://localhost:9275/websocket
$ # all messages will be fragmented with the maximum payload size specified
$ uwsc --fragmentation=42 http://localhost:9275/websocket
```
