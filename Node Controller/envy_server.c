#include "envy_network.h"

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

void htonPacket(struct pkt packet, char buffer[sizeof(struct pkt)]);
void *TCPHandler(void* arg);

int didACK = 0;
int NODE_ID = 0;
pthread_mutex_t lock;

int main(int argc, char** argv)
{
	//Create TCP thread and mutex
	pthread_mutex_init(&lock, NULL);
	pthread_t server_thread;
	pthread_create(&server_thread, NULL, &TCPHandler, NULL);
	

	/*Set up UDP socket for broadcast until we can establish a reliable
	TCP connection with the master controller*/
	int udp_sock, udp_socklen, udp_status;
	struct sockaddr_in bcast;
	int yes;

	udp_socklen = sizeof(struct sockaddr_in);
	memset(&bcast, 0, udp_socklen);
	
	udp_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	bcast.sin_addr.s_addr = htonl(INADDR_ANY);
	bcast.sin_port = htons(0);
	bcast.sin_family = PF_INET;

	udp_status = bind(udp_sock, (struct sockaddr *) &bcast, udp_socklen);
	udp_status = setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));

	bcast.sin_addr.s_addr=htonl(-1);
	bcast.sin_port = htons(UDP_PORT);
	bcast.sin_family = PF_INET;

	struct pkt packet;
	packet.header.pkt_type = PKT_TYPE_STATUS;
	//packet.header.node_id = 0;
	packet.header.status = STATUS_BOOT_OK;
	packet.header.p_length = 0;
	packet.header.timestamp = 0;
	char buffer[sizeof(struct pkt)];
	htonPacket(packet, buffer);
	
	pthread_mutex_lock(&lock);
	fprintf(stdout, "Beginning broadcast on port %d...\n", UDP_PORT);
	while(!didACK)
	{
		udp_status = sendto(udp_sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &bcast, udp_socklen);
		pthread_mutex_unlock(&lock);
		sleep(BCAST_SLEEP);
		pthread_mutex_lock(&lock);
	}

	close(udp_sock);
	pthread_join(server_thread, NULL);
}

//Set up TCP socket to listen for incoming TCP connection from master controller
void *TCPHandler(void *args)
{
	fprintf(stdout, "TCP listener thread running! Waiting for connection on port %d.\n", TCP_PORT);

	int fd;
	struct sockaddr_in serv_addr, cli_addr;
	int yes = 1;

	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(TCP_PORT);

	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}

	if(bind(fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind");
		exit(1);
	}

	listen(fd, 1);
	socklen_t clilen = sizeof(cli_addr);
	int client = accept(fd, (struct sockaddr*) &cli_addr, &clilen);

	if(client < 0)
	{
		perror("accept");
		exit(1);
	}
	else
	{
		pthread_mutex_lock(&lock);
		didACK = 1;
		pthread_mutex_unlock(&lock);
		
		char buffer[sizeof(struct pkt)];
		recvfrom(client, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli_addr, &clilen);
		struct pkt recv_packet = ntohPacket(buffer);
		int tmp;
		memcpy(&tmp, recv_packet.payload.data, sizeof(tmp));
		NODE_ID = ntohl(tmp);
		fprintf(stdout, "Received acknowledgement from master controller! My node ID is %" PRIu16 "!\n", NODE_ID);

		int finished = 0;
		struct pkt send_packet;
		
		while(!finished) //Loop until we receive a stop command from master
		{
			recvfrom(client, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli_addr, &clilen);
			recv_packet = ntohPacket(buffer);

			//Handle each type of packet differently
			if(recv_packet.header.pkt_type == PKT_TYPE_TASK)
			{
				//We got a task from MC....	
			}
			else if(recv_packet.header.pkt_type == PKT_TYPE_CMD)
			{
				//We got a command to do something...
				memcpy(&tmp, recv_packet.payload.data, sizeof(tmp)); //Data for command will be in the payload

				switch(ntohl(tmp))
				{
					case CMD_PING:
						send_packet.header.pkt_type = PKT_TYPE_STATUS;
						send_packet.header.status = STATUS_OK;
						send_packet.header.p_length = sizeof(STATUS_KEEP_ALIVE);
						send_packet.header.timestamp = 0;
						tmp = STATUS_KEEP_ALIVE;
						tmp = htonl(tmp);
						memcpy(send_packet.payload.data, &tmp, sizeof(tmp));
						htonPacket(send_packet, buffer);
						sendto(client,buffer,sizeof(buffer),0,(struct sockaddr *)&cli_addr,&clilen);
						break;
				}
			}
			else if(recv_packet.header.pkt_type == PKT_TYPE_STATUS)
			{
				//We got a status update or statistic...
			}

		}
	}

}
