
from mininet.net import Mininet
from mininet.cli import CLI
from mininet.link import Link

if '__main__' == __name__:

    net = Mininet()

    #Add Hosts
    h1 = net.addHost('h1')
    h2 = net.addHost('h2')

    #Add L2 Switch s5
    s5 = net.addHost('s5')

    link_h1s5 = net.addLink('h1', 's5', intfName1 = 'h1-s5', intfName2 = 's5-h1')
    link_h2s5 = net.addLink('h2', 's5', intfName1 = 'h2-s5', intfName2 = 's5-h2')
    #Link(h1, s5)
    #Link(h2, s5)

    net.build()

    h1.cmd("ifconfig h1-s5 0")
    h2.cmd("ifconfig h2-s5 0")

    s5.cmd("ifconfig s5-h1 0")
    s5.cmd("ifconfig s5-h2 0")

    s5.cmd("brctl addbr vlan10")
    s5.cmd("ifconfig vlan10 up")

    s5.cmd("brctl addif vlan10 s5-h1")
    s5.cmd("brctl addif vlan10 s5-h2")

    h1.cmd("ifconfig h1-s5 10.0.10.1 netmask 255.255.255.0")
    h2.cmd("ifconfig h2-s5 10.0.10.2 netmask 255.255.255.0")

    h1.cmd("ip route add default via 10.0.10.254 dev h1-s5")
    h2.cmd("ip route add default via 10.0.10.254 dev h2-s5")

    CLI(net)
    
    net.stop()
