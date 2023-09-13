# About The Project

czlyrics is a microservice application for retrieving the lyrics for a given song.

This project is being actively worked on, so stay tuned for more updates!

### Built With
* C
* Docker

# Getting Started

There are a few ways to start using this application, via Docker or by compiling and running the code locally.

### Build with Docker

In the main directory, you can find a `run.sh` script that you can use for running the container build and deployment process. If you aren't on a Linux machine, you can use the following commands to get the app up and running:

Build the Docker image:

```sh
docker build -t czlyrics .
```

Run a new container using the image:

```sh
docker run -d -p 8080:8080 czlyrics
```

Remember to look up the auto-generated container name and stop the container once you're done.

### Compile Locally

The application has a few dependencies, those being:

* cesanta-mongoose
* mjson
* OpenSSL

Getting an environment setup locally can be an involving process, but if you wanted to get started the Dockerfile is a great resource. I'll be updating this section soon with more information.

### Using the API

The API requires that requests be formatted as such:

```sh
http://localhost:8080/api/lyrics/{artist}/{song}/
```

It's important that the trailing forward slash is included, as it tells the parser where to stop.

The response message will be a JSON object with the following structure:

```json
{
  "error":
  {
    "code": 200,
    "message": "Success"
  },
  "data":
  {
    "artist": "artist name",
    "song": "song title",
    "lyrics": "song lyrics"
  }
}
```

# License

Distributed under the MIT LIcense. See `LICENSE` for more information.

