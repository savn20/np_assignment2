# UDP Assignment

### Initial Setup

- [ ] Study `calcMessage` and `calcProtocol` structures
- [ ] Establish connection to server -> send `calcMessage`

### Handle abort

- if server doesn't support, it'll report with `calcMessage` with

```js
var abort = {
    type=2, 
    message=2, 
    major_version=1,
    minor_version=0
}
```
    
- The client should print a notification that the server sent a 'NOT OK' message, then terminate.

### Perform mentioned assignment 

- [ ] if server supports the client, it'll respond with `calcProtocol` with assignment to perform

### Handle timeouts
- [ ] Start a timer and after 2 seconds, if there's no reply send again
- [ ] After three such timeouts, it must abort and print a notification that the server did not reply.
- [ ] Study the local IP Issue -> Hence, in the UDP unbound socket case, it is ok to show the obtained address of '0.0.0.0'.