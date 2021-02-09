from mininet.net import Mininet
from mininet.cli import CLI
from mininet.link import Link

if __name__ == "__main__":

    net = Mininet()

    #add hosts
    h1 = net.addHost('h1')
    h2 = net.addHost('h2')
    h3 = net.addHost('h3')
    h4 = net.addHost('h4')

    s5 = net.addHost('s5')

    Link(h1,s5)
    Link(h2,s5)
    Link(h3,s5)
    Link(h4,s5)

    net.build()

    h1.cmd("ifconfig h1-eth0 0")
    h2.cmd("ifconfig h2-eth0 0")
    h3.cmd("ifconfig h3-eth0 0")
    h4.cmd("ifconfig h4-eth0 0")

    s5.cmd("ifconfig s5-eth0 0")
    s5.cmd("ifconfig s5-eth1 0")
    s5.cmd("ifconfig s5-eth2 0")
    s5.cmd("ifconfig s5-eth3 0")

    s5.cmd("brctl addbr vlan10")
    s5.cmd("brctl addbr vlan20")
    
    s5.cmd("ifconfig vlan10 up")
    s5.cmd("brctl addif vlan10 s5-eth0")
    s5.cmd("brctl addif vlan10 s5-eth1")

    s5.cmd("ifconfig vlan20 up")
    s5.cmd("brctl addif vlan20 s5-eth2")
    s5.cmd("brctl addif vlan20 s5-eth3")

    h1.cmd("ifconfig h1-eth0 10.0.10.1 netmask 255.255.255.0")
    h2.cmd("ifconfig h2-eth0 10.0.10.2 netmask 255.255.255.0")
    h3.cmd("ifconfig h3-eth0 10.0.20.1 netmask 255.255.255.0")
    h4.cmd("ifconfig h4-eth0 10.0.20.2 netmask 255.255.255.0")

    h1.cmd("ip route add default via 10.0.10.254 dev h1-eth0")
    h2.cmd("ip route add default via 10.0.10.254 dev h2-eth0")
    h3.cmd("ip route add default via 10.0.20.254 dev h3-eth0")
    h4.cmd("ip route add default via 10.0.20.254 dev h3-eth0")

    CLI(net)
    net.stop()
