#!/bin/bash

CLIENTADDR=192.168.5.1
SERVERADDR=192.168.5.2
CLIENTADDR2=192.168.5.3
SERVERADDR2=192.168.5.4
CLIENTADDR_6=2000::5:1
SERVERADDR_6=2000::5:2
CLIENTADDR2_6=2000::5:3
SERVERADDR2_6=2000::5:4

# Create two network namespaces
sudo ip netns add 'client'
sudo ip netns add 'server'

# Create a veth virtual-interface pair
sudo ip link add 'myns-1-eth0' type veth peer name 'myns-2-eth0'

# Assign the interfaces to the namespaces
sudo ip link set 'myns-1-eth0' netns 'client'
sudo ip link set 'myns-2-eth0' netns 'server'

# Change the names of the interfaces (I prefer to use standard interface names)
sudo ip netns exec 'client' ip link set 'myns-1-eth0' name 'eth0'
sudo ip netns exec 'server' ip link set 'myns-2-eth0' name 'eth0'

# Assign an address to each interface
sudo ip netns exec 'client' ip addr add $CLIENTADDR/24 dev eth0
sudo ip netns exec 'server' ip addr add $SERVERADDR/24 dev eth0
sudo ip netns exec 'client' ip addr add $CLIENTADDR2/24 dev eth0
sudo ip netns exec 'server' ip addr add $SERVERADDR2/24 dev eth0
sudo ip netns exec 'client' ip -6 addr add $CLIENTADDR_6/64 dev eth0
sudo ip netns exec 'server' ip -6 addr add $SERVERADDR_6/64 dev eth0
sudo ip netns exec 'client' ip -6 addr add $CLIENTADDR2_6/64 dev eth0
sudo ip netns exec 'server' ip -6 addr add $SERVERADDR2_6/64 dev eth0

# Bring up the interfaces (the veth interfaces the loopback interfaces)
sudo ip netns exec 'client' ip link set 'lo' up
sudo ip netns exec 'client' ip link set 'eth0' up
sudo ip netns exec 'server' ip link set 'lo' up
sudo ip netns exec 'server' ip link set 'eth0' up

# Configure routes
sudo ip netns exec 'client' ip route add default via $CLIENTADDR dev eth0
sudo ip netns exec 'server' ip route add default via $SERVERADDR dev eth0

# Support interleave
sudo ip netns exec 'client' sysctl -w net.sctp.intl_enable=1
sudo ip netns exec 'server' sysctl -w net.sctp.intl_enable=1

# Support reconfig
sudo ip netns exec 'client' sysctl -w net.sctp.reconf_enable=1
sudo ip netns exec 'server' sysctl -w net.sctp.reconf_enable=1

# Support ASCONF
sudo ip netns exec 'client' sysctl -w net.sctp.addip_enable=1
sudo ip netns exec 'server' sysctl -w net.sctp.addip_enable=1

sudo ip netns exec 'client' sysctl -w net.sctp.default_auto_asconf=1
sudo ip netns exec 'server' sysctl -w net.sctp.default_auto_asconf=1

sudo ip netns exec 'client' sysctl -w net.sctp.addip_noauth_enable=1
sudo ip netns exec 'server' sysctl -w net.sctp.addip_noauth_enable=1

# Test the connection (in both directions)
sudo ip netns exec 'client' ping -c 1 $SERVERADDR
sudo ip netns exec 'server' ping -c 1 $CLIENTADDR
sudo ip netns exec 'client' ping -c 1 $SERVERADDR2
sudo ip netns exec 'server' ping -c 1 $CLIENTADDR2

sudo ip netns exec 'client' ping6 -c 1 $SERVERADDR_6
sudo ip netns exec 'server' ping6 -c 1 $CLIENTADDR_6
sudo ip netns exec 'client' ping6 -c 1 $SERVERADDR2_6
sudo ip netns exec 'server' ping6 -c 1 $CLIENTADDR2_6
