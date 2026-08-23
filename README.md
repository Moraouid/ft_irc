*This project has been created as part of the 42 curriculum by isakrout, sel-abbo, sfaouzi.*

# ft_irc

## Description

This project is a lightweight IRC server written in C++98. It implements the core networking and chat logic needed to allow multiple clients to connect, authenticate, join channels, exchange messages, and use channel moderation features over a real TCP socket connection.

The server is built around the standard Unix socket API and uses `poll()` to manage multiple simultaneous clients efficiently. It was designed as a faithful exercise in network programming, message parsing, and event-driven server architecture.

### Included features

- Client connection handling with non-blocking sockets
- Authentication flow using `PASS`, `NICK`, and `USER`
- Private messaging (`PRIVMSG`) between users
- Channel creation and membership management (`JOIN`, `PART`)
- Channel operator features such as `INVITE`, `KICK`, `TOPIC`, and basic mode handling
- Bot integration (`loffi`) responding to commands such as `!help`, `!time`, and `!users`
- Graceful shutdown handling with clean socket cleanup

## Instructions

### Requirements

- A Unix-like environment (Linux is used for this project)
- A C++ compiler compatible with the C++98 standard
- `make` available in the system shell

### Compilation

From the project root, run:

```bash
make
```

This will compile the server executable:

```bash
./ircserv
```

### Execution

The server is configured to listen on port `6667` and uses the default password `pass` as defined by the project code.

To connect, you can use any IRC client or a simple TCP client such as `nc`:

```bash
nc 127.0.0.1 6667
```

A typical login sequence is:

```text
PASS pass
NICK alice
USER alice 0 * :Alice
JOIN #general
PRIVMSG #general :Hello everyone!
```

### Project structure

- `server.cpp` / `server.hpp`: server lifecycle, socket management, polling, and client handling
- `client.cpp` / `client.hpp`: client state and command dispatch logic
- `channel.cpp` / `channel.hpp`: channel storage, membership, and channel modes
- `command.cpp` / `command.hpp`: IRC command implementations
- `irc_utils.cpp` / `irc_utils.hpp`: IRC message formatting and helper functions
- `Bot.cpp` / `Bot.hpp`: minimal bot behavior for server-side interaction
- `main.cpp`: startup entry point

## Resources

### Relevant references

- RFC 1459: Internet Relay Chat Protocol
- RFC 2812: Internet Relay Chat: Client Protocol
- POSIX sockets programming documentation and examples for `socket()`, `bind()`, `listen()`, `accept()`, and `recv()`
- Linux `poll(2)` documentation for multiplexed I/O
- IRC community documentation and protocol examples for command semantics and message formatting

### AI usage

AI tools were used as a support layer during the development of this project for:

- clarifying IRC command behavior and server/client message flow
- reviewing command parsing and edge cases around invalid input and channel restrictions
- suggesting clean patterns for managing multiple client connections with `poll()`
- helping structure the code and generate documentation text, including parts of the README and implementation notes

These tools were used as an aid for reasoning, debugging, and documentation, but the protocol logic, socket design, and final integration were implemented in the project itself.

## Notes

This project is a simplified IRC server and focuses on the core protocol mechanisms expected in the 42 curriculum. It is intended as a practical introduction to network programming and backend concurrency patterns in C++.