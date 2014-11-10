#netspray
========
##what
shittier netcat, but as a c library

##todo
better read capabality aka recv() while stream==1. i need to research how naive i can get,
but i might be able to just sping off a thread for each connection with an infite blocking recv() loop.
afaik this will work as recv() will scream bloody murder if the connection ever dies (and thus thread will exit)
but will happily read 5eva until then.

lisening! you know, that other thing netcat can do.

macros for features so we can get the most unhealthily slim binaries if we only need a subset of this feature poor library.

consolodate options into flags so we can use a sexy monolithic switch statement in netspray()
to save a bit of memory and potentially make code a bit easier to read (maybe...)

udp? probably but definately lowest priority until i actually need it
