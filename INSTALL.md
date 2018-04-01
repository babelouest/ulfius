# Install Ulfius

## Debian-ish packages

[![Packaging status](https://repology.org/badge/vertical-allrepos/ulfius.svg)](https://repology.org/metapackage/ulfius)

Ulfius is now available in Debian Buster (testing) and some Debian based distributions. To install it on your device, use the following command as root:

```shell
# apt install libulfius-dev # Or apt install libulfius.1 if you don't need the development files
```

### Pre-compiled packages

You can install Ulfius with a pre-compiled package available in the [release pages](https://github.com/babelouest/ulfius/releases/latest/). `jansson`, `libmicrohttpd`, `gnutls` and `libcurl-gnutls` development files packages are required to install Ulfius. The packages files `ulfius-dev-full_*` contain the libraries `orcania`, `yder` and `ulfius`.

For example, to install Ulfius with the `ulfius-dev-full_2.3.0_Debian_stretch_x86_64.tar.gz` package downloaded on the `releases` page, you must execute the following commands:

```shell
$ sudo apt install -y libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev libgnutls28-dev libgcrypt20-dev
$ wget https://github.com/babelouest/ulfius/releases/download/v2.3.0/ulfius-dev-full_2.3.0_Debian_stretch_x86_64.tar.gz
$ tar xf ulfius-dev-full_2.3.0_Debian_stretch_x86_64.tar.gz
$ sudo dpkg -i liborcania-dev_1.2.0_Debian_stretch_x86_64.deb
$ sudo dpkg -i libyder-dev_1.2.0_Debian_stretch_x86_64.deb
$ sudo dpkg -i libulfius-dev_2.3.0_Debian_stretch_x86_64.deb
```

If there's no package available for your distribution, you can recompile it manually using `CMake` or `Makefile`.

## Manual install

Download Ulfius source code from Github, get the submodules, compile and install each submodule, then compile and install ulfius:

```shell
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
```

### Prerequisites

#### External dependencies

Ulfius requires the following dependencies

- libmicrohttpd (required), minimum 0.9.53 if you require Websockets support
- libjansson (optional), minimum 2.4, required for json support
- libgnutls, libgcrypt (optional), required for Websockets and https support
- libcurl (optional), required to send http/smtp requests

For example, to install all the external dependencies on Debian Stretch, run as root:

```shell
# apt-get install libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev libgnutls28-dev libgcrypt20-dev
```

### CMake - Multi architecture

You can build Ulfius library using cmake, example:

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make && sudo make install
```

The available options for cmake are:
- `-DWITH_JANSSON=[on|off]` (default `on`): Build with Jansson dependency
- `-DWITH_CURL=[on|off]` (default `on`): Build with libcurl dependency
- `-DWITH_WEBSOCKET=[on|off]` (default `on`): Build with websocket functions, not available for Windows, requires libmicrohttpd 0.9.53 minimum and GnuTLS installed.
- `-DBUILD_STATIC=[on|off]` (default `off`): Build the static archive in addition to the shared library
- `-DBUILD_TESTING=[on|off]` (default `off`): Build unit tests
- `-DINSTALL_HEADER=[on|off]` (default `on`): Install header file `ulfius.h`
- `-DCMAKE_BUILD_TYPE=[Debug|Release]` (default `Release`): Compile with debugging symbols or not

### Good ol' Makefile

#### Install Ulfius as a shared library

Ulfius can also be installed via the traditional Makefile

```shell
$ cd lib/orcania
$ make && sudo make install
$ cd ../yder
$ make && sudo make install
$ cd ../..
$ make
$ sudo make install
```

#### Disable dependencies

To disable libcurl functions, append the option `CURLFLAG=-DU_DISABLE_CURL` to the make command when you build Ulfius:

```shell
$ make CURLFLAG=-DU_DISABLE_CURL
```

If libcurl functions are disabled, `libcurl4-gnutls-dev` is no longer mandatory for install.

To disable libjansson functions, append the option `JANSSONFLAG=-DU_DISABLE_JANSSON` to the make command when you build Ulfius and Orcania:

```shell
$ make JANSSONFLAG=-DU_DISABLE_JANSSON
```

If libjansson functions are disabled, `libjansson-dev` is no longer mandatory for install.

To disable websocket implementation and avoid installing libgnutls, append the option `WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET` to the make command when you build Ulfius:

```shell
$ make WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET
```

If websocket functions are disabled, `libgnutls-dev` is no longer mandatory for install.

To disable two or more libraries, append options, example:

```shell
$ make CURLFLAG=-DU_DISABLE_CURL JANSSONFLAG=-DU_DISABLE_JANSSON
```

#### Installation directory

By default, the shared libraries and the header files will be installed in the `/usr/local` location. To change this setting, you can modify the `DESTDIR` value in the `src/Makefile`, `lib/orcania/src/Makefile` and `lib/yder/src/Makefile` files.

```shell
$ make DESTDIR=/tmp install # to install ulfius in /tmp/lib for example
```

You can install Ulfius without root permission if your user has write access to `$(DESTDIR)`.
A `ldconfig` command is executed at the end of the install, it will probably fail if you don't have root permission, but this is harmless.
If you choose to install Ulfius in another directory, you must set your environment variable `LD_LIBRARY_PATH` properly.

#### Install from a `.zip` archive

If you download Ulfius as a `.zip` or `.tar.gz` file via github release tab, you must initiaize the submodules prior to the compilaton with the following command:

```shell
$ cd ulfius/
$ git submodule update --init
```

#### Install as a static archive

To install Ulfius library as a static archive, `libulfius.a`, use the make commands `make static*`:

The archives `liborcania.a` and `libyder.a` must previously be installed in the $(DESTDIR) directory.

```shell
$ cd ulfius/src
$ make static && sudo make static-install # or make DESTDIR=/tmp static-install if you want to install in `/tmp/lib`
```

The example program `example_program/simple_example` has a static make command to test and validate building a program with the archive.

