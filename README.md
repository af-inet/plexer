# plexer (WIP)

`plexer` is a lil C http server created for fun :)

The target platform is **Linux** only; as it uses the `epoll` syscall for multiplexing sockets, sorry other *nixs!

## build / run

```
clone https://github.com/DavidHargat/plexer

cd plexer

make

./plexer
```

## why

There are a lot of interesting problems to solve in building a web server, http parsing, sockets, concurrency, caching, etc.
Building your own server from scratch means you get to tackle those problems yourself, and learn in the process.
