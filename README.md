# UDP Assignment

### UDP Client

1. Initial Setup

   - [x] Study `calcMessage` and `calcProtocol` structures
   - [x] Establish connection to server -> send `calcMessage`

2. Handle abort

   - [x] if server doesn't support, it'll report with `calcMessage` with

   ```js
   var abort = {
       type=2,
       message=2,
       major_version=1,
       minor_version=0
   }
   ```

   - The client should print a notification that the server sent a 'NOT OK' message, then terminate.

3. Perform mentioned assignment

   - [x] if server supports the client, it'll respond with `calcProtocol` with assignment to perform

4. Handle timeouts

   - [x] Start a timer and after 2 seconds, if there's no reply send again
   - [x] After three such timeouts, it must abort and print a notification that the server did not reply

### UDP Server

1. Server Setup
   - [x] Create a UDP server at given ip:port
   - [x] Start listening for clients
2. Handling Clients
   - [x] Handle multiple clients
   - [x] Accept clients that supports version 1
3. Task Management
    - [x] Generate random task
    - [x] Set timeout for 10s
    - [x] Reject clients that sends incorrect IDs 


### Debugging

While building it's common to log debug info to the console,
but it's best to have a flag to turn on/off those logs

- [x] use macros to turn on/off debugging
