# SASD Protocols

Tests of many syncronization methods with TCP:

## Protocols

* Timewarp + LocalLag
* Perceptive Consistency
* Trailing State Syncronization
* Dead Reckoning

## Build instructions

**Requirements**:

- `cmake` >= 3.0
- `SDL2` (Graphics library)

Tested on linux and macOS.

First, compile netutils library:

```bash
# Inside the project root directory execute
cd src/netutils
mkdir build
cd build
cmake ..
make
make install
```

Then, compile tests:

```bash
# Inside the project root directory execute
mkdir build
cd build
cmake ..
make
make install # Copies useful start scripts to build directory
```

If succesful, the following binary files will be generated:

- `server`: Broadcast server. Simulates p2p multicast with a TCP connection.
- `dr_server`: Centralized server. Simulates game state updates and sends them back to all connected clients. Used for the dead reckoning protocol.
- `game_timewarp`: Timewarp client implementation.
- `game_fasteventordering`: Perceptive consistency client implementation.
- `game_deadreckoning`: Dead reckoning client implementation.

## How to run

**Timewarp test**:

As a p2p protocol, it need an instance of `server` running before any client is connected. To ensure consistency in tests, it is preferable to start all `game_timewarp` clients simultaneously.

To run predefined tests, from build directory execute:

```bash
# In a terminal
./server
# In another terminal
sh tw_start.sh
```

**Perceptive consistency test**:

As a p2p protocol, it need an instance of `server` running before any client is connected. To ensure consistency in tests, it is preferable to start all `game_fasteventordering` clients simultaneously.

**Note** that `game_fasteventordering` needs to know how many clients there will be connected, so it receives a single argument to acknoledge it at runtime.

To run predefined tests, from build directory execute:

```bash
# In a terminal
./server
# In another terminal
sh feo_start.sh
```

**Dead reckoning test:**

This protocol needs its server executed at the same time as the clients since it also simulates new states. To ensure consitency, all `game_deadreckoning` clients and `dr_server` must be started simultaneously.

To run predefined tests, from build directory execute:

```bash
sh dr_start.sh
```

## Simulated lag

Lag can be simulated by modifying the following parameters:

**Timewarp**:

In the beginning of the file `src/game_timewarp.c`, a *"lag matrix"* is defined. Each cell indicates the lag from client `row` to client `column`.

**Perceptive consistency**:

In the beginning of the file `src/game_fasteventordering.c`, a *"lag matrix"* is defined. Each cell indicates the lag from client `row` to client `column`.

**Dead reckoning**:

In the beginning of the file `src/game_deadreckoning.c`, there are 2 *"lag arrays"*:

- `input_lags` indicate the simulated delay for the incoming states from the server.
- `local_lags` indicate the delay between the event summoning frame and its execution frame (before it has reached the server and back).

## Document compilation instructions

Required packages:

- `texlive-fourier`
