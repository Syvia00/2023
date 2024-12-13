inline int scan_int() {
    int result;
    int addr = 0x0816;
    asm volatile("lw %[res], 0(%[adr])": [res]"=r"(result): [adr]"r"(addr));
    return result;
}

int volatile *const ConsoleWriteInt = (int *)0x0804;
int main() {
    int i;
    i = scan_int();
    *ConsoleWriteInt = i;
    return 0;
}