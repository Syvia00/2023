/**
 * comp2017 - assignment 3
 * Hanzhang Peng
 * hpen0373
 */

#include "pe_exchange.h"

//////// signal handle //////////
// signal handle of check child process
volatile int disconnect = 0;
void sigchild_handle(int i){
	if (i == SIGCHLD){
		int stat;
		disconnect = wait(&stat);
    }
}

// signal handle for check signal from child process
volatile int received = 0;
volatile int sender_pid = -1;
void get_pid(int sig, siginfo_t *info, void *context){
	if (sig == SIGUSR1){
        received = 1;
		sender_pid = info->si_pid;
    }
}

//////// checking method ////////
// check trader is disconnect
int checkDisconnect(int trader, int* disconnect_trader, int disconnect_num){
	int result = 0;
	for (int i = 0; i < disconnect_num; i ++){
		if (disconnect_trader[i] == trader){
			result = 1;
			break;
		}
	}
	return result;
}

// check number in order is valid
int checkNum(char* num){
	int result = 0;
	if (strtol(num, NULL, 10) <= 999999 && strtol(num, NULL, 10) >= 1){
		result = 1;
	}
	return result;
}

// check product in product list
int checkProduct(char* product, char** products, int product_num){
	int result = 0;
	for (int i = 0; i < product_num; i++){
		if (strcmp(product, products[i]) == 0){
			result = 1;
			break;
		}
	}
	return result;
}

// check order exist
int checkOrder(order** orders, char** message, int trader_id, int order_num){
	int find = -1;
	for (int j = 0; j < order_num; j++){
		if (strcmp(orders[j]->order_id, message[1]) == 0 && orders[j]->trader_id == trader_id){
			if (orders[j]->status == 0){
				find = j;
				break;
			}
		}
	}
	return find;
}

// check orderid
int checkOrderId(order** orders, char** message, int trader_id, int order_num){
	int result = 0;
	int prev = 0;
	if(strtol(message[1], NULL, 10) == 0){
		prev= 1;
	}
	for (int j = 0; j < order_num; j++){
		if (orders[j]->trader_id == trader_id){
			if (orders[j]->order_id == message[1]){
				result = -1;
			}
			if (strtol(orders[j]->order_id, NULL, 10) == (strtol(message[1], NULL, 10) - 1)){
				prev = 1;
			}
		}
	}
	if (prev == 1 && result == 0){
		result = 1;
	}
	return result;
}

//////// new order ////////
// set new order
order* newOrder(int trader_id, char** message, int type){
	order* new = malloc(sizeof(order));
	new->trader_id = trader_id;
	new->order_id = malloc(sizeof(char)*BUFFER_SIZE);
	new->product = malloc(sizeof(char)*BUFFER_SIZE);
	memcpy(new->order_id, message[1],strlen(message[1])+1); 
	memcpy(new->product, message[2], strlen(message[2])+1);
	new->number = strtol(message[3], NULL, 10);
	new->price = strtol(message[4], NULL, 10);
	new->value = 0;
	new->amount = 0;
	new->type = type;
	new->status = 0;
	return new;
}

// modifies order
order* modifyOrder(order* order, char* number, char* price){
	order->number = strtol(number, NULL, 10);
	order->price = strtol(price, NULL, 10);
	return order;
}

/////// matching order /////////
static long total_fees = 0;

// successful match order
order** matchOrderMsg(order** orders, int* child_pids, int* write_all, int order_num, int match_order, int* disconnect_trader, int disconnect_num, int match_id){
	// change number of product in order and assgn amount of product
	if (orders[match_order]->number >= orders[match_id]->number){
		orders[match_order]->number = orders[match_order]->number - orders[match_id]->number;
		if (orders[match_id]->type == 1){
			orders[match_id]->amount = orders[match_id]->amount + orders[match_id]->number;
			orders[match_order]->amount = orders[match_order]->amount - orders[match_id]->number;
		}
		else{
			orders[match_id]->amount = orders[match_id]->amount - orders[match_id]->number;
			orders[match_order]->amount = orders[match_order]->amount + orders[match_id]->number;
		}
		orders[match_id]->number = 0;
		orders[match_id]->status = 1;
		if (orders[match_order]->number == orders[match_id]->number){
			orders[match_order]->status = 1;
			orders[match_order]->number = 0;
		}
	}
	else{
		orders[match_id]->number = orders[match_id]->number - orders[match_order]->number;
		if (orders[match_id]->type == 1){
			orders[match_id]->amount = orders[match_id]->amount + orders[match_order]->number;
			orders[match_order]->amount = orders[match_order]->amount - orders[match_order]->number;
		}
		else{
			orders[match_id]->amount = orders[match_id]->amount - orders[match_order]->number;
			orders[match_order]->amount = orders[match_order]->amount + orders[match_order]->number;
		}
		orders[match_order]->number = 0;
		orders[match_order]->status = 1;
	}
	// match order quantity and price
	int order_price = 0;
	int buyer = 0;
	int seller = 0;
	if (orders[match_id]->type == 1){
		order_price = orders[match_order]->price;
		buyer = match_id;
		seller = match_order;
	}
	else{
		order_price = orders[match_order]->price;
		seller = match_id;
		buyer = match_order;
	}
	int order_qty = abs(orders[match_order]->amount);

	// check if the trader is disconnect
	if (checkDisconnect(orders[buyer]->trader_id,disconnect_trader,disconnect_num) == 0){
		// first send fill message to buyer
		char* buyer_msg = malloc(sizeof(char)*BUFFER_SIZE);
		sprintf(buyer_msg,"FILL %s %d;", orders[buyer]->order_id, order_qty);
		write(write_all[orders[buyer]->trader_id],buyer_msg,strlen(buyer_msg));
		kill(child_pids[orders[buyer]->trader_id],SIGUSR1);
		free(buyer_msg);
	}
	if (checkDisconnect(orders[seller]->trader_id,disconnect_trader,disconnect_num) == 0){
		// send filll message to seller
		char* seller_msg = malloc(sizeof(char)*BUFFER_SIZE);
		sprintf(seller_msg,"FILL %s %d;", orders[seller]->order_id, order_qty);
		write(write_all[orders[seller]->trader_id],seller_msg,strlen(seller_msg));
		kill(child_pids[orders[seller]->trader_id],SIGUSR1);
		free(seller_msg);
	}

	// output match order in standard output
	long total = (long)order_qty*(long)order_price;
	long double f_fees = (long double)(total/(double)100);
	long fees = round(f_fees);
	if (orders[match_id]->type == 1){
		orders[match_id]->value = orders[match_id]->value - total-fees;
		orders[match_order]->value = orders[match_order]->value + total;
	}
	else{
		orders[match_id]->value = orders[match_id]->value + total-fees;
		orders[match_order]->value = orders[match_order]->value - total;
	}
	total_fees += fees;
	printf("%s Match: Order %s [T%d], ", LOG_PREFIX, orders[match_order]->order_id, orders[match_order]->trader_id);
	printf("New Order %s [T%d], ", orders[match_id]->order_id, orders[match_id]->trader_id);
	printf("value: $%ld, fee: $%ld.\n", total, fees);
	return orders;
}

// match new buy order
order** matchBuyOrder(order** orders, int* child_pids, int* write_all, int order_num, int* disconnect_trader, int disconnect_num, int match_id){
	int finish = 0;
	// find all match orders
	while(!finish){
		int lowest_price = MAX_INTEGER;
		int match_order = -1;
		// match with the lowest price order
		for (int i = 0; i < order_num-1; i++){
			if (orders[i]->status == 0 && orders[i]->type == 0){
				if (strcmp(orders[i]->product, orders[match_id]->product) == 0 && orders[i]->price <= orders[match_id]->price){
					if(lowest_price > orders[i]->price){
						lowest_price = orders[i]->price;
						match_order = i;
					}
				}
			}
		}
		// no match order than finish
		if(match_order == -1){
			finish = 1;
		}
		else{
			// change number of product in order
			if (orders[match_order]->number >= orders[match_id]->number){
				finish = 1;
			}

			// fill message to buyer and seller, also print in std output
			orders = matchOrderMsg(orders, child_pids, write_all, order_num, match_order, disconnect_trader, disconnect_num, match_id);
		}
	}
	return orders;
}

// match new sell order
order** matchSellOrder(order** orders, int* child_pids, int* write_all, int order_num, int* disconnect_trader, int disconnect_num, int match_id){
	int finish = 0;
	// find all match orders
	while(!finish){
		int highest_price = 0;
		int match_order = -1;
		// match with the lowest price order
		for (int i = 0; i < order_num-1; i++){
			if (orders[i]->status == 0 && orders[i]->type == 1){
				if (strcmp(orders[i]->product, orders[match_id]->product) == 0 && orders[i]->price >= orders[match_id]->price){
					if(highest_price < orders[i]->price){
						highest_price = orders[i]->price;
						match_order = i;
					}
				}
			}
		}
		// no match order than finish
		if(match_order == -1){
			finish = 1;
		}
		else{
			// change number of product in order
			if (orders[match_order]->number >= orders[match_id]->number){
				finish = 1;
			}
			// fill message to buyer and seller, also print in std output
			orders = matchOrderMsg(orders, child_pids, write_all, order_num, match_order, disconnect_trader, disconnect_num, match_id);
		}
	}
	return orders;
}

////////// message and report /////////
// send message to all
void sendAllMsg(int trader_num, char* msg, int* pids, int* pipes, int except, int* disconnect_trader, int disconnect_num){
	for (int i = 0; i < trader_num; i ++){
		if (i != except && checkDisconnect(i,disconnect_trader,disconnect_num) == 0){
			write(pipes[i],msg,strlen(msg));
			kill(pids[i],SIGUSR1);
		}
	}
}

// report order message
void printOrderMsg(int trader_id, char** message, int len){
	printf("%s [T%d] Parsing command: <", LOG_PREFIX, trader_id);
	for (int i = 0; i < len; i ++){
		printf("%s",message[i]);
		if (i != len - 1){
			printf(" ");
		}
	}
	printf(">\n");
}

// orderbook
void printOrderbook(int product_num, char** products, order** orders, int order_num){
	printf("%s\t--ORDERBOOK--\n", LOG_PREFIX);
	for (int i = 0; i < product_num; i ++){
		int buy_level = 0;
		int sell_level = 0;
		int* price_list = malloc(sizeof(int)*(order_num+1));
		price_list[0] = 0;
		int prev_price = 0;
		// find the price list sort in decrease order
		int finish = 0;
		while (!finish){
			int max_price = 0;
			int type = -1;
			for (int j = 0; j < order_num; j++){
				if (orders[j]->status == 0 && strcmp(orders[j]->product,products[i]) == 0){
					if (prev_price == 0 || orders[j]->price < prev_price){
						if (max_price < orders[j]->price){
							max_price = orders[j]->price;
							type = orders[j]->type;
						}
					}
				}
			}
			if (type == 0){
				price_list[buy_level+sell_level] = max_price;
				prev_price = max_price;
				sell_level ++;
			}
			else if (type == 1){
				price_list[buy_level+sell_level] = max_price;
				prev_price = max_price;
				buy_level ++;
			}
			else{
				finish = 1;
			}
		}
		printf("%s\tProduct: %s; Buy levels: %d; Sell levels: %d\n", LOG_PREFIX, products[i], buy_level, sell_level);
		for (int k = 0; k < buy_level+sell_level; k ++){
			int amount = 0;
			int num = 0;
			int type = -1;
			for (int l = 0; l < order_num; l ++){
				if (orders[l]->status == 0 && orders[l]->price == price_list[k] && strcmp(orders[l]->product,products[i]) == 0){
					if (type == -1){
						type = orders[l]->type;
						amount += orders[l]->number;
						num ++;
					}
					else if(orders[l]->type == type){
						amount += orders[l]->number;
						num ++;
					}
				}
			}
			if (type == 1){
				printf("%s\t\t%s",LOG_PREFIX, "BUY");
			}
			else{
				printf("%s\t\t%s",LOG_PREFIX, "SELL");
			}
			if (num > 1){
				printf(" %d @ $%d (%d orders)\n", amount, price_list[k], num);
			}
			else{
				printf(" %d @ $%d (%d order)\n", amount, price_list[k], num);
			}
		}
		free(price_list);
	}
}

// position
void printPosition(int trader_num, int product_num, char** products, order** orders, int order_num){
	printf("%s\t--POSITIONS--\n", LOG_PREFIX);
	for (int i = 0; i < trader_num; i ++){
		printf("%s\tTrader %d: ", LOG_PREFIX, i);
		for (int j = 0; j < product_num; j++){
			int num = 0;
			long cost = 0;
			for (int k = 0; k < order_num; k ++){
				if (orders[k]->status != -1 && orders[k]->trader_id == i && strcmp(orders[k]->product,products[j]) == 0){
					num += orders[k]->amount;
					cost += orders[k]->value;
				}
			}
			printf("%s %d ($%ld)", products[j], num, cost);
			if (j < product_num-1){
				printf(", ");
			}
			else{
				printf("\n");
			}
		}
	}
}

///////// main //////////
int main(int argc, char **argv) {

	// command line should contain at least 3 argument
	if (argc < 3){
		return 0;
	}

	// register signal handler
    // signal(SIGCHLD,sigchild_handle);

	// starting exchange market
	printf("%s Starting\n",LOG_PREFIX);

	///// read product file /////
	// get product information from product file
	int product_num = 0;
	char* line = (char*)malloc(sizeof(char)*(PRODUCT_MAXLEN+2));

	// open product file
    FILE* file = fopen(argv[1], "r");
	fgets(line, PRODUCT_MAXLEN, file);
	// number of all product at first line
	product_num = strtol(line,NULL,10);
	char** products = (char**)malloc(sizeof(char*)*(product_num+1));
	
	// store all products
	for (int i = 0; i < product_num; i++){
		fgets(line, PRODUCT_MAXLEN+2, file);
		line[strlen(line)-1] = '\0';
		products[i] = (char*)malloc((PRODUCT_MAXLEN+2)*sizeof(char));
		strcpy(products[i], line);
	}

	// print product information
	printf("%s Trading %d products:", LOG_PREFIX, product_num);
	for (int i = 0; i < product_num; i++){
		printf(" %s", products[i]);
	}
	printf("\n");

	// assign trader num(count the number of traders)
	int trader_num = 0;

	// check market opep
	int check_open = 0;

	// initial two pipe array, one store pipe from exchange to each trader, other for each trader to exchange
	int len_fifo_exchange = strlen(FIFO_EXCHANGE) + argc;
	int len_fifo_trader = strlen(FIFO_TRADER) + argc;
	char** exchange_pipes = (char**)malloc(sizeof(char*)*argc);
	char** trader_pipes = (char**)malloc(sizeof(char*)*argc);
	int* read_all = malloc(sizeof(int)*(argc-1));
	int* write_all = malloc(sizeof(int)*(argc-1));
	int* child_pids = malloc(sizeof(int*)*(argc-1));

	// initial pid for process
	pid_t pid;

	// trader start at third in command argument
	while (trader_num < argc-2){

		///// create two named pipes for each trader /////
		// alloc memory for pipes
		exchange_pipes[trader_num] = (char*)malloc(sizeof(char)*len_fifo_exchange);
		trader_pipes[trader_num] = (char*)malloc(sizeof(char)*len_fifo_trader);

		// set named pipe for each trader
		sprintf(exchange_pipes[trader_num],FIFO_EXCHANGE,trader_num);
		sprintf(trader_pipes[trader_num],FIFO_TRADER,trader_num);

		//check whether pipes exist
		if (!access(exchange_pipes[trader_num], F_OK)){
			unlink(exchange_pipes[trader_num]);
		}
		if (!access(trader_pipes[trader_num], F_OK)){
			unlink(trader_pipes[trader_num]);
		}

		// for each trader create two named pipe, and check pipe success create
		if (mkfifo(exchange_pipes[trader_num],0777) < 0 || mkfifo(trader_pipes[trader_num],0777) < 0){
			perror("[PEX] cannot make named pipe for trader\n");
			return(0);
		}
		else{
			printf("%s Created FIFO %s\n%s Created FIFO %s\n", LOG_PREFIX, exchange_pipes[trader_num], LOG_PREFIX, trader_pipes[trader_num]);
		}

		///// launch trader binary /////
		// trader binary name
		char* trader_binary = argv[trader_num+2];

		// create child process
		pid = fork();

		// child process
		if (pid == 0){
			char* str_num = malloc(sizeof(char)*BUFFER_SIZE);
			sprintf(str_num, "%d", trader_num);
			// launch trader binary as child process
			execl(trader_binary, trader_binary, str_num, (char*) NULL);
			free(str_num);
		}
		// parent process
		else if (pid > 0) {
			child_pids[trader_num] = pid;
			printf("%s Starting trader %d (%s)\n", LOG_PREFIX, trader_num, trader_binary);
			int read_trader = open(trader_pipes[trader_num], O_RDONLY);
			int write_exchange = open(exchange_pipes[trader_num], O_WRONLY);

			// store file point to array
			read_all[trader_num] = read_trader;
			write_all[trader_num] = write_exchange;

			// check connect of named pipe
			if (write_exchange < 0 || read_trader < 0){
				check_open = 1;
			}
			else{
				printf("%s Connected to %s\n", LOG_PREFIX, exchange_pipes[trader_num]);
				printf("%s Connected to %s\n", LOG_PREFIX, trader_pipes[trader_num]);
			}
		}
		// error
		else {
			fprintf(stderr, "[PEX] fork failed\n");
			return(0);
		}

		// increase num
		trader_num ++;
	}
	
	///// market open /////
	// after all binaries lauched and pipes open, send market open
	if (check_open == 0){
		char* msg = malloc(sizeof(char)*BUFFER_SIZE);
		sprintf(msg,"MARKET OPEN;");
		sendAllMsg(trader_num,msg,child_pids,write_all,-1, 0, 0);
		free(msg);
	}

	int disconnect_num = 0;
	int order_num = 0;
	int* disconnect_trader = malloc(sizeof(int)*(trader_num));
	order** orders = malloc(sizeof(order*)*MAX_INTEGER);

	///// Exchange order flow ///
	while(disconnect_num < trader_num){
		// recieve SIGUSR1 signal from trader
		struct sigaction sa;
		sa.sa_flags = SA_SIGINFO;
		sa.sa_sigaction = get_pid;
		sigaction(SIGUSR1, &sa, NULL);

		// recieve child process terminated from trader
		signal(SIGCHLD,sigchild_handle);
		pause();

		// while recieve signal from one trader
		if(received == 1){
			int trader_id = 0;
			// find the trader id of sender
			for (int i = 0; i < trader_num; i ++){
				if (child_pids[i] == sender_pid){
					trader_id = i;
					break;
				}
			}

			// check the order message could be accepted
			int accept = 0;

			// check the order type and to the match order
			int match_id = -1;;

			// read command from named pipe
			char* buffer = (char*)malloc(sizeof(char)*(BUFFER_SIZE+1));
			char** message = (char**)malloc(sizeof(char*)*7);

			//read from pipes and get message of trader
			int size = read(read_all[trader_id], buffer, BUFFER_SIZE);
			// split message to each word
			buffer[size-1] = '\0';
			char* token = strtok(buffer, " ");
			int a = 0;
			while (token != NULL){
				message[a] = token;
				token = strtok(NULL, " ");
				a++;
				if (a == 6){
					break;
				}
			}   
			
			// message for market and trader
			char* traderMsg = malloc(sizeof(char)*BUFFER_SIZE);
			char* msg = malloc(sizeof(char)*BUFFER_SIZE);

			// parsing order message
			printOrderMsg(trader_id,message,a);

			// check the command type
			// buy order
			if(strcmp(message[0],"BUY") == 0 && a == 5){
				// check order valid
				if (checkProduct(message[2],products,product_num) == 1 && checkNum(message[3]) == 1 && checkNum(message[4]) == 1 && checkOrderId(orders, message, trader_id, order_num) == 1){
					orders[order_num] = newOrder(trader_id,message,1);
					order_num ++;
					sprintf(traderMsg,"ACCEPTED %s;", message[1]);
					sprintf(msg,"MARKET %s %s %s %s;", message[0], message[2], message[3], message[4]);
					match_id = order_num-1;
				}
				else{accept = 1;}
			}

			// sell order
			else if(strcmp(message[0],"SELL") == 0 && a == 5){
				// check order valid
				if (checkProduct(message[2],products,product_num) == 1 && checkNum(message[3]) == 1 && checkNum(message[4]) == 1 && checkOrderId(orders, message, trader_id, order_num) == 1){
					orders[order_num] = newOrder(trader_id,message,0);
					order_num ++;
					sprintf(traderMsg,"ACCEPTED %s;", message[1]);
					sprintf(msg,"MARKET %s %s %s %s;", message[0], message[2], message[3], message[4]);
					match_id = order_num-1;
				}
				else{accept = 1;}
			}

			// amend order
			else if(strcmp(message[0],"AMEND") == 0 && a == 4){
				int current = checkOrder(orders, message, trader_id, order_num);
				if (current >= 0 && checkNum(message[2]) == 1 && checkNum(message[3]) == 1){
					orders[current] = modifyOrder(orders[current], message[2], message[3]);
					sprintf(traderMsg,"AMENDED %s;", message[1]);
					if (orders[current]->type == 1){
						sprintf(msg,"MARKET %s %s %s %s;", "BUY", orders[current]->product, message[2], message[3]);
					}
					else{
						sprintf(msg,"MARKET %s %s %s %s;", "SELL", orders[current]->product, message[2], message[3]);
					}
					match_id = current;
				}
				else{accept = 1;}
			}

			// cancel order
			else if(strcmp(message[0],"CANCEL") == 0 && a == 2){
				int current = checkOrder(orders, message, trader_id, order_num);
				if (current >= 0){
					orders[current]->status = -1;
					orders[current] = modifyOrder(orders[current], "0", "0");
					sprintf(traderMsg,"CANCELLED %s;", message[1]);
					if (orders[current]->type == 1){
						sprintf(msg,"MARKET %s %s %s %s;", "BUY", orders[current]->product, "0", "0");
					}
					else{
						sprintf(msg,"MARKET %s %s %s %s;", "SELL", orders[current]->product, "0", "0");
					}
				}
				else{accept = 1;}
			}

			// invalid message
			else{
				accept = 1;
			}

			if (accept == 1){
				sprintf(traderMsg,"INVALID;");
			}

			// send message back to trader
			write(write_all[trader_id],traderMsg,strlen(traderMsg));
			kill(sender_pid,SIGUSR1);

			if (accept == 0){
				// send to all other trader
				sendAllMsg(trader_num,msg,child_pids,write_all,trader_id, disconnect_trader, disconnect_num);

				// matching order
				// if order can be matched
				if (match_id >= 0){
					if (orders[match_id]->type == 1){
						orders = matchBuyOrder(orders, child_pids, write_all, order_num, disconnect_trader, disconnect_num, match_id);
					}
					else if (orders[match_id]->type == 0){
						orders = matchSellOrder(orders, child_pids, write_all, order_num, disconnect_trader, disconnect_num, match_id);
					}
				}

				// Report order book
				printOrderbook(product_num,products,orders,order_num);

				// report position of each trader
				printPosition(trader_num, product_num,products, orders, order_num);
			}

			free(msg);
			free(traderMsg);
			free(buffer);
			free(message);
		}

		if (disconnect != 0){
			// check disconnect
			for (int i = 0; i < trader_num; i++){
				if (disconnect == child_pids[i]){
					printf("%s Trader %d disconnected\n", LOG_PREFIX, i);
					disconnect_trader[disconnect_num] = i;
					close(read_all[i]);
					close(write_all[i]);
					disconnect_num ++;
					break;
				}
			}
		}

		// set signal value to 0
		received = 0;
		disconnect = 0;
	}

	// market close
	if (disconnect_num == trader_num){
		printf("%s Trading completed\n", LOG_PREFIX);
		printf("%s Exchange fees collected: $%ld\n",LOG_PREFIX, total_fees);
	}

	// free all memory
	free(disconnect_trader);
	free(child_pids);
	free(read_all);
	free(write_all);
	for (int i = 0; i < product_num; i++){
		free(products[i]);
	}
	free(products);
	free(line);
	for (int i = 0; i < argc-2; i++){
		unlink(exchange_pipes[i]);
		free(exchange_pipes[i]);
	}
	free(exchange_pipes);
	for (int i = 0; i < argc-2; i++){
		unlink(trader_pipes[i]);
		free(trader_pipes[i]);
	}
	free(trader_pipes);
	for (int i = 0; i < order_num; i++){
		free(orders[i]->order_id);
		free(orders[i]->product);
		free(orders[i]);
	}
	free(orders);

	return 0;
}