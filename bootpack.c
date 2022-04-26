void io_hlt(void);

void HariMain(void)
{
	int i;
	char *p;

	for (i = 0x000a0000; i <= 0x000affff; ++i) {
		p = (char *)i;
		*p = i & 0x0f;
	}

	for (;;)
		io_hlt();
}