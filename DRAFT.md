> : included in
store > cmd, server
cmd, net > client, server

server:
 - server creates store
 - server listens on socket
 - server parses received commands
 - server executes commands (commands need to know about the store)
 - server sends response on socket

client:
 - connect to server
 - query user for text
 - send text to server
 - parse response to cmd result
 - display response

Architecture:
 - netstring
 - cmd 
    - class Cmd<Output>
        - parse from netstring
        - execute(store)
        - Output::parse from netstring ()

 - cli <host> <port>
    connect host port
    while True:
        read line
        line -> netstring
        send netstring
        recv netstring
        parse netstring -> Cmd::Output
        print output
        
 - server <host> <port>
    - class store
        ping (only client side?)
        echo(bytes) -> bytes // debug
        get(bytes) -> bytes
        set(bytes, bytes)
        save(path)
        load(path)
        info
        shutdown
        flushall
        (maybe) get_range() -> vec<bytes>
