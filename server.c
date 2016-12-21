#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<sqlite3.h>
#include<sys/wait.h>

#define QUEUE_SIZE 16

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
  int i;
  for(i = 0; i < argc; i++)
  {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

  }
  printf("\n");
  return 0;
}

int main(int argc, char* argv[])
{
  int sockfd, newsockfd, clntaddrlen, portNo, idcount = 0, status;
  struct sockaddr_in servAddr, clntAddr;
  char msgbuffer[256], ipbuffer[256], id[20], sql_query[100];
  char *sender_ip;
  pid_t pid;

  portNo = atoi(argv[1]);

  sqlite3 *db;
  int rc;
  char *Zerrmsg, *sql;

  rc = sqlite3_open("Log.db", &db);
  if(rc)
  {
    fprintf(stderr, "Database could not be opened Successfully: %s\n", sqlite3_errmsg(db));
    exit(1);
  }
  else
  {
    fprintf(stderr, "Database opened Successfully\n");
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  clntaddrlen = sizeof(clntAddr);

  if(socket < 0)
    {
      printf("Socket Creation Failed\n");
      exit(1);
    }

  fprintf(stdout, "Socket Created\n");

  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(portNo);
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
      printf("Binding Failed\n");
      exit(1);
    }

  fprintf(stdout, "Binding Successful\n");

  if(listen(sockfd, QUEUE_SIZE) < 0)
    {
      printf("Listening Failed\n");
      exit(1);
    }

  fprintf(stdout, "I'm Listening\n");

  while(1)
    {
      newsockfd = accept(sockfd, (struct sockaddr*)&clntAddr, &clntaddrlen);

      if(newsockfd < 0)
	     {
	        printf("Accepting Failed\n");
	         exit(1);
	     }

       printf("Accepting Successful\n");

       ++idcount;
       sprintf(id, "%d", idcount);

       pid = fork();
      if(pid == 0)
	     {
            sender_ip = inet_ntoa(clntAddr.sin_addr);
            int n = recv(newsockfd, ipbuffer, sizeof(ipbuffer), 0);
            ipbuffer[n] = '\0';
            printf("IP address received: %s\n", ipbuffer);
            send(newsockfd, ipbuffer, n, 0);
	          int n1 = recv(newsockfd, msgbuffer, sizeof(msgbuffer), 0);
            msgbuffer[n1] = '\0';
            printf("Message Received: %s\n", msgbuffer);
            send(newsockfd, msgbuffer, n, 0);
            strcpy(sql_query, "INSERT INTO sender_receiver(ID,SENDER_IP, RECEIVER_IP, MESSAGE) VALUES(");
            strcat(sql_query, id);
            strcat(sql_query, ", ");
            strcat(sql_query, "'");
            strcat(sql_query, sender_ip);
            strcat(sql_query, "'");
            strcat(sql_query, ", ");
            strcat(sql_query, "'");
            strcat(sql_query, ipbuffer);
            strcat(sql_query, "'");
            strcat(sql_query, ", ");
            strcat(sql_query, "'");
            strcat(sql_query, msgbuffer);
            strcat(sql_query, "'");
            strcat(sql_query, ");");
            sql = sql_query;
            //printf("%s\n", sql);
            rc = sqlite3_exec(db, sql, callback, 0,  &Zerrmsg);
            if(rc != SQLITE_OK)
            {
              fprintf(stderr, "SQL Error: %s\n", Zerrmsg);
              sqlite3_free(Zerrmsg);
            }
            else
            {
              fprintf(stderr, "Data inserted Successfully into RECEIVER\n");
            }
            close(newsockfd);
            exit(0);
	     }
       else if(pid != 0)
       {
         wait(&status);
       }
    }
  sqlite3_close(db);
  close(sockfd);
  return 0;
}
