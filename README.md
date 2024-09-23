# ft_irc

## Table of Contents
1. [Introduction](#introduction)
2. [Features](#features)
3. [Installation](#installation)
4. [Usage](#usage)
5. [Contributors](#contributors)

## Introduction
`ft_irc` is a custom implementation of an IRC (Internet Relay Chat) server. This project is part of the 42 curriculum and follows the guidelines outlined in the 42 ft_irc subject. The goal is to implement an IRC server that adheres to the RFC 1459 standard, allowing multiple clients to connect and communicate with each other.

## Features
- Handles multiple users and channels.
- Support for basic IRC commands:
  - `PASS`: Enter server's password
  - `NICK`: Set a user's nickname.
  - `USER`: Register a user to the server.
  - `QUIT`: Quit the server.
  - `JOIN`: Join a channel.
  - `PRIVMSG`: Send a private message.
  - `PING`: Connection keep-alive.
  - `TOPIC`: View or change the channel topic.
  - `INVITE`: Invite user to a channel.
  - `KICK`: Kick user from a channel.
  - `WHOIS`: List users on a channel.
  - `MODE`:
    - `+i`/`-i`: Set/remove invite-only channel.
    - `+t`/`-t`: Set/remove the right to change topic to operators only.
    - `+k`/`-k`: Set/remove channel key.
    - `+o`/`-o`: Give/take operator rights.
    - `+l`/`-l`: Set/remove channel's user limit.
- Connection handling with multiple clients.
- Error management for incorrect commands or syntax.
- Compatibility with common IRC clients.
- A bot (PRIVMSG Bot HELP)

## Installation

To compile the project, simply clone this repository and run `make`:
```bash
git clone https://github.com/jocorrea42/ft_irc.git
cd ft_irc
make
```
This will create the executable `ircserv`.

## Usage

To start the IRC server, run the following command:
```bash
./ircserv <port> <password>
```
- `<port>`: The port number on which the server will listen for connections.
- `<password>`: The password clients need to connect to the server.
Example:
```bash
./ircserv 6667 ok
```
You can connect to the server using an IRC clients like `irssi`,  `WeeChat` or `HexChat`.

## Contributors

- [arnoop88](https://github.com/arnoop88)
- [jocorrea42](https://github.com/jocorrea42)
