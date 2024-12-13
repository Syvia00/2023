#include "pe_trader.h"

volatile int received = 0;
void siguser_handle(int i){
    if (i == SIGUSR1){
        received = 1;
    }
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    // order number
    int orderBuy = 0;
    // int orderSell = 0;

    // register signal handler
    signal(SIGUSR1, siguser_handle);

    // connect to named pipes
    int trader_id = strtol(argv[1],NULL,10);
    int len_fifo_exchange = strlen(FIFO_EXCHANGE) + argc;
	int len_fifo_trader = strlen(FIFO_TRADER) + argc;
    char* file_exchange = (char*)malloc(sizeof(char)*len_fifo_exchange);
	char* file_trader = (char*)malloc(sizeof(char)*len_fifo_trader);

    sprintf(file_exchange,FIFO_EXCHANGE,trader_id);
    sprintf(file_trader,FIFO_TRADER,trader_id);
    
    int read_exchange = open(file_exchange, O_RDONLY);
    int write_trader = open(file_trader, O_WRONLY);

    // check pipe open
    if (read_exchange < 0 || write_trader < 0){
        return 0;
    }

    int disconnect = 0;

    // event loop
    while(disconnect == 0){
        // wait for exchange update (MARKET message)
        signal(SIGUSR1, siguser_handle);
        pause();

        if(received == 1){
            // alloc memory for message from exchange and order information
            char* buffer = (char*)malloc(sizeof(char)*(BUFFER_SIZE+1));
            char** message = (char**)malloc(sizeof(char*)*6);

            //read from pipes and get message of exchange
            int size = read(read_exchange, buffer, BUFFER_SIZE);
            // split message to each word
            // delete ";" from message
            buffer[size-1] = '\0';
            char* token = strtok(buffer, " ");
            int a = 0;
            while (token != NULL){
                message[a] = token;
                token = strtok(NULL, " ");
                a++;
            }

            if(strcmp(message[1],"OPEN") == 0){
                disconnect = 0;
            }
            // recieve sell order from exchange
            else if (strcmp(message[1],"SELL") == 0){
                // if order number larger than 1000, then disconnect from exchange
                if (strtol(message[3], NULL, 10) >= 1000){
                    disconnect = 1;
                }
                else{
                    // send order;
                    int len_order = strlen(message[2])+strlen(message[3])+strlen(message[4])+32;
                    char* orderMsg = malloc(sizeof(char)*len_order);
                    sprintf(orderMsg,"BUY %d %s %s %s;", orderBuy, message[2],message[3],message[4]);
                    write(write_trader,orderMsg,strlen(orderMsg));
                    kill(getppid(),SIGUSR1);
                    // wait for exchange confirmation (ACCEPTED message)
                    free(orderMsg);
                }
            }
            else if(strcmp(message[0],"ACCEPTED") == 0){
                orderBuy ++;
            }
            free(buffer);
            free(message);
            
            
        }
        received = 0;
    }

    close(write_trader);
    close(read_exchange);

    // free memory
    free(file_exchange);
    free(file_trader);
    return 0;
}
