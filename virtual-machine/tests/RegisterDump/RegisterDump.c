int volatile *const RdDump = (int *)0x0824;
int main() {
    *((char *)0x0800) = 'a';
    *RdDump;
    return 0;
}