#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
// #define address "98:D3:51:FD:98:D9"

// compile -> gcc bluetooth.c -lbluetooth
// run     -> ./a.out

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
	{
		printf("Wrong argument\n");
		printf("Example usage: ./tune \"98:D3:A1:FD:65:C1\"\n");
        return 0;
	}
    struct sockaddr_rc addr = { 0 };
    int s, status;
    char dest[18];
    char input[128] = {0};

	memcpy(dest, argv[1], 17);
	printf("%s\n", dest);

    uint8_t buffer;
    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    // send a message
    if( status == 0 ) {
        printf("Start print string\n");
        printf("press 1 for printing only\n");
        printf("press 2 for print + send\n");
        char command;
        scanf("%c", &command);
        printf("number pressed: %c\n", command);
        if(command == '2'){
            scanf("%s", input);
            strcat(input, "\r\n");
            printf("number of byte = %d\n", (int)strlen(input));
            write(s, input, strlen(input));
        }
        while(1){
            if(!kbhit()){
                recv(s, &buffer, 1, MSG_DONTWAIT);
                if(buffer != 0){
                    printf("%c", buffer);
                    buffer = 0;
                }
            }else{
                getch();
                if(command == '2'){
                    printf("Type 'quit' to exit, 'dc' for reconnect\n");
                    scanf("%s", input);
                    if(strcmp(input, "quit") == 0)
                        break;
                    strcat(input, "\n");
                    write(s, input, strlen(input));
                    printf("number of byte = %d\t%s", (int)strlen(input), input);
                    strcpy(input, " ");
                }else{
                    printf("Type 'quit' to exit, 'dc' for reconnect\n");
                    scanf("%s", input);
                    if(strcmp(input, "quit") == 0)
                        break;
                    else if(strcmp(input, "dc") == 0){
                        close(s);
                        printf("Reconnecting ");
                        memcpy(dest, argv[1], 17);
                        printf("%s\n", dest);
                        status = 1;
                        s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
                        addr.rc_family = AF_BLUETOOTH;
                        addr.rc_channel = (uint8_t) 1;
                        str2ba( dest, &addr.rc_bdaddr );
                        status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
                        if(status == 0){
                            printf("Reconnection success\n");
                        }else{
                            printf("Reconnection unsuccess.  Please reconnect again\n");
                        }
                    }
                }
            }

        }
        printf("bye!!!\n");
    }

    if( status < 0 ) perror("uh oh");

    close(s);
    return 0;
}
