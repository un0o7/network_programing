#!/bin/bash

iptables -F
iptables -t filter -P INPUT DROP
iptables -t filter -P OUTPUT DROP
iptables -t filter -P FORWARD DROP

iptables -A INPUT -p icmp -j ACCEPT 
iptables -A OUTPUT -p icmp -j ACCEPT
iptables -A FORWARD -p icmp -j ACCEPT

iptables -A INPUT -p tcp --dport 22 -j DROP 
iptables -A OUTPUT -p tcp --sport 22  -j DROP
