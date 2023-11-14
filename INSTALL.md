# Install Ulfius

- [Distribution packages](#distribution-packages)
- [Manual install](#manual-install)
  - [Prerequisites](#prerequisites)
  - [CMake - Multi architecture](#cmake---multi-architecture)
  - [Good ol' Makefile](#good-ol-makefile)

## Distribution packages

[![Packaging status](https://repology.org/badge/vertical-allrepos/ulfius.svg)](https://repology.org/metapackage/ulfius)

Ulfius is available in multiple distributions as official package. Check out your distribution documentation to install the package automatically.

```shell
$ # Example for Debian testing, install ulfius library with development files and uwsc
$ sudo apt install libulfius-dev uwsc
$ # Example for Debian testing, install uwsc only
$ sudo apt install uwsc
```

## Manual install

### Prerequisites

Ulfius requires the following dependencies.

- libmicrohttpd (required), minimum 0.9.53 if you require Websockets support
- libjansson (optional), minimum 2.4, required for json support
- libgnutls, libgcrypt (optional), required for Websockets and https support
- libcurl (optional), required to send http/smtp requests
- libsystemd (optional), required for [Yder](https://github.com/babelouest/yder) to log messages in journald
- zlib (optional), required for Websockets support

Note: the build stacks require a compiler (`gcc` or `clang`), `make`, `cmake` (if using CMake build), and `pkg-config`.

For example, to install all the external dependencies on Debian Stretch, run as root:

```shell
# apt install -y libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev libgnutls28-dev libgcrypt20-dev zlib1g-dev
```

### Good ol' Makefile

Download Orcania, Yder and Ulfius source code from GitHub, compile and install Orcania, then Yder, then compile and install Ulfius:

Last Orcania release: [https://github.com/babelouest/orcania/releases/latest/](https://github.com/babelouest/orcania/releases/latest/)
Last Yder release: [https://github.com/babelouest/yder/releases/latest/](t/](https://github.com/babelouest/orcania/releases/latest/)
Last Ulfius release: [https://github.com/babelouest/ulfius/releases/latest/](https://github.com/babelouest/ulfius/releases/latest/)

**Note: Make sure all the previous installed version of those libraries, via a package manager for example, are removed.**

```shell
$ cd <orcania_source>
$ make && sudo make install
$ cd <yder_source>
$ make && sudo make install
$ cd <ulfius_source>
$ make && sudo make install
```

#### Disable Ulfius dependencies

To disable libcurl functions, append the option `CURLFLAG=1` to the make command when you build Ulfius:

```shell
$ make CURLFLAG=1
```

If libcurl functions are disabled, `libcurl4-gnutls-dev` is no longer mandatory for install.

To disable libjansson functions, append the option `JANSSONFLAG=1` to the make command when you build Ulfius and Orcania:

```shell
$ make JANSSONFLAG=1
```

If libjansson functions are disabled, `libjansson-dev` is no longer mandatory for install.

To disable GNU TLS extensions (HTTPS client certificate support) and avoid installing libgnutls, append the option `GNUTLSFLAG=1` to the make command when you build Ulfius:

```shell
$ make GNUTLSFLAG=1
```

If GNU TLS extensions are disabled, `libgnutls-dev` is no longer mandatory for install. However, this will also disable websocket support, since it depends on libgnutls.

To disable websocket implementation, append the option `WEBSOCKETFLAG=1` to the make command when you build Ulfius:

```shell
$ make WEBSOCKETFLAG=1
```

To disable Yder library (you will no longer have log messages available!), append the option `YDERFLAG=1` to the make command when you build Ulfius:

```shell
$ make YDERFLAG=1
```

To disable uwsc build, append the option `UWSCFLAG=1` to the make command when you build Ulfius:

```shell
$ make UWSCFLAG=1
```

To disable two or more libraries, append options, example:

```shell
$ make CURLFLAG=1 JANSSONFLAG=1
```

#### Install Ulfius as a static archive

Install  Ulfius as a static archive, `libulfius.a`, use the make commands `make static*`:

```shell
$ cd src
$ make static && sudo make static-install # or make DESTDIR=/tmp static-install if you want to install in `/tmp/lib`
```

#### Enable embedded systems flags

To build Ulfius on [FreeRTOS](https://www.freertos.org/), append the option `FREERTOSFLAG=1` to the make command when you build Ulfius:

```shell
$ make FREERTOSFLAG=1
```

To build Ulfius with the [LWIP](https://savannah.nongnu.org/projects/lwip/) library, append the option `LWIPFLAG=1` to the make command when you build Ulfius:

```shell
$ make LWIPFLAG=1
```

Those two options are exclusive, you can't use them both at the same time.

#### Installation directory

By default, the shared libraries and the header files will be installed in the `/usr/local` location. To change this setting, you can modify the `DESTDIR` value in the `src/Makefile`, `lib/orcania/src/Makefile` and `lib/yder/src/Makefile` files.

```shell
$ make DESTDIR=/tmp install # to install ulfius in /tmp/lib for example
```

You can install Ulfius without root permission if your user has write access to `$(DESTDIR)`.
A `ldconfig` command is executed at the end of the install, it will probably fail if you don't have root permission, but this is harmless.
If you choose to install Ulfius in another directory, you must set your environment variable `LD_LIBRARY_PATH` properly.

#### Install uwsc

uwsc is a small command-line tool to connect to websocket services. To install uwsc only, you can use the `Makefile` in the `uwsc/` directory:

```shell
$ cd ulfius/uwsc
$ make && sudo make install
```

This will compile and install uwsc in `/usr/local/bin`, to install it in another directory, you can change the value of `DESTDIR`.

### CMake - Multi architecture

You can build Ulfius library using CMake, example:

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make && sudo make install
```

The available options for CMake are:
- `-DWITH_JANSSON=[on|off]` (default `on`): Build with Jansson dependency
- `-DWITH_CURL=[on|off]` (default `on`): Build with libcurl dependency
- `-DWITH_GNUTLS=[on|off]` (default `on`): Build with GNU TLS extensions (HTTPS client certificate support), requires GnuTLS library.
- `-DWITH_WEBSOCKET=[on|off]` (default `on`): Build with websocket functions, not available for Windows, requires libmicrohttpd 0.9.53 minimum.
- `-DWITH_JOURNALD=[on|off]` (default `on`): Build with journald (SystemD) support for logging
- `-DWITH_YDER=[on|off]` (default `on`): Build with Yder library for logging messages
- `-DBUILD_UWSC=[on|off]` (default `on`): Build uwsc
- `-DBUILD_STATIC=[on|off]` (default `off`): Build the static archive in addition to the shared library
- `-DBUILD_ULFIUS_TESTING=[on|off]` (default `off`): Build unit tests
- `-DBUILD_ULFIUS_DOCUMENTATION=[on|off]` (default `off`): Build the documentation when running `make doc`, doxygen is required
- `-DINSTALL_HEADER=[on|off]` (default `on`): Install header file `ulfius.h`
- `-DBUILD_TGZ=[on|off]` (default `off`): Build tar.gz package when running `make package`
- `-DBUILD_DEB=[on|off]` (default `off`): Build DEB package when running `make package`
- `-DBUILD_RPM=[on|off]` (default `off`): Build RPM package when running `make package`
- `-DCMAKE_BUILD_TYPE=[Debug|Release]` (default `Release`): Compile with debugging symbols or not
