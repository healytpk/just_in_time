#define X __asm__ volatile("nop");
#define Y X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X
#define Z Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y
void MyCode(void)
{
    __asm__ volatile("add $1, %rsp");
    __asm__ volatile("sub $1, %rsp");
    Z Z Z Z Z Z Z Z
    __asm__ volatile("ret");
}
