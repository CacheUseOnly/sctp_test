#!/bin/bash

CLIENTADDR=192.168.5.1
SERVERADDR=192.168.5.2

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

# Test the connection (in both directions)
sudo ip netns exec 'client' ping -c 2 $SERVERADDR
sudo ip netns exec 'server' ping -c 2 $CLIENTADDR
