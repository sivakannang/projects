/*
      gcc getifaddr.c -o getifaddr
      ./getifaddr 127.0.0.1

      Finds the network interface that owns an IP address, and shows
      network byte order versus host byte order for an IPv4 address.
*/
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

// Returns 1 if found, 0 if no interface has this address, -1 on error
int getDeviceNameByIp(const char *ip, char *device, size_t size)
{
	struct ifaddrs *ifaddr, *ifa;
	char host[NI_MAXHOST];
	int isFound = 0;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs failed");
		return -1;
	}

	// Walk through the linked list, keeping the head so it can be freed afterwards
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		int family, s;

		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;
		if (family != AF_INET && family != AF_INET6)
			continue;

		s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, sizeof(host), NULL, 0, NI_NUMERICHOST);
		if (s != 0) {
			printf("getnameinfo() failed: %s\n", gai_strerror(s));
			continue;
		}

		printf("\t%s address: <%s>\n", ifa->ifa_name, host);

		if (strcmp(host, ip) == 0) {
			snprintf(device, size, "%s", ifa->ifa_name);
			isFound = 1;
			break;
		}
	}

	freeifaddrs(ifaddr);

	return isFound;
}

int main(int argc, char *argv[])
{
	char device[48] = {0};
	char text[INET_ADDRSTRLEN] = {0};
	struct in_addr address;
	int found;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <ip address>\n", argv[0]);
		return 2;
	}

	found = getDeviceNameByIp(argv[1], device, sizeof(device));
	if (found == 1)
		printf("%s => %s\n", argv[1], device);
	else if (found == 0)
		printf("%s => no interface has this address\n", argv[1]);

	// s_addr is always in network byte order, and ntohl converts it for printing as a number
	if (inet_pton(AF_INET, "192.168.0.10", &address) == 1 && inet_ntop(AF_INET, &address, text, sizeof(text)) != NULL)
		printf("%s => host byte order value %u\n", text, ntohl(address.s_addr));

	return (found == 1) ? 0 : 1;
}
