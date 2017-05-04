# Install Ulfius

## Prerequisites

### External dependencies

To install the external dependencies, for Debian based distributions (Debian, Ubuntu, Raspbian, etc.), run as root:

```shell
# apt-get install libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev libssl-dev
```

### Note

Here libcurl4-gnutls-dev for the example, but any `libcurl*-dev` library should be sufficent, depending on your needs and configuration.

Also, it seems that Debian Wheezy uses an old version of libjansson (2.3), you can either upgrade to jessie or download the latest version of libjansson from [github](https://github.com/akheron/jansson).

As in version 2.0, libcurl and libjansson are no longer mandatory if you don't need one or both.

## Installation

Download Ulfius source code from Github, get the submodules, compile and install:

```shell
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
$ make
$ sudo make install
```

To disable libcurl functions, append the option `CURLFLAG=-DU_DISABLE_CURL` to the make command:

```shell
$ make CURLFLAG=-DU_DISABLE_CURL
```

To disable libjansson functions, append the option `JANSSONFLAG=-DU_DISABLE_JANSSON` to the make command:

```shell
$ make JANSSONFLAG=-DU_DISABLE_JANSSON
```

To disable websocket implementation and avoid installing libssl, append the option `WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET` to the make command:

```shell
$ make WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET
```

To disable two or more libraries, append options:

```shell
$ make CURLFLAG=-DU_DISABLE_CURL JANSSONFLAG=-DU_DISABLE_JANSSON
```

By default, the shared libraries and the header files will be installed in the `/usr/local` location. To change this setting, you can modify the `PREFIX` value in the `src/Makefile`, `lib/orcania/Makefile` and `lib/yder/src/Makefile` files.

If you download Ulfius as a `.zip` or `.tar.gz` file via github release tab, you must initiaize the submodules prior to the compilaton with the following command:

```shell
$ cd ulfius/
$ git submodule update --init
```
