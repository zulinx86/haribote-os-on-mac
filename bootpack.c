void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void)
{
	int i;

	for (i = 0x000a0000; i <= 0x000affff; ++i)
		write_mem8(i, 15);

	for (;;)
		io_hlt();
}