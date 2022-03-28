#define MAXREQ (4096*1024)
#define CRLF "\r\n"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "sem.c"
#include "bbuffer.c"

char *www_path;

void error(const char *msg) { perror(msg); exit(1); }

char *getPathOfGetRequest(char *buf) {
  char *path  = NULL;

  if(strtok(buf, " ")) {
    path = strtok(NULL, " ");
    if(path)
      path=strdup(path);
  }
  return(path);
}

char* catstr(const char *s1, const char *s2) {
  if(!s2 || !s1) return NULL;
  char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}

void *processRequest(void *ringBuffer) {
  char reqBuffer[MAXREQ];
  int n = 0;
  while(1) {
    int threadSockfd = bb_get(ringBuffer);
    bzero(reqBuffer,sizeof(reqBuffer));
    n = read (threadSockfd,reqBuffer,sizeof(reqBuffer)-1);

    if (n < 0) {
      error("ERROR reading from socket");
    }

    //Get path of request, ignore favico
    char *inPath = getPathOfGetRequest(reqBuffer);
    if(strstr(inPath, "/favicon.ico") != NULL) {
      close(threadSockfd);
      continue;
    }

    char *path = catstr(www_path, inPath);
    
    if(!strcmp(inPath, "/")){
      path = catstr(path, "index.html");
    }
    
    //Read file from requested path
    FILE *f;
    char *fileContent;
    char *responseCode;

    // Checks if provided paths contain '..' and that www_path is a part of path to file
    // This would correspond to trying to access something other than the webroot directory (as stated in part e)
    if(strstr(path, "..") == NULL && strstr(path, www_path) != NULL ) {  
      if(f = fopen(path, "r")) { 
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        fileContent = malloc(fsize + 1);
        fread(fileContent, fsize, 1, f);
        fclose(f);

        fileContent[fsize] = 0;
        
        responseCode = "200 OK";
      }
      else {
        fileContent = "<html><body><h1>Error 404 page not found</h1></body></html>";
        responseCode = "404 Not Found";
      }
    } else {
      fileContent = "<html><body><h1>Error 403 illegal access</h1></body></html>";
      responseCode = "403 Forbidden";
    }
    
    char *msg = malloc(MAXREQ);
    sprintf(msg, 
      "HTTP/0.9 %s\n"
      "Content-Type: text/html\n"
      "Content-Length: %d\n\n"
      "%s", responseCode, strlen(fileContent), fileContent);

    n = write (threadSockfd,msg,strlen(msg));
    
    if (n < 0) {
      error("ERROR writing to socket");
    } 
    
    if (!strcmp(responseCode, "200 OK")) {
      free(fileContent);
    }
    
    free(path);
    free(msg);
    close (threadSockfd);
    
  } 
}



int main(int argc, char *argv[]) {
  int port;
  int threads;
  int bufferslots;

  if(argc != 5) {
    error("to few args, try again with enough args");
  }

  www_path = argv[1];
  port = atoi(argv[2]);
  threads = atoi(argv[3]);
  bufferslots = atoi(argv[4]);
  
	int sockfd, newsockfd;
	socklen_t clilen; 
	struct sockaddr_in6 serv_addr, cli_addr;
	int n;
	sockfd = socket(PF_INET6, SOCK_STREAM, 0);

	if (sockfd < 0) error("ERROR opening socket");
  

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin6_family = AF_INET6;
  serv_addr.sin6_addr = in6addr_any;
  serv_addr.sin6_port = htons(port);

  int setsockFlag = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &setsockFlag, sizeof(setsockFlag)) < 0) 
    error("Error when setting socket options");

  if (bind(sockfd, (struct sockaddr *) &serv_addr,
    sizeof(serv_addr)) < 0) {
    error("ERROR on binding");
  }

  // create ring buffer with buffslot  
  BNDBUF *buff = bb_init(bufferslots);
  
  //create nr of threads
  pthread_t workerThreads[threads];
  int workerThreadsReturn[threads];

  for(int i = 0; i < threads; i++) {
    workerThreadsReturn[i] = pthread_create(&(workerThreads[i]), NULL, processRequest, buff);
  }


  listen(sockfd, bufferslots);
  while (1) {
    clilen = sizeof(cli_addr);
    newsockfd = accept (sockfd, (struct sockaddr *) &cli_addr,
      &clilen);

    if (newsockfd < 0) {
      error("ERROR on accept");
    }
    //push newsockfd to buffer
    bb_add(buff, newsockfd);
    
  }
}