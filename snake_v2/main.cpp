#include <iostream>
#include <list>
#include <tchar.h>
#include <thread>
using namespace std;

#include <Windows.h>

int nScreenWidth = 240;
int nScreenHeight = 63;

struct sSnakeSegment
{
	int x;
	int y;
};

int main()
{
	// Create Screen Buffer
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	for (int i = 0; i < nScreenWidth*nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	while(1)
	{
		// Reset to known state
		list<sSnakeSegment> snake = { {60,15} };
		int nFoodX = (rand() % (nScreenWidth-1))+1;
		int nFoodY = (rand() % (nScreenHeight-3))+4;
		int nScore = 0;
		int nSnakeDirection = 3;
		bool bDead = false;
		bool bKeyLeft = false, bKeyRight = false, bKeyLeftOld = false, bKeyRightOld = false;

		while (!bDead)
		{
			// Frame Timing, compensate for aspect ratio of command line
			auto t1 = chrono::system_clock::now();
			while ((chrono::system_clock::now() - t1) < ((nSnakeDirection % 2 == 1) ? 120ms : 200ms))
			{
				// Get Input,
				bKeyRight = (0x8000 & GetAsyncKeyState((unsigned char)('\x27'))) != 0;
				bKeyLeft = (0x8000 & GetAsyncKeyState((unsigned char)('\x25'))) != 0;

				if (bKeyRight && !bKeyRightOld)
				{
					nSnakeDirection++;
					if (nSnakeDirection == 4) nSnakeDirection = 0;
				}

				if (bKeyLeft && !bKeyLeftOld)
				{
					nSnakeDirection--;
					if (nSnakeDirection == -1) nSnakeDirection = 3;
				}

				bKeyRightOld = bKeyRight;
				bKeyLeftOld = bKeyLeft;
			}

			// ==== Logic

			// Update Snake Position, place a new head at the front of the list in
			// the right direction
			switch (nSnakeDirection)
			{
			case 0: // UP
				snake.push_front({ snake.front().x, snake.front().y - 1 });
				break;
			case 1: // RIGHT
				snake.push_front({ snake.front().x + 1, snake.front().y });
				break;
			case 2: // DOWN
				snake.push_front({ snake.front().x, snake.front().y + 1 });
				break;
			case 3: // LEFT
				snake.push_front({ snake.front().x - 1, snake.front().y });
				break;
			}

			// Collision Detect Snake V Food
			if (snake.front().x == nFoodX && snake.front().y == nFoodY)
			{
				nScore++;
				while (screen[nFoodY * nScreenWidth + nFoodX] != L' ')
				{
					nFoodX = (rand() % (nScreenWidth-1))+1;
					nFoodY = (rand() % (nScreenHeight-4))+4;
				}

				for (int i = 0; i < 2; i++)
					snake.push_back({ snake.back().x, snake.back().y });
			}

			// Collision Detect Snake V World

			/*
			if (snake.front().x < 1 || snake.front().x >= nScreenWidth-1)
				bDead = true;
			if (snake.front().y < 3 || snake.front().y >= nScreenHeight-1)
				bDead = true;
            */

            //Pop from other side on Collision with Wall
            if (snake.front().x < 1)snake.front().x=nScreenWidth-1;
            else if (snake.front().x >= nScreenWidth-1) snake.front().x = 1;
            if (snake.front().y < 3)snake.front().y=nScreenHeight-1;
            else if (snake.front().y >= nScreenHeight-1) snake.front().y = 3;


			// Collision Detect Snake V Snake
			for (list<sSnakeSegment>::iterator i = snake.begin(); i != snake.end(); i++)
				if (i != snake.begin() && i->x == snake.front().x && i->y == snake.front().y)
					bDead = true;

			// Chop off Snakes tail :-/
			snake.pop_back();

			// ==== Presentation

			// Clear Screen
			for (int i = 0; i < nScreenWidth*nScreenHeight; i++) screen[i] = L' ';

			// Draw Stats & Border
			for (int i = 0; i < nScreenWidth; i++)
			{
				screen[i] = L'=';
				screen[2 * nScreenWidth + i] = L'=';
				screen[(nScreenHeight-1)*nScreenWidth+i] = L'=';
			}
			for (int i = 3; i < nScreenHeight-1; i++)
			{
				screen[i*nScreenWidth] = L'|';
				screen[i*nScreenWidth+(nScreenWidth-1)] = L'|';
				//screen[2 * nScreenWidth + i] = L'=';
				//screen[(nScreenHeight-1)*nScreenWidth+i] = L'=';
			}
			wsprintfW(&screen[nScreenWidth + 5], L" R A F S A N ' s - S N A K E ! !                      SCORE: %d", nScore);


			// Draw Snake Body
			for (auto s : snake)
				screen[s.y * nScreenWidth + s.x] = bDead ? L'x' : L'o';

			// Draw Snake Head
			screen[snake.front().y * nScreenWidth + snake.front().x] = bDead ? L'X' : L'O';

			// Draw Food
			screen[nFoodY * nScreenWidth + nFoodX] = L'%';

			if (bDead)
				wsprintfW(&screen[20 * nScreenWidth + 40], L"    PRESS 'SPACE' TO PLAY AGAIN    ");

			// Display Frame
			WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		}

		// Wait for space
		while ((0x8000 & GetAsyncKeyState((unsigned char)('\x20'))) == 0);
	}

	return 0;
}
