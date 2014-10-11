pcap_t * raw_start(char * localip, char * filterme);
void raw_loop(pcap_t * adhandle, void (*packet_handler)(u_char *, const struct pcap_pkthdr *, const u_char *));

/* list all of the interfaces to stdout */
void raw_list();

/* adjust the BPF associated with the device */
void raw_filter(pcap_t * handle, char * localip, char * targetip, char * filter);
