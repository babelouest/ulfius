# Injection example

Provides an example program that will add and remove endpoints during the execution

## Compile and run

```bash
$ make test
```

## Usage

Right after starting the program, the endpoints available are the following:

- `GET http://localhost:4528/inject/first`
- `GET http://localhost:4528/inject/second`
- `GET http://localhost:4528/inject/third`

On the program console, press `<enter>` one time will add this new endpoint:
- `GET http://localhost:4528/inject/fourth`

Press `<enter>` again to add the following endpoint:
- `GET http://localhost:4528/inject/fifth`

Press `<enter>` to remove the following endpoint:
- `GET http://localhost:4528/inject/fourth`

Then press `<enter>` to quit the application

You can test the presence or absence of the specified endpoints between each step.
