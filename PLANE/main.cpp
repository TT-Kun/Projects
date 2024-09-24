#define _CRT_SECURE_NO_WARNINGS 1
#include "Plane.h"
int main()
{
	initgraph(WIN_WIDTH, WIN_HEIGHT);
	bool game_status = true;
	while (game_status) {
		Welcome();
		RunGame();
	}
	return 0;
}