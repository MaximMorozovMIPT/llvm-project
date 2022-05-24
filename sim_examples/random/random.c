#include "sim.h"

int main()
{
	int x = 0;
	int y = 0;
	for (y = 0; y < SIM_Y_SIZE; y++)
	{
		for (x = 0; x < SIM_X_SIZE; x++)
			simSetPixel(x, y, simRand());
		simFlush();
        simBkpt();
	}
	return 0;
}
