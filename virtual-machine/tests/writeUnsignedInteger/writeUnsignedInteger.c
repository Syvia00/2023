int volatile *const ConsoleWriteInt = (int *)0x0808;
int main() {
    *ConsoleWriteInt = 11;
    return 0;
}