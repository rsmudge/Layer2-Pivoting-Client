/* receive an ethernet frame [size][buffer] */
int client_recv_frame(SOCKET ch, void * buffer, int maxlength);

/* send an ethernet frame */
void client_send_frame(SOCKET ch, void * buffer, int len);

/* Construct a TCP channel */
SOCKET client_connect(char * targetip, int port);
