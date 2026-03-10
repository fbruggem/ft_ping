`ip a`

`sudo tc qdisc add dev lo root netem duplicate 50%`

`sudo iptables -A INPUT -s 127.0.0.1 -p icmp -j DROP`
