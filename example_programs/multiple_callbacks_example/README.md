# multiple_callbacks_example

Run a webservice with multiple endpoint callbacks possible and different levels of priority

## Compile and run

```bash
$ make test
```

## Endpoints available:

### Multiple callbacks in cascade

- `GET http://localhost:6875/multiple/zero`: Send "Level zero" response (status 200)
- `GET http://localhost:6875/multiple/zero/one`: Send "Level zero\nlevel one" response (status 200)
- `GET http://localhost:6875/multiple/zero/one/two`: Send "Level zero\nlevel one\nlevel two" response (status 200)
- `GET http://localhost:6875/multiple/zero/one/two/three`: Send "Level zero\nlevel one\nlevel two\nlevel three" response (status 200)

### Multiple callbacks but the cascade is stopped at `/zero/onec/`

- `GET http://localhost:6875/multiple/zero/one`: Send "Level zero\nlevel one" response (status 200)
- `GET http://localhost:6875/multiple/zero/one/two`: Send "Level zero\nlevel one" response (status 200)

### Hello World with authentication

username: `test`
password: `testpassword`

- `GET http://localhost:6875/multiple/auth/data`: Send "Hello World!" response if authentication is correct (status 200), otherwise status 401 with realm "default_realm"
- `PUT http://localhost:6875/multiple/auth/data`: Send "Hello World!" response if authentication is correct (status 200), otherwise status 401 with realm "specific_realm"
