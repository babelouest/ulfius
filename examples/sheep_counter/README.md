# sheep_example

Run a simple file webserver and a small REST API that listen on port 7437, and implement a file upload service.

## Compile and run

```bash
$ make test
```

## Usage:

### Sheep counter

Open in your browser the url [http://localhost:7437/index.html](http://localhost:7437/index.html), it's a jquery application that will count and display sheeps to help you fall asleep. There click on the buttons available to call the underline API.

The API endpoints are the following:

- `POST http://localhost:7437/sheep`: Initialise the counter with the specified `nbsheep` value provided in the json body (default 0) and return the new `nbsheep` value in the response as a json
- `PUT http://localhost:7437/sheep`: Add a sheep to the current counter and return the new `nbsheep` value in the response as a json
- `DELETE http://localhost:7437/sheep`: Reset the sheep counter to 0 and return the new `nbsheep` value in the response as a json

### File upload

Open in your browser the url [http://localhost:7437/upload.html](http://localhost:7437/upload.html), there upload a file, preferably not a big one, then click on the `Upload File` button.

The API endpoint is the following:

- `* http://localhost:7437/upload`: upload a file and show informations about it in the response.
