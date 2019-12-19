# Sock-KVDB

A simple non-persistent key value database to learn more about sockets and multithreading.

I eventually want to come back after a couple other side projects and improve upon this.
This includes better error trapping, persistence, user handling, and more key/value data (locks, more types, etc).
But, for now this was good enough for me to practice my new knowledge.

## Server
My server implementation contains a thread pool and a non-persistent key value database.
The server accepts new client connections and sends their required work to the thread pool to be processed.


## Client
The client is super basic and only supports a few commands to manage the key value database, I listed them below.
The username is decided by the client application, so there is no security around anything for now. 


## Client Commands
* ```:d 1``` - Delete element with key of 1
* ```:g 1``` - Get value for element with key of 1 from database
* ```:i``` - Get info about server
* ```:q``` - Disconnect from server
* ```:s 1 "Hello World"``` - Set value for element with key of 1 to "Hello World"


## Commands
* server: ```./server```
* client: ```./client <user>```


## References
* https://beej.us/guide/bgnet/html/
* https://en.wikipedia.org/wiki/Key-value_database
* http://cs241.cs.illinois.edu/coursebook/Threads

