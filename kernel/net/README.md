Linux Kernel Net Features
=========================

# TCP Fast Open, by google at CoNext2011
## Status
Merged into Linux kernel from 3.7+
The function can be opened with cmd
echo 3 > /proc/sys/net/ipv4/tcp_fastopen

## Test
Mimic a server and a client, interacting through TCP protocol
with different size of data immediately 
after the TCP three-way handshake initilization.
