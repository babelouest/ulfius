# Install Ulfius

## Prerequisites

### External dependencies

To install the external dependencies, for Debian based distributions (Debian, Ubuntu, Raspbian, etc.), run as root:

```shell
# apt-get install libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev
```

### Note

I write libcurl4-gnutls-dev for the example, but any `libcurl*-dev` library should be sufficent, depending on your needs and configuration.

## Installation

Download Ulfius source code from Github, get the submodules, compile and install:

```shell
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
$ make
$ sudo make install
```

By default, the shared libraries and the header files will be installed in the `/usr/local` location. To change this setting, you can modify the `PREFIX` value in the `src/Makefile`, `lib/orcania/Makefile` and `lib/yder/src/Makefile` files.

If you download Ulfius as a `.zip` or `.tar.gz` file via github release tab, you must initiaize the submodules prior to the compilaton with the following command:

```shell
$ cd ulfius/
$ git submodule update --init
```

### libcurl older than 7.20

If you use old version of liburl, as in Centos 6.5 for example, `ulfius_send_smtp_email` won't be available due to libcurl options problems. In this case, uncomment the following line in the file `src/Makefile`:

```Makefile
#SMTPFLAGS=-DULFIUS_IGNORE_SMTP
```

This will disable `ulfius_send_smtp_email` during the compilation.
