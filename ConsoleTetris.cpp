// ConsoleTetris.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>

using namespace std;

wstring tetromino[7];
const int PIECE_WIDTH = 4;
const wstring RENDER_CHARS = L" ABCDEFG=#";

void InitTetromino()
{
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"..X.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"..X.");
    tetromino[3].append(L"....");
    
    tetromino[4].append(L"....");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"....");

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");
}

enum RotateDegree
{
    ZeroDegree = 0,
    Degree90,
    Degree180,
    Degree270,
};

int Rotate(int x, int y, int rotateDegree)
{
    switch (rotateDegree % 4)
    {
    case 0: return y * 4 + x; // 0 degree
    case 1: return 12 + y - (x * 4); // 90 degree
    case 2: return 15 - (y * 4) - x; // 180 degree
    case 3: return 3 - y + (x * 4); // 270 degree
    default:
        break;
    }
    return 0;
}
int nScreenWidth = 80; // console screen size X, number of columns
int nScreenHeight = 30; // console screen size Y, number of rows
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;
const int pField_Offset = 2;

void InitPlayField()
{
    pField = new unsigned char[nFieldWidth * nFieldHeight];
    for (int x = 0; x < nFieldWidth; x++)
    {
        for (int y = 0; y < nFieldHeight; y++)
        {
            bool atBoundary = x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1;
            pField[y * nFieldWidth + x] = atBoundary ? 9 : 0;
        }
    }
}

// (nPosX, nPosY) is the coord of the top-left corner of the piece in the field
bool DoesPieceFit(int nTetromino, int rotation, int nPosX, int nPosY)
{
    for (int px = 0; px < PIECE_WIDTH; px++)
    {
        for (int py = 0; py < PIECE_WIDTH; py++)
        {
            // Get index of the rotated piece
            int pi = Rotate(px, py, rotation);

            // Get index into field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            bool WithinHorizontalBound = nPosX + px >= 0 && nPosX + px < nFieldHeight;
            bool WithinVerticalBound = nPosY + py >= 0 && nPosY + py < nFieldHeight;
            if (WithinHorizontalBound && WithinVerticalBound)
            {
                // collision detection
                if (tetromino[nTetromino][pi] !=  L'.' && pField[fi] != 0) {
                    return false; // fail on first hit
                }
            }
        }
    }
    return true;
}

int main()
{
    InitTetromino();
    InitPlayField();

    wchar_t* screen = new wchar_t[nScreenHeight * nScreenWidth];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
    {
        screen[i] = L' ';
    }
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);

    // Game Logic Variables
    DWORD dwBytesWritten = 0;

    int nCurrentPiece = 1;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    bool bKey[4]; // each bool indicate if the key is pressed. Here we only keep record of Left Arrow, Right Arrow, Down Arrow, and Z key
    const int RIGHT_KEY = 0;
    const int LEFT_KEY = 1;
    const int DOWN_KEY = 2;
    const int ROTATION_KEY = 3;
    bool bRotateKeyHold = false;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;
    int difficulty = 10;
    vector<int> lines_to_destroy;

    int nScore = 0;

    bool GameOver = false;
    while (!GameOver)
    {
        // GAME TIMING
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);

        // INPUT
        for (int k = 0; k < 4; k++)
        {
            // virtual key code: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
        }

        // GAME LOGIC
        nCurrentX -= (bKey[LEFT_KEY] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentX += (bKey[RIGHT_KEY] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (bKey[DOWN_KEY] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
        
        if (bKey[ROTATION_KEY])
        {
            nCurrentRotation += (!bRotateKeyHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateKeyHold = true;
        }
        else {
            bRotateKeyHold = false;
        }

        if (bForceDown)
        {
            nSpeedCounter = 0;
            nSpeedCounter++;
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
                nCurrentY++;
            else
            {
                // Lock the current piece in the field
                for (int px = 0; px < PIECE_WIDTH; px++)
                {
                    for (int py = 0; py < PIECE_WIDTH; py++)
                    {
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
                    }
                }

                nPieceCount++;
                if (nPieceCount % difficulty == 0)
                {
                    if (nSpeed > 10) nSpeed--;
                }

                // Check if we got any full lines
                for (int py = 0; py < PIECE_WIDTH; py++)
                {
                    // outmost layer is #s aka play field boundary, so the condition is "smaller than nFieldHeight-1"
                    if (nCurrentY + py < nFieldHeight - 1)
                    {
                        bool LineFull = true;
                        for (int px = 1; px < nFieldWidth - 1; px++)
                        {
                            LineFull &= pField[(nCurrentY + py) * nFieldWidth + px] != 0;
                        }
                        if (LineFull)
                        {
							for (int px = 1; px < nFieldWidth - 1; px++)
							{
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8; // index for '=' in RENDER_CHARS
							}
                            lines_to_destroy.push_back(nCurrentY + py);
                        }
                    }
                }

                nScore += 25;
                if (!lines_to_destroy.empty()) nScore += (1 << lines_to_destroy.size()) * 100;

                // Choose next piece
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;

                // if piece does not fit
                GameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
        }

        // RENDER OUTPUT


        // Draw Playing Field
        for (int x = 0; x < nFieldWidth; x++)
        {
            for (int y = 0; y < nFieldHeight; y++)
            {
                // we initialized the play field with 0s and 9s
                // 0 will render an empty space, 9 will render # which is the boundary
                screen[(y + pField_Offset) * nScreenWidth + (x + pField_Offset)] = RENDER_CHARS[pField[y * nFieldWidth + x]];
            }
        }

        // Draw Current Piece
        for (int px = 0; px < PIECE_WIDTH; px++)
        {
            for (int py = 0; py < PIECE_WIDTH; py++)
            {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                {
                    screen[(nCurrentY + py + pField_Offset) * nScreenWidth + (nCurrentX + px + pField_Offset)] = nCurrentPiece + 65; // 65 is letter 'A' in ASCII
                }
            }
        }

        // Draw Score
        swprintf_s(&screen[pField_Offset * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

        if (!lines_to_destroy.empty())
        {
			// Display Frame
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
            this_thread::sleep_for(400ms);
            for (int& y : lines_to_destroy)
            {
                for (int px = 1; px < nFieldWidth - 1; px++)
                {
                    for (int py = y; py > 0; py--)
                    {
                        // moving chars in each columns down
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    }
                    // clearing the very top row
                    pField[px] = 0;
                }

            }
            lines_to_destroy.clear();
        }

        // Display Frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
    }
    // Game Over
    CloseHandle(hConsole);
    cout << "Game Over!! Score: " << nScore << endl;
    system("pause");
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
