# Block 7/1

## a)

_Via Wireshark GUI:_

	In the bottom pane after "Packets:"

**Packat count**
62856

## b)

_Via Wireshark GUI:_

	Statistics > Paket Length... > "" (no filter) > Create Stat 
	First row after in column "average"

**Average Size**
1221,15 Bytes

## c)

_Via TShark:_

	tshark -r trace.cap -T fields -e eth.dst > destination-mac.list
	tshark -r trace.cap -T fields -e eth.src > source-mac.list
	cat *-mac.list | sort | uniq
	
_Via Wireshark GUI:_
	
	Statistics > Endpoints > Uncheck "Name resolution"
	Column "Address"
	
**Found MAC-Addresses**	
00:0d:56:c7:76:0e
00:0f:3d:fc:75:2b
00:11:25:43:ca:fe
ff:ff:ff:ff:ff:ff	

## d)

_Via TShark:_

	tshark -r trace.cap -2 -R "tcp.flags==2" -T fields -e ip.src > source-ips.list
	tshark -r trace.cap -2 -R "tcp.flags==2" -T fields -e ip.dst > destination-ips.list
    cat *-ips.list | sort | uniq
    
_Via Wireshark GUI:_
	
	Statistics > Endpoints > Tab "IPv4"
	Column "Address"

134.220.3.28
154.11.129.59
159.33.166.3
172.176.84.86
192.168.0.4
199.71.40.129
199.71.40.137
199.71.40.139
202.156.187.206
207.126.111.225
207.188.7.81
208.187.91.202
208.38.45.168
208.38.45.169
209.148.129.83
209.62.176.181
209.62.176.51
212.120.229.6
212.179.164.48
212.58.226.53
216.52.17.216
217.125.164.176
217.164.237.108
217.164.244.4
217.165.102.173
217.17.253.32
24.130.7.93
24.42.136.83
24.80.224.201
64.157.228.20
64.192.137.102
65.205.8.187
65.216.78.79
65.43.156.13
68.196.243.184
68.206.53.235
69.175.156.4
69.199.189.231
70.24.178.54
70.48.201.244
71.198.243.48
72.14.203.104
72.56.18.248
80.198.2.220
82.156.213.83
84.13.71.61
84.175.93.161

**Anzahl IP Addressen**
47 verschiedene

## e)

## m)

	Filter "ip.ttl > 200", "ip.ttl == 128" and "ip.ttl == 48"
	
**IP-Pakets with specific TTLs**
23 pakets with more than 200
16757 pakets with exactly 128
105 pakets with exactly 48

## n)

	[CTRL] + [G] 16
		
### a)

	Click on "Ethernet II" in packet description and read from status bar
	
**Ethernet-header size**
14 bytes

### b)

**IP-header size**
20 bytes

### c)

	(In packet inspector) IPv4 > Total Length

**IP-datagram size**
1380 bytes

### d)

**TCP-header size**
20 bytes

### e)

	(In packet inspector) IPv4 > TCP Segment Len
	
**TCP-datagram size**
1336 bytes

## p)

	Statistics > Conversations > Tab "IPv4" > Order by column "Bytes"
	
**Most bytes shared**
"60.254.20.53" <-> "192.168.0.4" 
10032803 Bytes (7114 up, 8634889 down)

	Statistics > IO Graph
	Then apply filter "ip.addr==60.254.20.53 && ip.addr==192.168.0.4"

**Histogram**
See file `./71-P.png`

#### TODO: Interpret


## q)

	Filter packages by protocol "ssl"
	Copy destination/source IPs that are outside the local network
	Enter in Webbrowser to get hostname
	
**SSH Connections**
Yes, the client (where the browser was detected later) established a secure connection to `canada.com`.

## r)

	Filter packages by protocol "http"
	Inspect any "GET ..." package
	HTTP > User-Agent Header

**Browser Benutzung**
Yes, Firefox 1.0.7 on a US-English Windows XP.
_Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.7.12) Gecko/20050915 Firefox/1.0.7_

## s)

    Filter packages by protocol "rtsp"
	
**Media Stream**
A video (or similar) from CBC News namend "Harper fends off attacks from Duceppe: Martin in final debate"

## t)

**P2P Filesharing**
Yes, via BitTorrent.






