# netspray
========

## what
shittier netcat, but as a c library

given a host and port, can create a tcp connection and send/receive arbitrary data. good for spamming.

## todo
what will probably end up being a total rewrite. i don't see the api changing much but the more i program in different languages the more my style changes.

listening! you know, that other thing netcat can do.

macros for features so we can get the most unhealthily slim binaries if we only need a subset of this feature poor library.

consolidate options into flags so we can use a sexy monolithic switch statement in netspray()
to save a bit of memory and potentially make code a bit easier to read (maybe...), will also make setting the options easier.

udp? maybe; but definately lowest priority until i actually need it.
