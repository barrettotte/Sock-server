FROM alpine:3.10

WORKDIR /usr/src/server

COPY . .

RUN apk add --no-cache gcc musl-dev make

EXPOSE 5024

CMD make server && ./server 5024

#  docker build -t skvdb-server . 
#  docker run -d --rm skvdb-server
#  -it 
