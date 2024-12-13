int volatile *const DumpPC = (int *)0x0820;
int main() {
    *((char *)0x0800) = 'a';
    *DumpPC;
    return 0;
}
