# BRu Command Deck v0.1
Custom firmware for M5Stack Core Basic (original ESP32, 320x240 LCD, three buttons).

## Setup
1. Install VS Code and the PlatformIO IDE extension.
2. Open this folder in VS Code.
3. Copy `include/secrets.example.h` to `include/secrets.h` and set your Wi-Fi and Psydekik URL.
4. Connect the Core Basic via a USB **data** cable.
5. In PlatformIO, run Build then Upload. Open Serial Monitor at 115200 if needed.

## Navigation
A/C navigate, B opens, B/C return from a page. A on a page forces a health check.

## Networking
Psydekik API must listen on an address reachable from the ESP32, not only 127.0.0.1. For example: bind FastAPI/Uvicorn to `0.0.0.0` behind LAN firewall restrictions, 
and set `PSYDEKIK_HEALTH_URL` to the server's LAN IP. HTTP is for trusted-LAN prototyping only; use authenticated HTTPS for privileged commands later.

## Notes
The HTTP 200 status is treated as an available server, not proof that every Psydekik subsystem is healthy. 
The firmware intentionally contains no API key or arbitrary remote execution facility.
