int volatile *const ConsoleWriteInt = (int *)0x0804;
int main() {
    *ConsoleWriteInt = 10;
    return 0;
}