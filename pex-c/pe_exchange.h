#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"
#include <sys/wait.h> 

#define LOG_PREFIX "[PEX]"
#define PRODUCT_MAXLEN 16
#define BUFFER_SIZE 256
#define MAX_INTEGER 999999

typedef struct order order;

struct order {
  int trader_id;
  char* order_id;
  char* product;
  int number;
  int price;
  long value;
  int amount;
  int type; // buy is 1, sell is 0
  int status; // finish is 1, not is 0, -1 is cancel
};

void sigchild_handle(int i);
void get_pid(int sig, siginfo_t *info, void *context);
int checkDisconnect(int trader, int* disconnect_trader, int disconnect_num);
int checkNum(char* num);
int checkProduct(char* product, char** products, int product_num);
int checkOrder(order** orders, char** message, int trader_id, int order_num);
int checkOrderId(order** orders, char** message, int trader_id, int order_num);
order* newOrder(int trader_id, char** message, int type);
order* modifyOrder(order* order, char* number, char* price);
order** matchOrderMsg(order** orders, int* child_pids, int* write_all, int order_num, int match_order, int* disconnect_trader, int disconnect_num, int match_id);
order** matchBuyOrder(order** orders, int* child_pids, int* write_all, int order_num, int* disconnect_trader, int disconnect_num, int match_id);
order** matchSellOrder(order** orders, int* child_pids, int* write_all, int order_num, int* disconnect_trader, int disconnect_num, int match_id);
void sendAllMsg(int trader_num, char* msg, int* pids, int* pipes, int except, int* disconnect_trader, int disconnect_num);
void printOrderMsg(int trader_id, char** message, int len);
void printOrderbook(int product_num, char** products, order** orders, int order_num);
void printPosition(int trader_num, int product_num, char** products, order** orders, int order_num);

#endif
