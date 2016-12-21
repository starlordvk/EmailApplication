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

int counter = 1;
//Function to send image
int send_image(int socket, char image_path[]){

   FILE *picture;
   int size, read_size, stat, packet_index;
   char send_buffer[10240], read_buffer[256];
   packet_index = 1;

   picture = fopen(image_path, "r");
   printf("Getting Picture Size\n");   

   if(picture == NULL) {
        printf("Error Opening Image File"); } 

   fseek(picture, 0, SEEK_END);
   size = ftell(picture);
   fseek(picture, 0, SEEK_SET);
   printf("Total Picture size: %d\n",size);

   //Send Picture Size
   printf("Sending Picture Size\n");
   write(socket, (void *)&size, sizeof(int));

   //Send Picture as Byte Array
   printf("Sending Picture as Byte Array\n");

   do { //Read while we get errors that are due to signals.
      stat = read(socket, &read_buffer , 255);
      printf("Bytes read: %i\n",stat);
   } while (stat < 0);

   printf("Received data in socket\n");
   printf("Socket data: %s\n", read_buffer);

   while(!feof(picture)) {
   //while(packet_index = 1){
      //Read from the file into our send buffer
      read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

      //Send data through our socket 
      do{
        stat = write(socket, send_buffer, read_size);  
      }while (stat < 0);

      printf("Packet Number: %d\n",packet_index);
      printf("Packet Size Sent: %d\n",read_size);     
      printf(" \n");
      printf(" \n");


      packet_index++;  

      //Zero out our send buffer
      bzero(send_buffer, sizeof(send_buffer));
     }
     return 0;
  }
//**********************************************************

//Function To Receive Message
/*int receive_image(int size,char imagearray[])
{ // Start function 

char verify = '1';
FILE *image;

char path[50] = "/home/gk/Documents/Test/ClientImages/img";
char path1[10];
sprintf(path1, "%d", counter++);
strcat(path, path1);
strcat(path, ".png");
image = fopen(path, "w");

if( image == NULL) {
printf("Error has occurred. Image file could not be opened\n");
return -1; }

//Write the currently read data into our image file
int write_size = fwrite(imagearray,1,size, image);
printf("Written image size: %i\n",write_size);
if(size !=write_size) 
 {
    printf("error in read write\n");
}
fclose(image);
printf("Image successfully Received!\n");
return 1;
}*/
//*********************************************************************
//*********************************************************************
  int receive_image(int socket)
{ // Start function 

int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
char imagearray[10241];

char verify = '1';
FILE *image;

//Find the size of the image
do{
stat = read(socket, &size, sizeof(int));
}while(stat<0);

printf("Packet received.\n");
printf("Packet size: %i\n",stat);
printf("Image size: %i\n",size);
printf(" \n");

char buffer[] = "Got it";

//Send our verification signal
do{
stat = write(socket, &buffer, sizeof(int));
}while(stat<0);

printf("Reply sent\n");
printf(" \n");
char path[80];
strcpy(path, "/home/gk/Documents/Test/ClientImages/img");
char path1[10];
sprintf(path1, "%d", counter);
strcat(path, path1);
strcat(path, ".png");
counter++;
image = fopen(path, "w");

if( image == NULL) {
printf("Error has occurred. Image file could not be opened\n");
return -1; }

//Loop while we have not received the entire file yet


int need_exit = 0;
struct timeval timeout = {10,0};

fd_set fds;
int buffer_fd, buffer_out;

while(recv_size < size) {
//while(packet_index < 2){

    FD_ZERO(&fds);
    FD_SET(socket,&fds);

    buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

    if (buffer_fd < 0)\
       printf("error: bad file descriptor set.\n");

    if (buffer_fd == 0)
       printf("error: buffer read timeout expired.\n");

    if (buffer_fd > 0)
    {
        do{
               read_size = read(socket,imagearray, 10241);
            }while(read_size <0);

            printf("Packet number received: %i\n",packet_index);
        printf("Packet size: %i\n",read_size);


        //Write the currently read data into our image file
         write_size = fwrite(imagearray,1,read_size, image);


         printf("Written image size: %i\n",write_size); 

             if(read_size !=write_size) {
                 printf("error in read write\n");    }


             //Increment the total number of bytes read
             recv_size += read_size;
             packet_index++;
             printf("Total received image size: %i\n",recv_size);
             printf(" \n");
             printf(" \n");
    }

}
  fclose(image);
  printf("Image successfully Received!\n");
  return 1;
  }
//************************************************************************
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
//****************************************************************************

int main(int argc, char* argv[])
{
  int sockfd, portNo, status;
  struct sockaddr_in servAddr;
  char ipbuffer[256], msgbuffer[256], pending_msg[100], time_buffer[100], 
  pending_timestamp[100], sql_query[100],semail_id[100],remail_id[100], subject[50];
  char *ipaddr, *message, *sql;
  char pending_subject[50], imagearray[10241], image_path[50];

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

  sleep(2);

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
    sleep(2);
    receive_image(sockfd);
    sleep(2);

    int m1 = recv(sockfd, pending_msg, sizeof(pending_msg), 0);
    pending_msg[m1] = '\0';
    printf("Message %d :  %s\n",i+1, pending_msg);
    sleep(2);

    int m3 = recv(sockfd, semail_id, sizeof(semail_id), 0);
    semail_id[m3] = '\0';
    printf("\nSent By: %s\n", semail_id);
    sleep(2);

    int m2=recv(sockfd,pending_timestamp,sizeof(pending_timestamp),0);
    pending_timestamp[m2]='\0';
    printf("\nSent on:  %s\n",pending_timestamp);
    printf("Received on: %s\n", time_buffer);
    sleep(2);

    int m4  = recv(sockfd, pending_subject, sizeof(pending_subject), 0);
    pending_subject[m4] = '\0';
    printf("Subject: %s\n", pending_subject);
    sleep(2);

    /*strcpy(sql_query, "UPDATE sender_receiver SET R_TIMESTAMP = '");
    strcat(sql_query, time_buffer);
    strcat(sql_query, "' WHERE MESSAGE = '");
    strcat(sql_query, pending_msg);
    strcat(sql_query, "';");*/
    printf("--------------------------------------------------------\n");
  }

  //Sending the Image
  printf("Enter the Image Path: ");
  scanf("%s", image_path);
  send_image(sockfd, image_path);

  sleep(2);

  //Enter the Email Id of the Receiving Client
  printf("Enter the Email Id of the Recipient Client: ");
  scanf("%s",remail_id);

  //Enter the Subject of the Message
  printf("Enter the Subject: ");
  scanf("%s", subject);

  //enter the message to send
  printf("Enter The Message To Send: ");
  scanf("%s", msgbuffer);

  sleep(2);
  //Sending the email_id to the server
  int n4 = send(sockfd, remail_id, sizeof(remail_id), 0);
  recv(sockfd, remail_id, sizeof(remail_id), 0);
  remail_id[n4] = '\0';
  printf("Email Id of Receiving Client: %s\n", remail_id);

  sleep(2);
  //Sending the subject to the server
  int n6 = send(sockfd, subject, sizeof(subject), 0);
  recv(sockfd, subject, sizeof(subject), 0);
  subject[n6] = '\0';
  printf("Subject: %s\n", subject);

  sleep(2);
  //Sending the Message to the server
  int n5 = send(sockfd, msgbuffer, sizeof(msgbuffer), 0);
  recv(sockfd, msgbuffer, sizeof(msgbuffer), 0);
  msgbuffer[n5] = '\0';
  printf("The Message: %s\n", msgbuffer);

  sleep(2);
  //Sending the Timestamp of Sent Message
  curtime;
  time(&curtime);
  sprintf(time_buffer, "%s", ctime(&curtime));
  int n1 = send(sockfd, time_buffer, sizeof(time_buffer), 0);

  
  close(sockfd);
  return 0;
}