# NeXonDDoSProtection

**NeXonDDoSProtection** is a robust and efficient DDoS protection system built in C++ designed to safeguard web applications from malicious traffic. It combines **rate-limiting**, **reverse proxying**, and **IP blocking** to detect and mitigate excessive or harmful requests in real-time. This project supports both **HTTP** and **HTTPS** traffic and is fully configurable through an `.env` file.

## Features
- **HTTP/HTTPS Support**: Seamlessly handles both HTTP and HTTPS traffic by forwarding requests to a target URL.
- **Rate Limiting**: Automatically limits excessive requests from individual IP addresses to prevent abuse.
- **IP Blocking**: Temporarily blocks malicious IPs that exceed rate limits.
- **Reverse Proxy**: Forwards legitimate traffic to the intended target URL.
- **Configurable Parameters**: Flexible `.env` configuration for quick and easy setup.
- **High Performance**: Built with multithreading to handle multiple clients concurrently.

## Table of Contents
- [Getting Started](#getting-started)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [How It Works](#how-it-works)
- [Contributing](#contributing)
- [License](#license)

---

## Getting Started

### Prerequisites
Ensure you have the following tools installed:
- A **Linux-based VPS** or server.
- `g++` compiler for C++.
- `libcurl` development library for HTTP/HTTPS handling.

Install dependencies:
```bash
sudo apt-get install g++ libcurl4-openssl-dev
```

---

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/NeXeonSystems/NeXonDDoSProtection
   cd NeXonDDoSProtection
   ```

2. **Set Up Configuration**:
   Create a `.env` file in the project directory:
   ```bash
   nano .env
   ```
   Add the following configuration (modify as needed):
   ```env
   TARGET_URL=https://yoursite.com
   RATE_LIMIT=100
   TIME_WINDOW=10
   PROXY_PORT=80
   BLOCK_DURATION=60
   ```

3. **Compile the Code**:
   Use the `g++` compiler to build the executable:
   ```bash
   g++ -o NeXonDDoSProtection NeXonDDoSProtection.cpp -lcurl -lpthread
   ```

4. **Run the Program**:
   Execute the compiled binary:
   ```bash
   ./NeXonDDoSProtection
   ```

---

## Usage

Once the application is running:
1. Configure your **application** or **service** to use the reverse proxy by pointing to `http://<your-server-ip>:80`.
2. The proxy will forward all legitimate requests to the `TARGET_URL` specified in the `.env` file while blocking or rate-limiting excessive requests.

---

## Configuration

The `.env` file controls the behavior of NeXonDDoSProtection. Below are the available options:

| Parameter       | Description                                                                 | Default Value       |
|------------------|----------------------------------------------------------------------------|---------------------|
| `TARGET_URL`     | The URL of the site to protect (e.g., `https://yoursite.com`).             | Required            |
| `RATE_LIMIT`     | Maximum allowed requests per client within the time window.                | 100                 |
| `TIME_WINDOW`    | The duration of the rate limit window (in seconds).                        | 10                  |
| `PROXY_PORT`     | The port on which the reverse proxy listens for incoming traffic.          | 80                  |
| `BLOCK_DURATION` | How long (in seconds) to block IPs that exceed the rate limit.             | 60                  |

---

## How It Works

### Core Features
1. **Rate Limiting**:
   - Tracks the number of requests per IP address.
   - If an IP exceeds the allowed number of requests (`RATE_LIMIT`) within the time window (`TIME_WINDOW`), it is temporarily blocked for the specified duration (`BLOCK_DURATION`).

2. **Reverse Proxy**:
   - Forwards legitimate requests to the `TARGET_URL`.
   - Ensures malicious traffic does not reach the target site.

3. **IP Blocking**:
   - Once an IP is flagged as malicious, it is temporarily blocked and added to the block list.
   - Blocked IPs are automatically cleared after the specified `BLOCK_DURATION`.

### Multithreading
The system uses multithreading to handle multiple incoming requests concurrently, ensuring high performance even under heavy traffic.

---------------------------------------------------------
**© NeXeonSystems & Nethuka, 2024. All Rights Reserved.**
---------------------------------------------------------
