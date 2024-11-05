#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <sstream>
#include <winsock2.h>
#include <windows.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

enum CommandOpcode {
    CLEAR_DISPLAY_OPCODE,
    DRAW_PIXEL_OPCODE,
    DRAW_LINE_OPCODE,
    DRAW_RECTANGLE_OPCODE,
    FILL_RECTANGLE_OPCODE,
    DRAW_ELLIPSE_OPCODE,
    FILL_ELLIPSE_OPCODE
};

struct Command {
    const CommandOpcode opcode;
    Command(const CommandOpcode opcode) : opcode(opcode) {};
    virtual ~Command() {}
};

struct fillScreen : Command {
    const uint16_t color;
    fillScreen(const uint16_t color) : Command(CLEAR_DISPLAY_OPCODE), color(color) {};
};

struct DrawPixel : Command {
    const int16_t x0, y0;
    const int16_t newX, newY;
    const uint16_t color;
    DrawPixel(const int16_t x0, const int16_t y0, const int16_t newX, const int16_t newY, const uint16_t color) :
        Command(DRAW_PIXEL_OPCODE), x0(x0), y0(y0), newX(newX), newY(newY), color(color) {};
};

struct DrawLine : Command {
    const int16_t x0, y0, x1, y1;
    const uint16_t color;
    DrawLine(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1,
        const uint16_t color) :
        Command(DRAW_LINE_OPCODE), x0(x0), y0(y0), x1(x1), y1(y1), color(color) {}
};

struct DrawRectangle : Command {
    const int16_t x0, y0, x1, y1;
    const uint16_t color;
    DrawRectangle(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1,
        const uint16_t color) :
        Command(DRAW_RECTANGLE_OPCODE), x0(x0), y0(y0), x1(x1), y1(y1), color(color) {}
};

struct FillRectangle : Command {
    const int16_t x0, y0, x1, y1;
    const uint16_t color;
    FillRectangle(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1,
        const uint16_t color) :
        Command(FILL_RECTANGLE_OPCODE), x0(x0), y0(y0), x1(x1), y1(y1), color(color) {}
};

struct DrawEllipse : Command {
    const int16_t x0, y0, rx, ry;
    const uint16_t color;
    DrawEllipse(const int16_t x0, const int16_t y0, const int16_t rx, const int16_t ry,
        const uint16_t color) :
        Command(DRAW_ELLIPSE_OPCODE), x0(x0), y0(y0), rx(rx), ry(ry), color(color) {}
};

struct FillEllipse : Command {
    const int16_t x0, y0, rx, ry;
    const uint16_t color;
    FillEllipse(const int16_t x0, const int16_t y0, const int16_t rx, const int16_t ry,
        const uint16_t color) :
        Command(FILL_ELLIPSE_OPCODE), x0(x0), y0(y0), rx(rx), ry(ry), color(color) {}
};

class DisplayProtocol {
public:
    void parseCommand(const std::vector<uint8_t>& byteArray, Command*& command) {
        if (byteArray.empty()) {
            throw std::invalid_argument("Empty byte array");
        }
        uint8_t opcode = byteArray[0];
        switch (opcode) {
        case CLEAR_DISPLAY_OPCODE: {
            if (byteArray.size() != 3) {
                throw std::invalid_argument("Invalid parameters for clear display");
            }
            uint16_t color = parseColor(byteArray, 1);
            command = new fillScreen(color);
            break;
        }
        case DRAW_PIXEL_OPCODE: {
            if (byteArray.size() != 7) {
                throw std::invalid_argument("Invalid parameters for draw pixel");
            }
            int16_t x0 = parseInt16(byteArray, 1);
            int16_t y0 = parseInt16(byteArray, 3);
            uint16_t color = parseColor(byteArray, 5);

            int16_t newX = x0 + 50;
            int16_t newY = y0 + 50;
            command = new DrawPixel(x0, y0, newX, newY, color);
            break;
        }
        case DRAW_LINE_OPCODE: {
            if (byteArray.size() != 11) {
                throw std::invalid_argument("Invalid parameters for draw line");
            }
            int16_t x0 = parseInt16(byteArray, 1);
            int16_t y0 = parseInt16(byteArray, 3);
            int16_t x1 = parseInt16(byteArray, 5);
            int16_t y1 = parseInt16(byteArray, 7);
            uint16_t color = parseColor(byteArray, 9);
            command = new DrawLine(x0, y0, x1, y1, color);
            break;
        }
        case DRAW_RECTANGLE_OPCODE: {
            if (byteArray.size() != 11) {
                throw std::invalid_argument("Invalid parameters for draw rectangle");
            }
            int16_t x0 = parseInt16(byteArray, 1);
            int16_t y0 = parseInt16(byteArray, 3);
            int16_t x1 = parseInt16(byteArray, 5);
            int16_t y1 = parseInt16(byteArray, 7);
            uint16_t color = parseColor(byteArray, 9);
            command = new DrawRectangle(x0, y0, x1, y1, color);
            break;
        }
        case FILL_RECTANGLE_OPCODE: {
            if (byteArray.size() != 11) {
                throw std::invalid_argument("Invalid parameters for fill rectangle");
            }
            int16_t x0 = parseInt16(byteArray, 1);
            int16_t y0 = parseInt16(byteArray, 3);
            int16_t x1 = parseInt16(byteArray, 5);
            int16_t y1 = parseInt16(byteArray, 7);
            uint16_t color = parseColor(byteArray, 9);
            command = new FillRectangle(x0, y0, x1, y1, color);
            break;
        }
        case DRAW_ELLIPSE_OPCODE: {
            if (byteArray.size() != 11) {
                throw std::invalid_argument("Invalid parameters for draw ellipse");
            }
            int16_t x0 = parseInt16(byteArray, 1);
            int16_t y0 = parseInt16(byteArray, 3);
            int16_t rx = parseInt16(byteArray, 5);
            int16_t ry = parseInt16(byteArray, 7);
            uint16_t color = parseColor(byteArray, 9);
            command = new DrawEllipse(x0, y0, rx, ry, color);
            break;
        }
        case FILL_ELLIPSE_OPCODE: {
            if (byteArray.size() != 11) {
                throw std::invalid_argument("Invalid parameters for fill ellipse");
            }
            int16_t x0 = parseInt16(byteArray, 1);
            int16_t y0 = parseInt16(byteArray, 3);
            int16_t rx = parseInt16(byteArray, 5);
            int16_t ry = parseInt16(byteArray, 7);
            uint16_t color = parseColor(byteArray, 9);
            command = new FillEllipse(x0, y0, rx, ry, color);
            break;
        }
        default:
            throw std::invalid_argument("Invalid command opcode");
        }
    }

private:
    uint16_t parseColor(const std::vector<uint8_t>& byteArray, size_t start) {
        return (byteArray[start] << 8) | byteArray[start + 1];
    }

    int16_t parseInt16(const std::vector<uint8_t>& byteArray, size_t start) {
        return (byteArray[start] << 8) | byteArray[start + 1];
    }
};

HWND hwnd;
HDC hdc;
DisplayProtocol protocol;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Функція для малювання
void DrawCommand(Command* command) {
    switch (command->opcode) {
    case CLEAR_DISPLAY_OPCODE: {
        fillScreen* clearCommand = static_cast<fillScreen*>(command);
        HBRUSH brush = CreateSolidBrush(RGB((clearCommand->color >> 8) & 0xFF, clearCommand->color & 0xFF, 0));
        RECT rect = { 0, 0, 800, 600 }; // Припустимо, що розмір вікна 800x600
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        break;
    }
    case DRAW_PIXEL_OPCODE: {
        DrawPixel* pixelCommand = static_cast<DrawPixel*>(command);
        int pixelSize = 10; 
        HBRUSH brush = CreateSolidBrush(RGB((pixelCommand->color >> 8) & 0xFF, pixelCommand->color & 0xFF, 0));
        RECT rect = { pixelCommand->newX, pixelCommand->newY, pixelCommand->newX + pixelSize, pixelCommand->newY + pixelSize };
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        break;
    }
    case DRAW_LINE_OPCODE: {
        DrawLine* lineCommand = static_cast<DrawLine*>(command);
        HPEN pen = CreatePen(PS_SOLID, 1, RGB((lineCommand->color >> 8) & 0xFF, lineCommand->color & 0xFF, 0));
        SelectObject(hdc, pen);
        MoveToEx(hdc, lineCommand->x0, lineCommand->y0, NULL);
        LineTo(hdc, lineCommand->x1, lineCommand->y1);
        DeleteObject(pen);
        break;
    }
    case DRAW_RECTANGLE_OPCODE: {
        DrawRectangle* rectCommand = static_cast<DrawRectangle*>(command);
        HBRUSH brush = CreateSolidBrush(RGB((rectCommand->color >> 8) & 0xFF, rectCommand->color & 0xFF, 0));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB((rectCommand->color >> 8) & 0xFF, rectCommand->color & 0xFF, 0));
        SelectObject(hdc, brush);
        SelectObject(hdc, pen);
        Rectangle(hdc, rectCommand->x0, rectCommand->y0, rectCommand->x1, rectCommand->y1);
        DeleteObject(brush);
        DeleteObject(pen);
        break;
    }
    case FILL_RECTANGLE_OPCODE: {
        FillRectangle* fillRectCommand = static_cast<FillRectangle*>(command);
        HBRUSH brush = CreateSolidBrush(RGB((fillRectCommand->color >> 8) & 0xFF, fillRectCommand->color & 0xFF, 0));
        SelectObject(hdc, brush);
        RECT rect = { fillRectCommand->x0, fillRectCommand->y0, fillRectCommand->x1, fillRectCommand->y1 };
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        break;
    }
    case DRAW_ELLIPSE_OPCODE: {
        DrawEllipse* ellipseCommand = static_cast<DrawEllipse*>(command);
        HPEN pen = CreatePen(PS_SOLID, 1, RGB((ellipseCommand->color >> 8) & 0xFF, ellipseCommand->color & 0xFF, 0));
        SelectObject(hdc, pen);
        Ellipse(hdc, ellipseCommand->x0 - ellipseCommand->rx, ellipseCommand->y0 - ellipseCommand->ry,
            ellipseCommand->x0 + ellipseCommand->rx, ellipseCommand->y0 + ellipseCommand->ry);
        DeleteObject(pen);
        break;
    }
    case FILL_ELLIPSE_OPCODE: {
        FillEllipse* fillEllipseCommand = static_cast<FillEllipse*>(command);
        HBRUSH brush = CreateSolidBrush(RGB((fillEllipseCommand->color >> 8) & 0xFF, fillEllipseCommand->color & 0xFF, 0));
        SelectObject(hdc, brush);
        Ellipse(hdc, fillEllipseCommand->x0 - fillEllipseCommand->rx, fillEllipseCommand->y0 - fillEllipseCommand->ry,
            fillEllipseCommand->x0 + fillEllipseCommand->rx, fillEllipseCommand->y0 + fillEllipseCommand->ry);
        DeleteObject(brush);
        break;
    }
    }
}

void NetworkThread(SOCKET serverSocket) {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    std::vector<uint8_t> buffer(1024);

    while (true) {
        int recvSize = recvfrom(serverSocket, (char*)buffer.data(), buffer.size(), 0, (sockaddr*)&clientAddr, &clientAddrSize);
        if (recvSize == SOCKET_ERROR) {
            std::cerr << "Error receiving data" << std::endl;
            continue;
        }

        Command* command = nullptr;
        try {
            buffer.resize(recvSize);
            protocol.parseCommand(buffer, command);
            if (command) {
                // Відправка повідомлення для основного потоку для малювання
                PostMessage(hwnd, WM_USER + 1, 0, (LPARAM)command);
            }
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        buffer.resize(1024);
    }
}

int main() {
    // Ініціалізація WinSock
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 2);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cerr << "Error initializing WinSock" << std::endl;
        return -1;
    }

    // Налаштування сокета сервера
    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket" << std::endl;
        WSACleanup();
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1111); // Порт
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error binding socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    // Створення вікна
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"DrawingWindow";
    RegisterClass(&wc);

    hwnd = CreateWindow(wc.lpszClassName, L"Graphic Display", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, wc.hInstance, NULL);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    hdc = GetDC(hwnd);

    // Запуск мережевого потоку
    std::thread networkThread(NetworkThread, serverSocket);
    networkThread.detach();

    // Основний цикл обробки повідомлень
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_USER + 1) {
            Command* command = (Command*)msg.lParam;
            DrawCommand(command); 
            delete command; 
        }
    }

    ReleaseDC(hwnd, hdc);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
