#include <iostream>
#include <vector>
#include <thread>
#include <Windows.h>

using namespace std::literals;

/* =================SCREEN GLOBALS================== */
int g_ScreenWidth = 80;
int g_ScreenHeight = 30;
wchar_t* g_Screen = nullptr;
HANDLE g_Console = nullptr;
DWORD g_DwBytesWritten = 0;

/* =================GAME SETTINGS GLOBALS================== */
std::wstring g_Tetromino[7];
bool g_Keys[5];
int g_FieldWidth = 12;
int g_FieldHeight = 18;
unsigned char* g_Field = nullptr;

/* =================GAME LOGIC GLOBALS================== */
int g_CurrentX = g_FieldWidth / 2;
int g_CurrentY = 0;
int g_CurrentRotation = 0;
int g_CurrentPiece = 3;//rand() % 7;
int g_Speed = 20;
int g_SpeedTickCounter = 0;
int g_PiecesCounter = 0;
int g_Score = 0;
bool g_GameOver = false;
bool g_IsRotationPressesd = false;
bool g_IsFalling = false;
std::vector<int> g_Lines;

int Rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0: return py * 4 + px;			//   0 deg
	case 1: return 12 + py - (px * 4);	//  90 deg
	case 2: return 15 - (py * 4) - px;	// 180 deg
	case 3: return 3 - py + (px * 4);	// 270 deg
	}
	return 0;
}

bool DoesPieceFit(int tetromino, int rotation, int posX, int posY)
{
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			int pi = Rotate(px, py, rotation);

			int fi = (posY + py) * g_FieldWidth + (posX + px);

			if (posX + px >= 0 && posX + px < g_FieldWidth)
			{
				if (posY + py >= 0 && posY + py < g_FieldHeight)
				{
					if (g_Tetromino[tetromino][pi] == L'X' && g_Field[fi] != 0)
						return false;
				}
			}
		}
	}

	return true;
}

void CreateTetrominos()
{
	g_Tetromino[0].append(L"..X.");
	g_Tetromino[0].append(L"..X.");
	g_Tetromino[0].append(L"..X.");
	g_Tetromino[0].append(L"..X.");

	g_Tetromino[1].append(L"..X.");
	g_Tetromino[1].append(L".XX.");
	g_Tetromino[1].append(L".X..");
	g_Tetromino[1].append(L"....");

	g_Tetromino[2].append(L".X..");
	g_Tetromino[2].append(L".XX.");
	g_Tetromino[2].append(L"..X.");
	g_Tetromino[2].append(L"....");

	g_Tetromino[3].append(L"....");
	g_Tetromino[3].append(L".XX.");
	g_Tetromino[3].append(L".XX.");
	g_Tetromino[3].append(L"....");

	g_Tetromino[4].append(L"..X.");
	g_Tetromino[4].append(L".XX.");
	g_Tetromino[4].append(L"..X.");
	g_Tetromino[4].append(L"....");

	g_Tetromino[5].append(L"....");
	g_Tetromino[5].append(L".XX.");
	g_Tetromino[5].append(L"..X.");
	g_Tetromino[5].append(L"..X.");

	g_Tetromino[6].append(L"....");
	g_Tetromino[6].append(L".XX.");
	g_Tetromino[6].append(L".X..");
	g_Tetromino[6].append(L".X..");
}

void Init()
{
	CreateTetrominos();
	
	g_Field = new unsigned char[g_FieldWidth * g_FieldHeight];
	for (int x = 0; x < g_FieldWidth; x++)
	{
		for (int y = 0; y < g_FieldHeight; y++)
		{
			g_Field[y * g_FieldWidth + x] = (x == 0 || x == g_FieldWidth - 1 || y == g_FieldHeight - 1) ? 9 : 0;
		}
	}

	g_Screen = new wchar_t[g_ScreenWidth * g_ScreenHeight];
	for (int i = 0; i < g_ScreenWidth * g_ScreenHeight; i++)
	{
		g_Screen[i] = L' ';
	}
	
	g_Console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(g_Console);
	g_DwBytesWritten = 0;
}

void Input()
{
	for (int k = 0; k < 5; k++)
	{														  //  R   L   D Z esc
		g_Keys[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z\x1B"[k]))) != 0;
	}
}

void HandleInput()
{
	if (g_Keys[0] /* Right */)
	{
		if (DoesPieceFit(g_CurrentPiece, g_CurrentRotation, g_CurrentX + 1, g_CurrentY))
		{
			g_CurrentX++;
		}
	}

	if (g_Keys[1] /* Left */)
	{
		if (DoesPieceFit(g_CurrentPiece, g_CurrentRotation, g_CurrentX - 1, g_CurrentY))
		{
			g_CurrentX--;
		}
	}

	if (g_Keys[2] /* Down */)
	{
		if (DoesPieceFit(g_CurrentPiece, g_CurrentRotation, g_CurrentX, g_CurrentY + 1))
		{
			g_CurrentY++;
		}
	}

	if (g_Keys[3] /* Rotate */)
	{
		if (!g_IsRotationPressesd && DoesPieceFit(g_CurrentPiece, g_CurrentRotation + 1, g_CurrentX, g_CurrentY))
		{
			g_CurrentRotation++;
			g_IsRotationPressesd = true;
		}
	}
	else
	{
		g_IsRotationPressesd = false;
	}

	if (g_Keys[4] /* Esc */)
	{
		g_GameOver = true;
	}
}

void LockPiece()
{
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			if (g_Tetromino[g_CurrentPiece][py * 4 + px] == L'X')
			{
				g_Field[(g_CurrentY + py) * g_FieldWidth + (g_CurrentX + px)] = g_CurrentPiece + 1;
			}
		}
	}
}

void CheckLines()
{
	for (int py = 0; py < 4; py++)
	{
		if (g_CurrentY + py < g_FieldHeight - 1)
		{
			bool line = true;
			for (int px = 1; px < g_FieldWidth - 1; px++)
			{
				line &= g_Field[(g_CurrentY + py) * g_FieldWidth + px] != 0;
			}

			if (line)
			{
				for (int px = 1; px < g_FieldWidth - 1; px++)
				{
					g_Field[(g_CurrentY + py) * g_FieldWidth + px] = 8;
				}

				g_Lines.push_back(g_CurrentY + py);
			}
		}
	}
}

void DestroyLines()
{
	for (auto line : g_Lines)
	{
		for (int px = 1; px < g_FieldWidth - 1; px++)
		{
			for (int py = line; py > 0; py--)
			{
				g_Field[py * g_FieldWidth + px] = g_Field[(py - 1) * g_FieldWidth + px];
			}

			g_Field[px] = 0;
		}
	}

	g_Lines.clear();
}

void SpawnNewPiece()
{
	g_CurrentX = g_FieldWidth / 2;
	g_CurrentY = 0;
	g_CurrentRotation = 0;
	g_CurrentPiece = 3;//rand() % 7;
}

void GameOver()
{
	CloseHandle(g_Console);
	std::cout << "Game Over! Score: " << g_Score << std::endl;
	g_GameOver = true;
}

void Update()
{
	g_SpeedTickCounter += 1;
	if (g_SpeedTickCounter == g_Speed)
	{
		g_SpeedTickCounter = 0;
		g_IsFalling = true;
	}

	if (g_IsFalling)
	{
		if (DoesPieceFit(g_CurrentPiece, g_CurrentRotation, g_CurrentX, g_CurrentY + 1))
		{
			g_CurrentY++;
		}
		else
		{
			LockPiece();
			
			g_PiecesCounter++;
			if (g_PiecesCounter % 10 == 0)
			{
				if (g_Speed >= 10)
				{
					g_Speed--;
				}
			}

			g_Score += 25;
			if (!g_Lines.empty())
			{
				g_Score += (1 << g_Lines.size()) * 100;
			}

			CheckLines();
			SpawnNewPiece();

			if (!DoesPieceFit(g_CurrentPiece, g_CurrentRotation, g_CurrentX, g_CurrentY))
			{
				GameOver();
			}
		}

		g_IsFalling = false;
	}
}

void Draw()
{
	for (int x = 0; x < g_FieldWidth; x++)
	{
		for (int y = 0; y < g_FieldHeight; y++)
		{
			g_Screen[(y + 2) * g_ScreenWidth + (x + 2)] = L" ABCDEFG=#"[g_Field[y * g_FieldWidth + x]];
		}
	}

	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			if(g_Tetromino[g_CurrentPiece][Rotate(px, py, g_CurrentRotation)] == L'X')
				g_Screen[(g_CurrentY + py + 2) * g_ScreenWidth + (g_CurrentX + px + 2)] = g_CurrentPiece + 65;
		}
	}

	swprintf_s(&g_Screen[2 * g_ScreenWidth + g_FieldWidth + 6], 16, L"SCORE: %8d", g_Score);

	if (!g_Lines.empty())
	{
		WriteConsoleOutputCharacter(g_Console, g_Screen, g_ScreenWidth * g_ScreenHeight, { 0, 0 }, &g_DwBytesWritten);
		std::this_thread::sleep_for(400ms);

		DestroyLines();
	}

	WriteConsoleOutputCharacter(g_Console, g_Screen, g_ScreenWidth * g_ScreenHeight, { 0, 0 }, &g_DwBytesWritten);
}

void Destroy()
{
	delete[] g_Screen;
	delete[] g_Field;
}

int main()
{
	Init();

	while (!g_GameOver)
	{
		std::this_thread::sleep_for(50ms);

		Input();
		HandleInput();
		Update();
		Draw();
	}

	system("pause");

	Destroy();

	return 0;
}