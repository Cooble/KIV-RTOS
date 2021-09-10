#include <stdstring.h>
#include <stdfile.h>

#include <drivers/bridges/uart_defs.h>

int main(int argc, char** argv)
{
	volatile int i;

	const char* msg = "\r\nRandom number: ";

	uint32_t f = open("DEV:uart/0", NFile_Open_Mode::Read_Write);
    uint32_t rndf = open("DEV:trng", NFile_Open_Mode::Read_Only);

	TUART_IOCtl_Params params;
	params.baud_rate = NUART_Baud_Rate::BR_115200;
	params.char_length = NUART_Char_Length::Char_8;
	ioctl(f, NIOCtl_Operation::Set_Params, &params);

    uint32_t rdbuf;
    char numbuf[16];

    uint32_t srf = open("DEV:sr", NFile_Open_Mode::Write_Only);

    write(srf, "\x00", 1);
    for (i = 0; i < 0x80000; i++)
			;
    write(srf, "\xFF", 1);

    close(srf);

    srf = open("DEV:segd", NFile_Open_Mode::Write_Only);
    write(srf, "4", 1);

	int p = 10;

	while (true)
	{
		write(f, msg, strlen(msg));

        read(rndf, reinterpret_cast<char*>(&rdbuf), 4);

		// timto jen muzeme overit, ze nam zasobnik umele nebobtna vlivem spatne implementace context switche
		//asm volatile("mov %0, sp\n\t" : "=r" (rdbuf) );

        bzero(numbuf, 16);
        itoa(rdbuf, numbuf, 16);

        write(f, numbuf, strlen(numbuf));

		for (i = 0; i < 0x80000; i++)
			;

		// nasledujici instrukce v uzivatelskem rezimu neprojde
		// v systemovem rezimu (tedy nez byly procesy presunuty do "userspace") by to zakazalo preruseni
		// a mj. ukradlo vsechen procesorovy cas pro tento proces
		//asm volatile("cpsid i\r\n");
	}

	close(f);

    return 0;
}