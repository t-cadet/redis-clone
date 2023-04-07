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
