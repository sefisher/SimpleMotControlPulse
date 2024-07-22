# HS_AsyncTCP 

This is a modification of the AsyncTCP library to run the callbacks on the WiFi core and to limit connections to one only and to add a doLoop() call back which is called more often then the default 500ms interval of the doPoll() call back.

See [High Speed ESP32](https://www.forward.com.au/pfod/ESP32/HighSpeedCtrl/index.html) for the details and usage.

# AsyncTCP 
[![Build Status](https://travis-ci.org/me-no-dev/AsyncTCP.svg?branch=master)](https://travis-ci.org/me-no-dev/AsyncTCP) ![](https://github.com/me-no-dev/AsyncTCP/workflows/Async%20TCP%20CI/badge.svg) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/2f7e4d1df8b446d192cbfec6dc174d2d)](https://www.codacy.com/manual/me-no-dev/AsyncTCP?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=me-no-dev/AsyncTCP&amp;utm_campaign=Badge_Grade)

### Async TCP Library for ESP32 Arduino

[![Join the chat at https://gitter.im/me-no-dev/ESPAsyncWebServer](https://badges.gitter.im/me-no-dev/ESPAsyncWebServer.svg)](https://gitter.im/me-no-dev/ESPAsyncWebServer?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

This is a fully asynchronous TCP library, aimed at enabling trouble-free, multi-connection network environment for Espressif's ESP32 MCUs.

This library is the base for [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

## AsyncClient and AsyncServer
The base classes on which everything else is built. They expose all possible scenarios, but are really raw and require more skills to use.
