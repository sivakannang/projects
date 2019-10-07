#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int getDeviceNameByIp(const char *ip, char *device);
int main(int argc, char *argv[])
{
	char device[48] = {0};
	char ipstr_local[48] = {0};
	struct in_addr nwInterfaceIpAddress = {0};
	getDeviceNameByIp(argv[1], device);
	printf("\n %s => %s", argv[1], device);
	nwInterfaceIpAddress.s_addr = htonl(inet_addr("192.168.0.10"));
	inet_ntop(AF_INET, &(nwInterfaceIpAddress.s_addr), ipstr_local, INET_ADDRSTRLEN);
	printf("\n %u => %s", nwInterfaceIpAddress.s_addr, ipstr_local);
	return 0;
}
int getDeviceNameByIp(const char *ip, char *device)
{

	struct ifaddrs *ifaddr, *ifa;
	int family, s, isFound = 0;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs failed");
		return -1;
	}


	// Walk through linked list, maintaining head pointer so we can free list later

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {

		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;


		if (family == AF_INET || family == AF_INET6) {

			memset(host, 0, sizeof(host));

			s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				return -1;
			}
			printf("\taddress: <%s>\n", host);
			if ( strcmp(host, ip) == 0)
			{
				strncpy(device, ifa->ifa_name, strlen(ifa->ifa_name));
				isFound = 1;
				break;
			}
		}
	}

	freeifaddrs(ifaddr);

	if ( !isFound)
		strncpy(device, "eth0", strlen("eth0"));

	return isFound;
}

