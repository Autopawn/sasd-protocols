# Sasd Protocols

Tests of many syncronization methods with TCP:

## Global considerations:

The server has the global clock that the clients attempt to synchronize with (depending of the protocol), using NTP.

Lag can be simulated on three possible ways: server-client, client-server, and client-client (for p2p protocols).

## Protocols

### Timewarp + LocalLag

* **Global clock:**

* **Function of the server:** As this protocol is designed for p2p, the server only takes care of keeping the global clock and the IP directions of the clients currently connected.

* **Responsivity:**

### Perceptive Consistency

* **Global clock:**

* **Function of the server:** As this protocol is designed for p2p, the server only takes care of keeping the global clock and the IP directions of the clients currently connected.

* **Responsivity:**

### Trailing State Syncronization

* **Global clock:**

* **Function of the server:**

* **Responsivity:**

### Dead Reckoning

<!-- Check if this is correct! -->

* **Global clock:** There is no need of using the clock as all the actions are executed as long as received and after the client receive a state of the simulation from the server, they assume that the current time is the one of the received state (so clients see a delayed vesion of the reality).

* **Function of the server:** All clients send their actions to the server, which holds the true version of the reality and sends it to the clients periodically. In the meantime the clients move the simulation forward locally.

* **Responsivity:** The actions are perceived by a client after they reach the server and the state comes back.

## Document compilation instructions

Required packages:

- `texlive-fourier`
