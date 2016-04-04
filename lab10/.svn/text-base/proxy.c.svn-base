/*
 * ID: 5130379024
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Andrew Carnegie, ac00@cs.cmu.edu 
 *     Harry Q. Bovik, bovik@cs.cmu.edu
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"
#include <semaphore.h>

#define CHUNK 0
#define LENGTH 1

sem_t mutex;

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void doit(int connfd);
ssize_t Rio_readnb_w(rio_t *rp, void *ptr, size_t nbytes); 
int Rio_writen_w(int fd, void *usrbuf, size_t n);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
void *thread(void *vargp);


/* 
 * main - Main routine for the proxy program 
 */


int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);
	/* Check arguments */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
		exit(0);
	}

	socklen_t clientlen;
	int listenfd,  port;
	int *connfd;
	struct sockaddr_in clientaddr;
	pthread_t tid;

	port = atoi(argv[1]);
	listenfd = Open_listenfd(port);
	sem_init(&mutex, 0, 1);

	while(1) {
		/* as a server */
		connfd = (int *)malloc(sizeof(int));
		clientlen = sizeof(clientaddr);
		*connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		pthread_create(&tid, NULL, thread, connfd);
	}
	exit(0);
}

void doit(int connfd)
{
	int clientfd, port, n;
	int msgMode = 3;
	int error = 0;          //if error, no need to read the content
	char buf[MAXLINE];
	char host[MAXLINE];		//hostname
	char path[MAXLINE];     //pathname
	char *ptr;             
	char *uri;
	rio_t rioC, rioS;

	rio_readinitb(&rioC, connfd);
	Rio_readlineb_w(&rioC, buf, MAXLINE);

	/* get uri */
	ptr = index(buf, ' ');
	uri = ++ptr;
	ptr = index(uri, ' ');
	if(ptr) {
		*ptr = '\0';
	}
	/* get uri end */

	char log_uri[MAXLINE];
	strcpy(log_uri, uri);
	parse_uri(uri, host, path, &port);

	clientfd = open_clientfd(host, port);
	rio_readinitb(&rioS,clientfd);

	/* send method */
	strcpy(buf, "GET \0");
	strcat(buf, "/");
	strcat(buf, path);
	strcat(buf, " ");
	strcat(buf, "HTTP/1.1\0");
	strcat(buf, "\r\n");

	n = strlen(buf);
	Rio_writen_w(clientfd, buf, n);

	/* send other messages from client to server */
	while((n = Rio_readlineb_w(&rioC, buf, MAXLINE)) != 0) {
		Rio_writen_w(clientfd, buf, n);
		if(!strcmp(buf, "\r\n")) { 
			break;
		}
	}

	/* prepare for log file */
	struct hostent *hp;
	struct sockaddr_in serveraddr;
	int size = 0;
	char logstring[MAXLINE];
	if((hp = gethostbyname(host)) == NULL) {
		printf("error\n");
		fflush(stdout);
	}
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
	serveraddr.sin_port = htons(port);
	
	/* deliver the content from the server to the client */
	int msgSize = 0;

	/* check the first line */
	n = Rio_readlineb_w(&rioS, buf, MAXLINE);
	Rio_writen_w(connfd, buf, n);
	if(strcmp(buf, "HTTP/1.1 200 OK\r\n")) error = 1;   // no need to read messages 

	while((n = Rio_readlineb_w(&rioS, buf, MAXLINE)) != 0) {	
		Rio_writen_w(connfd, buf, n);
		if(!strncasecmp(buf, "Content-Length: ", 15)) {   // get the length of the messages
			msgMode = LENGTH;
			ptr = buf+strlen("Content-Length: ");
			msgSize = strtol(ptr, &ptr, 10);
		}
		if(!strcmp(buf, "Transfer-Encoding: chunked\r\n")) msgMode = CHUNK;
		if(!strcmp(buf, "\r\n")) break;
		bzero(buf, n);
	}

	/* read the length of the message
	 * and convert it to type int using str2hex()
	 */
	if(error) { printf("no content\n"); fflush(stdout);}  // no content

	else if(msgMode == LENGTH) {   // Length known
		size += msgSize;
		while(1){
			n = (msgSize< MAXLINE-1)? msgSize: MAXLINE-1;
			if(n != 0){
				Rio_readnb_w(&rioS, buf,n );
				msgSize -= n;
				Rio_writen_w(connfd, buf, n);
				bzero(buf, MAXLINE);
			}
			if(msgSize <= 0) break;
		}
	}
	else {      // messages are sent by chunks
		while(1) {
			n = Rio_readlineb_w(&rioS, buf, MAXLINE);
			Rio_writen_w(connfd, buf, n);
			if(!strcmp(buf, "\r\n")) {
				n = Rio_readlineb_w(&rioS, buf, MAXLINE);
				Rio_writen_w(connfd, buf, n);
			}
			msgSize = strtol(buf, NULL, 16);
			size += msgSize;

			if(msgSize == 0) break;   // last chunk
		
			while(1) {   // receive the chunk 
						// and send it
 				n = (msgSize< MAXLINE-1)? msgSize: MAXLINE-1;
				if(n != 0) {	
					Rio_readnb_w(&rioS, buf,n );
					msgSize -= n;
					Rio_writen_w(connfd, buf, n);
					bzero(buf, MAXLINE);
				}
				if(msgSize <= 0) break;
			}
		}
	}
	/*
	 * before writing the log file
	 * lock
	 */
	sem_wait(&mutex);
	format_log_entry(logstring, &serveraddr, log_uri, size);
	FILE *fp = fopen("proxy.log", "a+");
	fprintf(fp, "%s\n", logstring);	
	fflush(fp);
	sem_post(&mutex);
}

/*
 * parse_uri - URI parser
 * 
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
	char *hostbegin;
	char *hostend;
	char *pathbegin;
	int len;

	if (strncasecmp(uri, "http://", 7) != 0) {
		hostname[0] = '\0';
		return -1;
	}

	/* Extract the host name */
	hostbegin = uri + 7;
	hostend = strpbrk(hostbegin, " :/\r\n\0");
	len = hostend - hostbegin;
	strncpy(hostname, hostbegin, len);
	hostname[len] = '\0';

	/* Extract the port number */
	*port = 80; /* default */
	if (*hostend == ':')   
		*port = atoi(hostend + 1);

	/* Extract the path */
	pathbegin = strchr(hostbegin, '/');
	if (pathbegin == NULL) {
		pathname[0] = '\0';
	}
	else {
		pathbegin++;	
		strcpy(pathname, pathbegin);
	}

	return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		char *uri, int size)
{
	time_t now;
	char time_str[MAXLINE];
	unsigned long host;
	unsigned char a, b, c, d;

	/* Get a formatted time string */
	now = time(NULL);
	strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

	/* 
	 * Convert the IP address in network byte order to dotted decimal
	 * form. Note that we could have used inet_ntoa, but chose not to
	 * because inet_ntoa is a Class 3 thread unsafe function that
	 * returns a pointer to a static variable (Ch 13, CS:APP).
	 */
	host = ntohl(sockaddr->sin_addr.s_addr);
	a = host >> 24;
	b = (host >> 16) & 0xff;
	c = (host >> 8) & 0xff;
	d = host & 0xff;


	/* Return the formatted log entry string */
	sprintf(logstring, "%s: %d.%d.%d.%d %s %d\n", time_str, a, b, c, d, uri, size);
}

ssize_t Rio_readnb_w(rio_t *rp, void *ptr, size_t nbytes) 
{
    ssize_t n;
  
    if ((n = Rio_readnb(rp, ptr, nbytes)) < 0)
	printf("Rio_readn error\n");
    return n;
}

int Rio_writen_w(int fd, void *usrbuf, size_t n) 
{
    if (rio_writen(fd, usrbuf, n) != n)
	printf("Rio_writen error");
	return n;
}

ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
		printf("Rio_readlineb error");
    return rc;
}

void *thread(void *vargp)
{
	int connfd = *((int *)vargp);
	pthread_detach(pthread_self());
	free(vargp);
	doit(connfd);
	close(connfd);
	return NULL;

}

