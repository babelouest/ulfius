# Install Ulfius

## Debian/Ubuntu package

Ulfius is now available in Debian Buster (testing), Debian sid and since Ubuntu 17.10 "The Artful Aardvark". To install it on your device, use the following command as root:

```shell
# apt install libulfius-dev # Or apt install libulfius.1 if you don't need the development files
```

## Manual install

### Prerequisites

#### External dependencies

To install all the external dependencies, for Debian based distributions (Debian, Ubuntu, Raspbian, etc.), run as root:

```shell
# apt-get install libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev libgnutls28-dev libgcrypt20-dev
```

#### Note

Here libcurl4-gnutls-dev for the example, but any `libcurl*-dev` library should be sufficent, depending on your needs and configuration. Note that gnutls is mandatory for websocket management and https support.

Also, it seems that Debian Wheezy uses an old version of libjansson (2.3), you can either upgrade to jessie or download the latest version of libjansson from [github](https://github.com/akheron/jansson).

As in version 2.0, libcurl and libjansson are no longer mandatory if you don't need one or both.

If you want to use the websocket server functions, you need to install libmicrohttpd 0.9.53 minimum version.

### Installation

#### Install Ulfius as a shared library

Download Ulfius source code from Github, get the submodules, compile and install each submodule, then compile and install ulfius:

```shell
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
$ cd lib/orcania
$ make && sudo make install
$ cd ../yder
$ make && sudo make install
$ cd ../..
$ make
$ sudo make install
```

#### Update Ulfius

If you update Ulfius from a previous version, you must install the corresponding version of the submodules as well:

```shell
$ cd ulfius/
$ git pull # Or git checkout <the version you need>
$ git submodule update
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

By default, the shared libraries and the header files will be installed in the `/usr/local` location. To change this setting, you can modify the `PREFIX` value in the `src/Makefile`, `lib/orcania/src/Makefile` and `lib/yder/src/Makefile` files.

```shell
$ make PREFIX=/tmp install # to install ulfius in /tmp/lib for example
```

You can install Ulfius without root permission if your user has write access to `$(PREFIX)`.
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

The archives `liborcania.a` and `libyder.a` must previously be installed in the $(PREFIX) directory.

```shell
$ cd ulfius/src
$ make static && sudo make static-install # or make PREFIX=/tmp static-install if you want to install in `/tmp/lib`
```

The example program `example_program/simple_example` has a static make command to test and validate building a program with the archive.

