#include "pcap.h"
#include "client.h"
#include "raw.h"

/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

/* make this global */
static SOCKET   server;
static pcap_t * sniffer;

DWORD ThreadProc(LPVOID parm);

void vpn_start(char * targetip, int port, char * localip) {

	/* first... connect to our VPN server */
	server = client_connect(targetip, port);

	/* makes the specified interface start sniffing */
	sniffer = raw_start(localip, targetip);

	/* start reading stuff */
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadProc, (LPVOID) NULL, 0, NULL);

	/* start looping... */
	raw_loop(sniffer, packet_handler);
}

DWORD ThreadProc(LPVOID param) {
	char * buffer = malloc(sizeof(char) * 65536);
	int len, result;
	unsigned short action;

	while (TRUE) {
		len = client_recv_frame(server, buffer, 65536);

		/* inject the frame we received onto the wire directly */
		result = pcap_sendpacket(sniffer, (u_char *)buffer, len);
		if (result == -1) {
			printf("Send packet failed: %d\n", len);
		}
	}
}

/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char * param, const struct pcap_pkthdr * header, const u_char * pkt_data) {
	/* send the raw frame to our server */
	client_send_frame(server, (void *)pkt_data, header->len);
}

int main(int argc, char * argv[]) {
	/* check the args.. */
	if (argc != 4) {
		printf("%s [remote ip] [port] [local ip]\n", argv[0]);
		return 0;
	}

	vpn_start(argv[1], atoi(argv[2]), argv[3]);
	return 0;
}

