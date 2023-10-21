# Micro HTTP Client

A generic, lightweight HTTP 1.1 client. Not fully featured yet.

Compatible Linux and Raspberry Pico W via the LWIP library.

* Fully developed in C
* no complex features, no chunking, no ssl, no keepalive ...
* not very tested, use at your own risk!
* it PURPOSELY does not use any feature-complete libraries (like cURL). It only use raw Sockets.
* easy cmake integration

This library has been developed with Pico W in mind (sdk do not provide http client yet...). 
Linux socket has been implemented in order to debug without pushing the code to the Pico.

In bonus you can find a Dockerfile with the whole stack necessary to compile your code for PicoW.
You can avoid installing another toolchain on your machine.
