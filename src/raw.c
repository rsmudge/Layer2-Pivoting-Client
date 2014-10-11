/*
 * Code for managing raw access to network resources
 */
#include <time.h>
#include <stdio.h>
#include "pcap/pcap.h"
#define PCAP_OPENFLAG_PROMISCUOUS       1
#define PCAP_OPENFLAG_NOCAPTURE_LOCAL   8
#include "client.h"

/* filter out a host */
void raw_filter_internal(pcap_t * handle, pcap_if_t * device, char * targetip, char * filter) {
	u_int netmask;
	char packet_filter[256];
	struct bpf_program fcode;

	/* setup our BPF filter */
	if (filter == NULL) {
		_snprintf(packet_filter, 256, "not host %s", targetip);
	}
	else {
		_snprintf(packet_filter, 256, "(%s) && not host %s", targetip, filter);
	}

	/* try to determine the netmask of our device */
	if (device->addresses != NULL)
		netmask = ((struct sockaddr_in*)(device->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		netmask = 0xffffff;

	/* compile our pcap filter */
	if (pcap_compile(handle, &fcode, packet_filter, 1, netmask)) {
		printf("Could not compile the filter: %s\n", targetip);
		return;
	}

	/* set it up */
	if (pcap_setfilter(handle, &fcode) < 0) {
		printf("Could not set the filter\n");
	}
}

pcap_if_t * find_interface(pcap_if_t ** alldevs, char * localip) {
	pcap_if_t * d;
	char errbuf[PCAP_ERRBUF_SIZE + 128];
	struct pcap_addr * addr;

	/* Retrieve the device list on the local machine */
	if (pcap_findalldevs_ex("rcap://", NULL, alldevs, errbuf) == -1) {
		printf("Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}

	for (d = *alldevs; d; d = d->next) {
		for (addr = d->addresses; addr; addr = addr->next) {
			char * checkme = inet_ntoa(((struct sockaddr_in *)addr->addr)->sin_addr);
			if (strcmp(checkme, localip) == 0)
				return d;
		}
	}

	printf("Interface for '%s' not found\n", localip);
	exit(0);
}

pcap_t * raw_start(char * localip, char * filterip) {
	pcap_t * adhandle   = NULL;
	pcap_if_t * d       = NULL;
	pcap_if_t * alldevs = NULL;
	char errbuf[PCAP_ERRBUF_SIZE];

	/* find out interface */
	d = find_interface(&alldevs, localip);

	/* Open the device */
	adhandle = (pcap_t *)pcap_open(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS | PCAP_OPENFLAG_NOCAPTURE_LOCAL, 1, NULL, errbuf);
	if (adhandle == NULL) {
		printf("\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		return NULL;
	}

	/* filter out the specified host */
	raw_filter_internal(adhandle, d, filterip, NULL);

	/* ok, now we can free out list of interfaces */
	pcap_freealldevs(alldevs);

	return adhandle;
}

void raw_loop(pcap_t * adhandle, void (*packet_handler)(u_char *, const struct pcap_pkthdr *, const u_char *)) {
	pcap_loop(adhandle, 0, packet_handler, NULL);
}
