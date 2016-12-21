#include<stdio.h>
#include<stdlib.h>
#include<sqlite3.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/wait.h>
#include<time.h>

//callback is used for sqlite3 functionality
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
  int sockfd, portNo, status;
  struct sockaddr_in servAddr;
  char ipbuffer[256], msgbuffer[256], pending_msg[100], time_buffer[100], pending_timestamp[100], sql_query[100],semail_id[100],remail_id[100];
  char *ipaddr, *message, *sql;

  //Retrieving The command line arguments
  portNo = atoi(argv[1]);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd < 0)
    {
      printf("Socket Creation Failed\n");
      exit(1);
    }

  fprintf(stdout, "Socket Created\n");

  //Socket Creation
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(portNo);
  servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
      printf("Connection Failed\n");
      exit(1);
    }

  fprintf(stdout, "Connection Successful\n");


  //Entering the Email id:
  printf("Please Enter Your Email ID: ");
  scanf("%s",semail_id);

  //Sending the email id to server to login:

  int e1=send(sockfd,semail_id,sizeof(semail_id), 0);

  printf("Logging In...\n\n");

  sleep(5);

  //Receiving the number of messages pending
  int count_pending_msg;
  recv(sockfd,(int*)&count_pending_msg,sizeof(int),0);
  
  //Getting the Receiving timestamp
  time_t curtime;
  time(&curtime);
  sprintf(time_buffer, "%s", ctime(&curtime));

  //Displaying the Pending Messages and the Timestamp when it was sent
  fprintf(stdout, "There are %d messages unread: \n\n", count_pending_msg);
  
  int i;
  for(i = 0; i < count_pending_msg; i++)
  {
    int m1 = recv(sockfd, pending_msg, sizeof(pending_msg), 0);
    pending_msg[m1] = '\0';
    printf("Message %d :  %s\n\n",i+1, pending_msg);
    int m3 = recv(sockfd, semail_id, sizeof(semail_id), 0);
    semail_id[m3] = '\0';
    printf("\nSent By: %s\n", semail_id);
    int m2=recv(sockfd,pending_timestamp,sizeof(pending_timestamp),0);
    pending_timestamp[m2]='\0';
    printf("\nSent on:  %s\n",pending_timestamp);
    printf("Received on: %s\n", time_buffer);
    strcpy(sql_query, "UPDATE sender_receiver SET R_TIMESTAMP = '");
    strcat(sql_query, time_buffer);
    strcat(sql_query, "' WHERE MESSAGE = '");
    strcat(sql_query, pending_msg);
    strcat(sql_query, "';");
    printf("--------------------------------------------------------\n");
  }

  //Enter the Email Id of the Receiving Client
  printf("Enter the Email Id of the Recipient Client: ");
  scanf("%s",remail_id);

  //enter the message to send
  printf("Enter The Message To Send: ");
  scanf("%s", msgbuffer);


  //Sending the email_id to the server
  int n4 = send(sockfd, remail_id, sizeof(remail_id), 0);
  recv(sockfd, remail_id, sizeof(remail_id), 0);
  remail_id[n4] = '\0';
  printf("Email Id of Receiving Client: %s\n", remail_id);

  //Sending the Message to the server
  int n5 = send(sockfd, msgbuffer, sizeof(msgbuffer), 0);
  recv(sockfd, msgbuffer, sizeof(msgbuffer), 0);
  msgbuffer[n5] = '\0';
  printf("The Message: %s\n", msgbuffer);

  //Sending the Timestamp of Sent Message
  curtime;
  time(&curtime);
  sprintf(time_buffer, "%s", ctime(&curtime));
  int n1 = send(sockfd, time_buffer, sizeof(time_buffer), 0);

  close(sockfd);
  return 0;
}
