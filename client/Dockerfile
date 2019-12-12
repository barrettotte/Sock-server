FROM alpine:3.10

WORKDIR /usr/src/skvdb

COPY . .

RUN apk add --no-cache gcc musl-dev make

#  docker build -t skvdb-client .
#  docker run --rm -it skvdb-client
