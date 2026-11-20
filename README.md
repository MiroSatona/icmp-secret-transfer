# secret: Encrypted File Transfer over ICMP

A client–server application written in C++20 that transfers files inside the data
section of **ICMP / ICMPv6 Echo-Request and Echo-Reply** messages. Each chunk of the
file is encrypted with **AES-256-CBC** before it is sent. The application supports
both **IPv4 and IPv6**.

This is a school project for the **ISA** course (Network Applications and Network
Administration) at the Faculty of Information Technology, Brno University of
Technology (BUT FIT), 2025. The full documentation is in [manual.pdf](manual.pdf), 
and the man page is in [secret.1](secret.1).

## How it works

- The client reads the file, splits it into blocks and encrypts each block separately.
  Every block gets its own encryption context and a random 16-byte IV.
- The encrypted blocks travel inside the payload of ICMP Echo messages, together with
  a small custom application protocol.
- The server listens on raw ICMP and ICMPv6 sockets, checks and decrypts the incoming
  blocks, and writes the file to its current working directory.
- The AES key is the SHA-256 hash of a shared secret (`LOGIN_KEY` in
  [inc/cipher/abstract_cipher.hpp](inc/cipher/abstract_cipher.hpp)). The client and
  the server must be built with the same value.

## Requirements

- Linux
- `g++` with C++20 support
- `make`
- OpenSSL development libraries (`libssl-dev` on Debian/Ubuntu)
- Root privileges to run the program, because it uses raw sockets

## Build

```sh
make          # builds the ./secret binary
make clean    # removes object files, the binary and temporary test files
```

## Usage

```sh
secret -r <file> -s <ip|hostname>   # client: send a file
secret -l                           # server: listen and receive files
secret -h                           # show help
```

| Option | Description |
|--------|-------------|
| `-r <file>` | Path (relative or absolute) to the file to send |
| `-s <ip\|hostname>` | IPv4/IPv6 address or hostname of the server |
| `-l` | Server mode. The server receives files until it is stopped with `Ctrl+C` |

Example:

```sh
# on the server
sudo ./secret -l

# on the client
sudo ./secret -r images/image.png -s 10.10.10.115
```

### Exit codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Invalid or missing command-line arguments |
| 2 | Protocol error (for example, unacknowledged packets or a failed transfer) |
| 3 | Internal error (encryption/decryption or socket failure) |

## Protocol

Every protocol message is stored in the data part of an ICMP/ICMPv6 Echo message and
starts with a 2-byte message type in network byte order.

| Type | Value | Direction | Meaning |
|------|-------|-----------|---------|
| `HELLO` | `0x01` | C → S | Starts the session and carries the encrypted file name |
| `HELLO_ACK` | `0x02` | S → C | Session accepted |
| `DATA` | `0x03` | C → S | Encrypted block of the file |
| `DATA_ACK` | `0x04` | S → C | Block received |
| `FIN` | `0x06` | C → S | End of transfer |
| `FIN_ACK` | `0x07` | S → C | Transfer finished successfully |
| `FIN_ERR` | `0x08` | S → C | Server-side error during the transfer |

`HELLO` and `DATA` messages:

```
+-------------------+---------------------+
| Type (2 B)        | Data length (4 B)   |
+-------------------+---------------------+
| Encrypted data (N B)                    |
+-----------------------------------------+
| Initialization vector (16 B)            |
+-----------------------------------------+
| Checksum of the payload (4 B)           |
+-----------------------------------------+
```

All other messages (`HELLO_ACK`, `DATA_ACK`, `FIN`, `FIN_ACK`, `FIN_ERR`) contain only
the type (2 B) and the checksum (4 B).

### Reliability

The client and the server are both driven by state machines
(`CLIENT_STATE` and `SERVER_STATE` in
[inc/types/protocol_definitions.hpp](inc/types/protocol_definitions.hpp)):

- Every message is acknowledged. The client waits **400 ms** for each reply and
  retries up to **3 times**.
- If the server does not answer `HELLO`, the client waits **1500 ms** and tries again,
  up to 3 rounds, before it gives up.
- The server ends a session with a client that has been inactive for **5500 ms**.
- When the server is stopped with `Ctrl+C` during a transfer, it sends `FIN_ERR` to the
  client.
- If the client cannot confirm that the whole file was delivered, it exits with an error.

## Project structure

```
inc/                      header files
├── cipher/               AES encryption and decryption (OpenSSL)
├── file_handle/          reading and writing files
├── network/              raw ICMP/ICMPv6 socket handling for the client and the server
├── program/              program logic: client, server, help, argument parsing
├── protocol_message/     building and parsing protocol messages
└── types/                protocol constants, message types, return codes
src/                      implementation of the classes, main.cpp
tests/run_test.sh         test script
secret.1                  man page
manual.pdf                full documentation (Czech)
```

## Tests

The test script checks argument handling and transfers random files of different
sizes (1 B, one full chunk, small and large files) over IPv4 (`127.0.0.1`) and
IPv6 (`::1`). It then compares the received file with the original.

```sh
make test     # builds the program and runs tests/run_test.sh (uses sudo if needed)
```

## Author

Martin Zůbek

## License

This project is licensed under the GNU Affero General Public License v3.0.
See [LICENSE](LICENSE).
