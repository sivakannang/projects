// gcc rsa_client.c rsa.c -o rsa_client -lcrypto

#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>


struct ipek_req
{
	char msg_len[2];
	char msg_type[1];
	char msg_version[1];
	char key_set_id[5];
	char public_key[4096];
};

char privateKey[]="-----BEGIN RSA PRIVATE KEY-----\n"\
				   "MIIEowIBAAKCAQEAy8Dbv8prpJ/0kKhlGeJYozo2t60EG8L0561g13R29LvMR5hy\n"\
				   "vGZlGJpmn65+A4xHXInJYiPuKzrKUnApeLZ+vw1HocOAZtWK0z3r26uA8kQYOKX9\n"\
				   "Qt/DbCdvsF9wF8gRK0ptx9M6R13NvBxvVQApfc9jB9nTzphOgM4JiEYvlV8FLhg9\n"\
				   "yZovMYd6Wwf3aoXK891VQxTr/kQYoq1Yp+68i6T4nNq7NWC+UNVjQHxNQMQMzU6l\n"\
				   "WCX8zyg3yH88OAQkUXIXKfQ+NkvYQ1cxaMoVPpY72+eVthKzpMeyHkBn7ciumk5q\n"\
				   "gLTEJAfWZpe4f4eFZj/Rc8Y8Jj2IS5kVPjUywQIDAQABAoIBADhg1u1Mv1hAAlX8\n"\
				   "omz1Gn2f4AAW2aos2cM5UDCNw1SYmj+9SRIkaxjRsE/C4o9sw1oxrg1/z6kajV0e\n"\
				   "N/t008FdlVKHXAIYWF93JMoVvIpMmT8jft6AN/y3NMpivgt2inmmEJZYNioFJKZG\n"\
				   "X+/vKYvsVISZm2fw8NfnKvAQK55yu+GRWBZGOeS9K+LbYvOwcrjKhHz66m4bedKd\n"\
				   "gVAix6NE5iwmjNXktSQlJMCjbtdNXg/xo1/G4kG2p/MO1HLcKfe1N5FgBiXj3Qjl\n"\
				   "vgvjJZkh1as2KTgaPOBqZaP03738VnYg23ISyvfT/teArVGtxrmFP7939EvJFKpF\n"\
				   "1wTxuDkCgYEA7t0DR37zt+dEJy+5vm7zSmN97VenwQJFWMiulkHGa0yU3lLasxxu\n"\
				   "m0oUtndIjenIvSx6t3Y+agK2F3EPbb0AZ5wZ1p1IXs4vktgeQwSSBdqcM8LZFDvZ\n"\
				   "uPboQnJoRdIkd62XnP5ekIEIBAfOp8v2wFpSfE7nNH2u4CpAXNSF9HsCgYEA2l8D\n"\
				   "JrDE5m9Kkn+J4l+AdGfeBL1igPF3DnuPoV67BpgiaAgI4h25UJzXiDKKoa706S0D\n"\
				   "4XB74zOLX11MaGPMIdhlG+SgeQfNoC5lE4ZWXNyESJH1SVgRGT9nBC2vtL6bxCVV\n"\
				   "WBkTeC5D6c/QXcai6yw6OYyNNdp0uznKURe1xvMCgYBVYYcEjWqMuAvyferFGV+5\n"\
				   "nWqr5gM+yJMFM2bEqupD/HHSLoeiMm2O8KIKvwSeRYzNohKTdZ7FwgZYxr8fGMoG\n"\
				   "PxQ1VK9DxCvZL4tRpVaU5Rmknud9hg9DQG6xIbgIDR+f79sb8QjYWmcFGc1SyWOA\n"\
				   "SkjlykZ2yt4xnqi3BfiD9QKBgGqLgRYXmXp1QoVIBRaWUi55nzHg1XbkWZqPXvz1\n"\
				   "I3uMLv1jLjJlHk3euKqTPmC05HoApKwSHeA0/gOBmg404xyAYJTDcCidTg6hlF96\n"\
				   "ZBja3xApZuxqM62F6dV4FQqzFX0WWhWp5n301N33r0qR6FumMKJzmVJ1TA8tmzEF\n"\
				   "yINRAoGBAJqioYs8rK6eXzA8ywYLjqTLu/yQSLBn/4ta36K8DyCoLNlNxSuox+A5\n"\
				   "w6z2vEfRVQDq4Hm4vBzjdi3QfYLNkTiTqLcvgWZ+eX44ogXtdTDO7c+GeMKWz4XX\n"\
				   "uJSUVL5+CVjKLjZEJ6Qc2WZLl94xSwL71E41H4YciVnSCQxVc4Jw\n"\
				   "-----END RSA PRIVATE KEY-----\n";

int send_public_key_to_server(char* public_key,int encrypted_len)
{
	int sockfd,n;
	int i;
	struct sockaddr_in servaddr,cliaddr;
	unsigned char recvline[2048];
	struct ipek_req ipek_r;
	char send_buffer[4096] = {0};
	int temp_byte1 , temp_byte2 = 0;

	sockfd=socket(AF_INET,SOCK_STREAM,0);

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	//servaddr.sin_addr.s_addr= inet_addr("192.168.56.102");
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port=htons(13001);

	if ( connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("\nConnect Failed");
		return -1;
	}
	memset(&ipek_r,'\0',sizeof(ipek_r));

	ipek_r.msg_type[0] = 0x1;
	ipek_r.msg_version[0] = 0x1;
	ipek_r.key_set_id[0] = 0xFF;
	ipek_r.key_set_id[1] = 0xFF;
	ipek_r.key_set_id[2] = 0x98;
	ipek_r.key_set_id[3] = 0x76;
	ipek_r.key_set_id[4] = 0x54;
	memcpy(&(ipek_r.public_key),public_key,encrypted_len);
	memcpy(send_buffer+2,ipek_r.msg_type,1);
	memcpy(send_buffer+3,ipek_r.msg_version,1);
	memcpy(send_buffer+4,ipek_r.key_set_id,5);
	memcpy(send_buffer+9,ipek_r.public_key,encrypted_len);

	int	len = encrypted_len + 5 + 1 + 1 + 2;
	printf("Length is %d\n",len);

	temp_byte1 = (len - 2) & 0x00ff;
	temp_byte2 = (len -2) & 0xff00;
	temp_byte2 = temp_byte2 >> 8;


	ipek_r.msg_len[0] = send_buffer[0] = temp_byte2 ;
	ipek_r.msg_len[1] = send_buffer[1] = temp_byte1;

#if 1
	//sendto(sockfd,send_buffer,len,0,(struct sockaddr *)&servaddr,sizeof(servaddr));
	send(sockfd, send_buffer, len, 0);
	n=recv(sockfd,recvline,10000,0);
	close(sockfd);

	printf("Received %d bytes from Server\n",n);

	for(i = 0 ; i < n; i++)
	{
		printf("%.2X ", recvline[i]);
	}
	printf("\n");

	unsigned char decrypted[4098]={};

	int decrypted_length = private_decrypt(recvline+5,n-5,privateKey, decrypted);

	if(decrypted_length == -1)
	{
		printLastError("Private Decrypt failed ");
		exit(0);
	}


	printf("Decrypted Length =%d\n",decrypted_length);
	printf("Decrypted Text :\n",decrypted);

	for(i = 0; i < decrypted_length; i++)
		printf("%.2X ",decrypted[i]);

	printf("\n");

#endif

	return 0;
}

int main()
{
	char publicKey[]="-----BEGIN PUBLIC KEY-----\n"\
					  "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAy8Dbv8prpJ/0kKhlGeJY\n"\
					  "ozo2t60EG8L0561g13R29LvMR5hyvGZlGJpmn65+A4xHXInJYiPuKzrKUnApeLZ+\n"\
					  "vw1HocOAZtWK0z3r26uA8kQYOKX9Qt/DbCdvsF9wF8gRK0ptx9M6R13NvBxvVQAp\n"\
					  "fc9jB9nTzphOgM4JiEYvlV8FLhg9yZovMYd6Wwf3aoXK891VQxTr/kQYoq1Yp+68\n"\
					  "i6T4nNq7NWC+UNVjQHxNQMQMzU6lWCX8zyg3yH88OAQkUXIXKfQ+NkvYQ1cxaMoV\n"\
					  "PpY72+eVthKzpMeyHkBn7ciumk5qgLTEJAfWZpe4f4eFZj/Rc8Y8Jj2IS5kVPjUy\n"\
					  "wQIDAQAB\n"\
					  "-----END PUBLIC KEY-----\n";

	printf("key len is %d\n",sizeof(publicKey));
	send_public_key_to_server(publicKey, sizeof(publicKey)-1);

	return 0;
}
