 Kalshi Market Maker

This project implements a minimal market making engine and websocket client for Kalshi’s trading API. It connects to Kalshi’s live market feed, subscribes to orderbook updates, and runs a strategy via a modular engine architecture.

⸻

Project Overview

The system is composed of:
	•	Engine — Core event loop for handling feed events and running strategies
	•	Strategy — Defines custom market making logic (e.g. KalshiMM)
	•	KalshiWsAdapter — Parses Kalshi WebSocket messages into structured FeedEvent objects
	•	WsClient — Generic, reconnecting WebSocket client that handles subscriptions and message routing
	•	KalshiAuth — Handles HMAC-style signing of requests to Kalshi’s API (for both HTTP and WS)


Dependencies

System:
	•	CMake (>= 3.15)
	•	g++ or clang++ with C++20 support
	•	libssl-dev (for signing)
	•	libcurl4-openssl-dev
	•	nlohmann-json
	•	ixwebsocket (WebSocket client)

Configuration

Your runner.json should contain:

```json
{
  "kalshi_private_key_path": "./path/to/kalshi_private.key",
  "kalshi_key_id": "YOUR-API-KEY-ID"
}

```

Installing Dependencies via vcpkg

If you don’t have vcpkg installed:
```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh    # or .\bootstrap-vcpkg.bat on Windows
```

Then install all required libraries:
```bash
./vcpkg install ixwebsocket nlohmann-json openssl
```

Make sure to integrate vcpkg with your CMake build system:
```bash
./vcpkg integrate install
```

Building
From the repository root:

```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=/Users/jakezur/dev/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

```

Then link compile commands for your linter to find:
```bash
ln -sf ./build/compile_commands.json ./compile_commands.json
```

Configuring API Credentials
Create a runner.json in the project root:
```json
{
  "kalshi_private_key_path": "./kalshi_private_pkcs8.key",
  "kalshi_key_id": "YOUR-API-KEY-ID"
}
```

Your private key must be in PKCS#8 PEM format.
If your key is in traditional OpenSSL format (the one I made from kalshi was the first time),
convert it with:
```bash
openssl pkcs8 -topk8 -inform PEM -outform PEM -nocrypt \
  -in kalshi_private_key.pem -out kalshi_private_pkcs8.key
```

To Run
From build:
```bash
./kalshi_mm ../runner.json
```


