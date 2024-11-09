a script to switch between wireguard configs

build: make all

pre-requirements: wireguard-tools

example:
---
0|off
1|oracle
2|proton <-- select
3|warp   <-- origin
---
output: (invokes wg-quick commands)
----- shutdown ------
[#] ip -4 rule delete table 51820
[#] ip -4 rule delete table main suppress_prefixlength 0
[#] ip -6 rule delete table 51820
[#] ip -6 rule delete table main suppress_prefixlength 0
[#] ip link delete dev warp
[#] resolvconf -d warp -f
[#] nft -f /dev/fd/63
---------------------
----- configure -----
[#] sysctl -w net.ipv6.conf.all.disable_ipv6=1
net.ipv6.conf.all.disable_ipv6 = 1
[#] ip link add proton type wireguard
[#] wg setconf proton /dev/fd/63
[#] ip -4 address add 10.2.0.2/32 dev proton
[#] ip link set mtu 1420 up dev proton
[#] resolvconf -a proton -m 0 -x
[#] wg set proton fwmark 51820
[#] ip -4 route add 0.0.0.0/0 dev proton table 51820
[#] ip -4 rule add not fwmark 51820 table 51820
[#] ip -4 rule add table main suppress_prefixlength 0
[#] sysctl -q net.ipv4.conf.all.src_valid_mark=1
[#] nft -f /dev/fd/63
---------------------

