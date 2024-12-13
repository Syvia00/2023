inline char scan_char() {
    char result;
    int addr = 0x0812;
    asm volatile("lw %[res], 0(%[adr])": [res]"=r"(result): [adr]"r"(addr));
    return result;
}

int volatile *const ConsoleWriteChar = (int *)0x0800;
int main() {
    char c;
    c = scan_char();
    *ConsoleWriteChar = c;
    return 0;
}