#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>

struct peer {
    int peer_id;
    char ip[20];
    int port;
};


struct login {
    int online;
    char username[1024];
    char password[1024];
    struct peer addrs_info;
};

struct block {
    char block_name[1024];
    struct peer loc;
    struct block *next;
};

struct file_node {
    char filename[1024];
    struct block *block_arr;
};



void display_online_peers(struct peer *peer_list) {
    printf("---------------------------------------------\n");
    printf("PEERS ONLINE:\n\n");
    for(int i=0; i<10; i++) {
        if(peer_list[i].peer_id != 0)
            printf("%d. PEER: %d IP: %s UDP_PORT: %d\n",
                    i, peer_list[i].peer_id, peer_list[i].ip, peer_list[i].port);
    }
    printf("---------------------------------------------\n");
}


int isPeer(struct peer peerlist[], struct peer peer) {
    // Determines if the new recieved peer is already in list or not.
    
    for(int i=0; i<9; i++)
        if(strcmp(peerlist[i].ip, peer.ip) == 0 && peerlist[i].port == peer.port)
            return 1;

    return 0;
}

int isNumber(char *buff) {
    // Determines if the string is a single digit.
    buff[strlen(buff)-1] = '\0';
    int num, flag = 0;
    for(int j=0; j<strlen(buff); j++) {
        if(buff[j] <= '9' && buff[j] >= '0')
            flag = 1;
        else {
            flag = 0;
            break;
        }
    }
    if(flag == 1) {
        sscanf(buff, "%d", &num);
        return num;
    }
    return -1;
}

void chat(int sock_id, struct peer other) {
    // A stupid chat function

    struct sockaddr_in other_struct, store; int len_store = sizeof(store);
    other_struct.sin_family = AF_INET;
    other_struct.sin_port = htons(other.port);
    inet_pton(AF_INET, other.ip, &(other_struct.sin_addr));

    char buff[1024]; 
    int n; 
    printf("---------------------------------------------\n");
    printf("---------------------------------------------\n");
    while(1) { 
        bzero(buff, 1024); 
        printf("Enter a message to send:\n");
        fgets(buff, 1024, stdin);

        int send_id = sendto(sock_id, buff, strlen(buff),
                             0, (struct sockaddr*)&other_struct, sizeof(other_struct));
        if(send_id == -1) {
            printf("Cannot send the chat message!\n");
            exit(1);
        }
        buff[strlen(buff)-1] = '\0';
        if(strcmp(buff, "exit") == 0) {
            printf("Quiting the chat session.\n");
            break;
        }

        bzero(buff, 1024);
        printf("Waiting for a reply...\n");

        int recv_id = recvfrom(sock_id, buff, sizeof(buff),
                                0 ,(struct sockaddr*)&store,&len_store);
        if(recv_id == -1) {
            printf("Cannot recieve the chat message:\n");
            exit(1);
        }

        printf("MSG: %s\n", buff);
        buff[strlen(buff)-1] = '\0';
        if(strcmp(buff, "exit") == 0) {
            printf("Quiting the chat session.\n");
            break;
        }
    }
    printf("---------------------------------------------\n"); 
    printf("---------------------------------------------\n");
}

void chat_recv(int sock_id) {
    // Another stupid chat function

    struct sockaddr_in store; int len_store = sizeof(store);
    
    char buff[1024]; 
    int n; 
    printf("---------------------------------------------\n");
    printf("---------------------------------------------\n");
    while(1) { 

        bzero(buff, 1024);
        printf("Waiting for a reply...\n");

        int recv_id = recvfrom(sock_id, buff, sizeof(buff),
                                0 ,(struct sockaddr*)&store,&len_store);
        if(recv_id == -1) {
            printf("Cannot recieve the chat message:\n");
            exit(1);
        }

        printf("MSG: %s\n", buff);
        buff[strlen(buff)-1] = '\0';
        if(strcmp(buff, "exit") == 0) {
            printf("Quiting the chat session.\n");
            break;
        }

        bzero(buff, 1024); 
        printf("Enter a message to send:\n");
        fgets(buff, 1024, stdin);

        int send_id = sendto(sock_id, buff, strlen(buff),
                             0, (struct sockaddr*)&store, sizeof(store));
        if(send_id == -1) {
            printf("Cannot send the chat message!\n");
            exit(1);
        }
        buff[strlen(buff)-1] = '\0';
        if(strcmp(buff, "exit") == 0) {
            printf("Quiting the chat session.\n");
            break;
        }
    }
    printf("---------------------------------------------\n"); 
    printf("---------------------------------------------\n");
}

long int findSize(const char *file_name)
{
    struct stat st;
    
    if(stat(file_name,&st)==0)
        return (st.st_size);
    else
        return -1;
}

int main(int args, char *argv[]) {
    if(args != 2) {
        printf("Insufficient data!\n");
        exit(1);
    }

    const int SERVER_PORT = atoi(argv[1]);
    int CLIENT_PORT;

    int sock_id = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_id == -1) {
        printf("Cannot create socket!\n");
        exit(1);
    }

    // Opening a UDP connection for the client at a predifiend port.
    int udp_sockid = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_sockid == -1) {
        printf("Cannot create UDP socket!\n");
        exit(1);
    }

    struct sockaddr_in self;
    int self_bind = -1;

    while(self_bind == -1) { // Loops till a valid udp socket is bind to the socket
        printf("Enter the UDP port number:\n");
        scanf("%d", &CLIENT_PORT);
        getchar();
        self.sin_addr.s_addr = INADDR_ANY;
        self.sin_port = htons(CLIENT_PORT);
        self.sin_family = AF_INET;

        self_bind = bind(udp_sockid, (struct sockaddr*)&self, sizeof(self));
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
  
    int connect_id = connect(sock_id, (struct sockaddr*)&addr, sizeof(addr));
    if(connect_id == -1) {
        printf("Cannot send connection request!\n");
        exit(1);
    }

    struct login self_details;
    fd_set fd_arr;
    FD_SET(sock_id, &fd_arr);
    FD_SET(0, &fd_arr);
    char buff[1024], reply[2];
    bzero(buff, 1024);

    struct peer peer_list[10]; // Peer list of individual client
    for(int i=0; i<10; i++)
        peer_list[i].peer_id = 0; 

    bzero(buff, 1024);
    int recv_id = recv(sock_id, buff, sizeof(buff), 0);
    if(recv_id == -1) {
        printf("Cannot recieve first server message:\n");
        exit(1);
    }
    buff[strlen(buff)-1] = '\0';
    puts(buff);
    
    fgets(reply, 2, stdin);
    getchar();

    int send_id = send(sock_id, reply, strlen(reply), 0);
    if(send_id == -1) {
        printf("Cannot send reply.\n");
        exit(1);
    }

    printf("Enter login details:\n");
    login:
        bzero(self_details.username, 1024);
        bzero(self_details.password, 1024);
        printf("Enter username:\n");
        fgets(self_details.username, 1024, stdin);
        printf("Enter password:\n");
        fgets(self_details.password, 1024, stdin);

        self_details.username[strlen(self_details.username)-1] = '\0';
        self_details.password[strlen(self_details.password)-1] = '\0';

    
        printf("Sending login details...\n");
        send_id = send(sock_id, &self_details, sizeof(self_details), 0);
        if(send_id == -1) {
            printf("Cannot send login details:\n");
            exit(1);
        }
        
        bzero(buff, 1024);
        recv_id = recv(sock_id, buff, sizeof(buff), 0);
        if(recv_id == -1) {
            printf("Cannot recieve greeting msg!\n");
            exit(1);
        }
        
        printf("FROM SERVER: %s\n", buff);

    if(strcmp(buff, "Login Successful") == 0 || strcmp(buff, "Registration successful") == 0) {
        printf("Sending the UDP socket info to server:\n");
        int send_udpid = send(sock_id, &self, sizeof(self), 0);
        if(send_udpid == -1) {
            printf("Cannot send the UDP socket info to the client:\n");
            exit(1);
        }
    }
    else if(strcmp(buff, "Login Failed") == 0){
        printf("Try again ...\n");
        goto login;
    }
    else if(strcmp(buff, "Username taken.") == 0) {
        printf("Username taken. Enter another username ...\n");
        goto login;
    }
    else if(strcmp(buff, "Already logged in.") == 0) {
        printf("Multiple logins not supported.\n");
        exit(0);
    }

    int maxfds = (sock_id > udp_sockid)?sock_id:udp_sockid;

    while(1) {
        FD_ZERO(&fd_arr);
        FD_SET(sock_id, &fd_arr);
        FD_SET(udp_sockid, &fd_arr);
        FD_SET(0, &fd_arr);

        int select_id = select(maxfds+1, &fd_arr, NULL, NULL, NULL);
        if(select_id == -1) {
            printf("Cannot select!\n");
            exit(1);
        }

        if(FD_ISSET(sock_id, &fd_arr)) {

            bzero(buff, 1024);
            recv_id = recv(sock_id, buff, sizeof(buff), MSG_DONTWAIT);
            if(recv_id == -1) {
                printf("Cannot recieve communication type from server!\n");
                exit(1);
            }

            send_id = send(sock_id, "ACK", strlen("ACK"), 0);
            if(send_id == -1) {
                printf("Cannot send acknowledgment!\n");
                exit(1);
            }

            if(strcmp(buff, "PeerUpdate") == 0) {
                printf("Recieved a peer list update:\n");

                recv_id = recv(sock_id, peer_list, 10*sizeof(struct peer), MSG_WAITALL);
                if(recv_id == -1) {
                    printf("Cannot recieve new peer update!\n");
                    exit(1);
                }
                printf("Recieved update\n");
                display_online_peers(peer_list);
            }

            else if(strcmp(buff, "BlockUpload") == 0) {
                
                char filename[256], buffer[BUFSIZ], cmnd[20];
                bzero(filename, 256);
        
                bzero(cmnd, 20);

                recv_id = recv(sock_id, filename, sizeof(filename), 0);
                if(recv_id == -1) {
                    printf("Cannot recieve filename!\n");
                    exit(1);
                }

                printf("RECIEVING BLOCK NAME: %s\n", filename);

                FILE* fp = fopen(filename, "w");
                if(fp == NULL) {
                    printf("Cannot create file!\n");
                    exit(1);
                }

                int recv_bytes = 0;  

                while( (recv_bytes = read(sock_id, buffer, BUFSIZ))> 0 ) {
                    fwrite(buffer, 1, recv_bytes, fp);
                    if(recv_bytes < BUFSIZ)
                        break;
                }

                if (recv_bytes < 0)
                    perror("Receiving");

                fclose(fp);
                printf("Block recieved.\n");

            }
        }

        if(FD_ISSET(udp_sockid, &fd_arr)) {
            
            char request[20];
            bzero(request, 20);
            struct sockaddr_in connecting_peer, src; int len_src = sizeof(src);
            recv_id = recvfrom(udp_sockid, request, sizeof(request),
                                 0, (struct sockaddr*)&src, &len_src);
            if(recv_id == -1) {
                printf("Cannot receive connecting peer details!\n");
                exit(1);
            }
            
            printf("Chat request recieved from:\n");
            printf("IP %s PORT %d\n", inet_ntoa(src.sin_addr), src.sin_port);

            chat_recv(udp_sockid);            
        }


        if(FD_ISSET(0, &fd_arr)) {
            bzero(buff, 100);
            fgets(buff, 1024, stdin);
            int input = isNumber(buff); 
            if(input  >= 0) {
                printf("Peer %d selected for communication.\n", input);
                printf("IP: %s PORT: %d\n", peer_list[input].ip, peer_list[input].port);
                printf("Initiating Chat ...\n");
 
                struct sockaddr_in peer_struct, store; int len_store = sizeof(store);
                peer_struct.sin_family = AF_INET;
                peer_struct.sin_addr.s_addr = inet_addr(peer_list[input].ip);
                peer_struct.sin_port = htons(peer_list[input].port);

                send_id = sendto(udp_sockid, "CHAT", sizeof("CHAT"), 0, 
                                    (struct sockaddr*)&peer_struct, sizeof(peer_struct));
                if(send_id == -1) {
                    printf("Cannot send peer connection request!\n");
                    exit(1);
                }

                chat(udp_sockid, peer_list[input]);

            }
            else if(strncmp(buff, "-s ", 3) == 0) {
                printf("Sending the message to the server...\n");
                if(strlen(buff) < 3)
                    printf("Message too small!\n");
                else {
                    char *msg = buff + 3;
                    int send_id = send(sock_id, msg, strlen(buff)-2, 0);
                    if(send_id == -1) {
                        printf("Cannot send message to the server!\n");
                        exit(1);
                    }

                    if(strcmp(msg, "exit") == 0) {
                        printf("Closing the peer..\n");
                        exit(0);
                    }
                    else if(strcmp(msg, "upload") == 0) {
                        char filename[20], filesize[256], sendbuffer[BUFSIZ];
                        int sent_bytes = 0;
                        bzero(filename, 20);
                        bzero(filesize, 256);
                        

                        printf("Enter file name:\n");
                        fgets(filename, 20, stdin);
                        filename[strlen(filename)-1] = '\0';

                        send_id = send(sock_id, filename, strlen(filename), 0);
                        if(send_id == -1) {
                            printf("Cannot send file name!\n");
                            exit(1);
                        }

                        FILE *f = fopen(filename, "r ");
                        if(f == NULL) {
                            printf("File doesn't exist!\n");
                            exit(1);
                        }

                        while(1) {
                            bzero(sendbuffer, BUFSIZ);
                            int nread = fread(sendbuffer, 1, BUFSIZ, f);
                            if(nread > 0) {
                                write(sock_id, sendbuffer, nread);
                            }
                            else if(nread == 0)
                                break;
                        }

                        printf("File uploaded.\n");

                    }
                    else if(strcmp(msg, "file-table") == 0) {
                        
                    }
                }
            }
            else if(strncmp(buff, "-sys ", 5) == 0) {
                printf("Executing system command ...\n");
                if(strlen(buff) < 5)
                    printf("Message too small!\n");
                else {
                    char *msg = buff + 5;
                    if(system(msg) == -1) {
                        printf("Cannot execute system command!\n");
                        exit(0);
                    }
                }
            }
        }
    }

    close(sock_id);
    close(udp_sockid);
    return 0;
}    