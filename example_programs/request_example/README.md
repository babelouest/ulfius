# HTTP Request send examples

Provides some basic http request send examples.

## Compile and run

```bash
$ make
$ ./server
```

On another console, run the following command

```bash
$ ./client
```

Then follow the instructions and observe the results.

## Mail send example

Modify the file `mail.c` values for `MAIL_SERVER`, `MAIL_SENDER` and `MAIL_RECIPIENT` with your own, then compile and run:

```bash
$ ./mail
```

An email should be sent to the `MAIL_RECIPIENT` address.
