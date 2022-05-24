#include "sim.h"

int main()
{
	simSetPixel(0, SIM_Y_SIZE - 1, 0xFFFF0000);
	simFlush();
	return 0;
}
