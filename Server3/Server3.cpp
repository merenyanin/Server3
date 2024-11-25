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
    FILL_ELLIPSE_OPCODE,
    DRAW_TEXT_OPCODE,

    // Нові опкоди
    SET_ORIENTATION_OPCODE, // для встановлення орієнтації
    GET_WIDTH_OPCODE,       // для запиту ширини
    GET_HEIGHT_OPCODE       // для запиту висоти
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
struct Drawtext : Command {
    const int16_t x, y; 
    const uint16_t color; 
    const std::string text; 
    Drawtext(int16_t x, int16_t y, uint16_t color, const std::string& text)
        : Command(DRAW_TEXT_OPCODE), x(x), y(y), color(color), text(text) {}
};
struct SetOrientation : Command {
    const int orientation;  
    SetOrientation(int orientation) : Command(SET_ORIENTATION_OPCODE), orientation(orientation) {}
};

struct GetWidth : Command {
    GetWidth() : Command(GET_WIDTH_OPCODE) {}
};

struct GetHeight : Command {
    GetHeight() : Command(GET_HEIGHT_OPCODE) {}
};

int width = 800;
int height = 600;
int orientation = 0; 
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
        case DRAW_TEXT_OPCODE: {
            if (byteArray.size() < 7) {
                throw std::invalid_argument("Invalid parameters for draw text");
            }

            // Отримуємо координати
            int16_t x = parseInt16(byteArray, 1);
            int16_t y = parseInt16(byteArray, 3);
            uint16_t color = parseColor(byteArray, 5);

            // Отримуємо текст після координат
            std::string text(reinterpret_cast<const char*>(&byteArray[7]), byteArray.size() - 7);

            // Створення команди для малювання тексту
            command = new Drawtext(x, y, color, text);
            break;
        }
        case SET_ORIENTATION_OPCODE: {
            if (byteArray.size() <2) {
                throw std::invalid_argument("Invalid parameters for set orientation");
            }
            int orientation = (byteArray[1] << 8) | byteArray[2];
            if (orientation != 0 && orientation != 90 && orientation != 180 && orientation != 270) {
                throw std::invalid_argument("Invalid orientation value");
            }
            command = new SetOrientation(orientation);
            break;
        }
        case GET_WIDTH_OPCODE: {
            if (byteArray.size() != 1) {
                throw std::invalid_argument("Invalid parameters for get width");
            }
            command = new GetWidth();
            break;
        }
        case GET_HEIGHT_OPCODE: {
            if (byteArray.size() != 1) {
                throw std::invalid_argument("Invalid parameters for get height");
            }
            command = new GetHeight();
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


void drawCharacter(HDC hdc, char c, int x, int y, uint16_t color, float scale) {
    const int baseWidth = 40;
    const int baseHeight = 50;
    const int width = baseWidth * scale;
    const int height = baseHeight * scale;

    HPEN pen = CreatePen(PS_SOLID, 1, RGB((color >> 8) & 0xFF, color & 0xFF, 0));
    SelectObject(hdc, pen);

    int startX = x;
    int startY = y;

  
    switch (c) {
    case 'A': {
       
        MoveToEx(hdc, startX, startY + height, NULL);
        LineTo(hdc, startX + width / 2, startY);
        LineTo(hdc, startX + width, startY + height);
        MoveToEx(hdc, startX + width / 4, startY + height / 2, NULL);
        LineTo(hdc, startX + 3 * width / 4, startY + height / 2);
        break;
    }
    case 'H': {
      
        MoveToEx(hdc, startX, startY, NULL);
        LineTo(hdc, startX, startY + height);
        MoveToEx(hdc, startX + width, startY, NULL);
        LineTo(hdc, startX + width, startY + height);
        MoveToEx(hdc, startX, startY + height / 2, NULL);
        LineTo(hdc, startX + width, startY + height / 2);
        break;
    }
    case 'E': {
       
        MoveToEx(hdc, startX, startY, NULL);
        LineTo(hdc, startX, startY + height);
        MoveToEx(hdc, startX, startY, NULL);
        LineTo(hdc, startX + width, startY);
        MoveToEx(hdc, startX, startY + height / 2, NULL);
        LineTo(hdc, startX + width, startY + height / 2);
        MoveToEx(hdc, startX, startY + height, NULL);
        LineTo(hdc, startX + width, startY + height);
        break;
    }
    case 'L': {
      
        MoveToEx(hdc, startX, startY, NULL);
        LineTo(hdc, startX, startY + height);
        MoveToEx(hdc, startX, startY + height, NULL);
        LineTo(hdc, startX + width, startY + height);
        break;
    }
    case 'O': {
        MoveToEx(hdc, startX, startY + height, NULL);       
        LineTo(hdc, startX + width, startY + height);       
        LineTo(hdc, startX + width, startY);                
        LineTo(hdc, startX, startY);                       
        LineTo(hdc, startX, startY + height);             
        break;
    }
    case 'W': {
       
        int topLeftX = startX;
        int topRightX = startX + width;
        int bottomLeftX = startX + width / 4;
        int bottomMiddleX = startX + width / 2;
        int bottomRightX = startX + 3 * width / 4;
        int bottomY = startY + height;
        MoveToEx(hdc, topLeftX, startY, NULL);           
        LineTo(hdc, bottomLeftX, bottomY);                
        LineTo(hdc, bottomMiddleX, startY + height / 2);   
        LineTo(hdc, bottomRightX, bottomY);                
        LineTo(hdc, topRightX, startY);                   
        break;
    }
     case 'R': {
        MoveToEx(hdc, startX, startY, NULL);
        LineTo(hdc, startX, startY + height);
        MoveToEx(hdc, startX, startY, NULL);
        LineTo(hdc, startX + width / 2, startY);
        MoveToEx(hdc, startX + width / 2, startY, NULL);
        LineTo(hdc, startX + width / 2, startY + height / 2);
        MoveToEx(hdc, startX, startY + height / 2, NULL);
        LineTo(hdc, startX + width / 2, startY + height / 2);
        MoveToEx(hdc, startX + width / 12, startY + height / 2, NULL);
        LineTo(hdc, startX + width / 2 + width / 8, startY + height);
        break;
     }

     case 'D': {
       
          MoveToEx(hdc, startX, startY, NULL);
          LineTo(hdc, startX, startY + height);
  
          MoveToEx(hdc, startX, startY, NULL);
          LineTo(hdc, startX + width / 2, startY);
          MoveToEx(hdc, startX, startY + height, NULL);
          LineTo(hdc, startX + width / 2, startY + height);
          int radius = height / 1.8;
          int centerX = startX + width / 2;
          int centerY = startY + radius;
          Arc(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius, startX + width / 2, startY + height, startX + width / 2, startY);

          break;
      }

    }

    DeleteObject(pen);
}



// Функція для малювання
void DrawCommand(Command* command) {
    switch (command->opcode) {

    case CLEAR_DISPLAY_OPCODE: {
        fillScreen* clearCommand = static_cast<fillScreen*>(command);
        int r = ((clearCommand->color >> 11) & 0x1F) * 255 / 31;
        int g = ((clearCommand->color >> 5) & 0x3F) * 255 / 63;
        int b = (clearCommand->color & 0x1F) * 255 / 31;
        HBRUSH brush = CreateSolidBrush(RGB(r, g, b));
        RECT rect = { 0, 0, 800, 600 };
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
    case DRAW_TEXT_OPCODE: {
        Drawtext* textCommand = static_cast<Drawtext*>(command);
        int x = textCommand->x;
        int y = textCommand->y;

        for (char c : textCommand->text) {
            drawCharacter(hdc, c, x, y, textCommand->color, 0.5f); 
            x += 6;
        }
        break;
    }
    case SET_ORIENTATION_OPCODE: {
        SetOrientation* setOrientationCommand = static_cast<SetOrientation*>(command);
        orientation = setOrientationCommand->orientation;

        switch (orientation) {
        case 0:
            SetGraphicsMode(hdc, GM_ADVANCED); 
            ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
            break;

        case 90:
            SetGraphicsMode(hdc, GM_ADVANCED);
            {
                XFORM xform = { 0 };
                xform.eM11 = 0.0f;
                xform.eM12 = 1.0f;
                xform.eM21 = -1.0f;
                xform.eM22 = 0.0f;
                xform.eDx = height;
                xform.eDy = 0.0f;
                SetWorldTransform(hdc, &xform);
            }
            break;

        case 180:
            SetGraphicsMode(hdc, GM_ADVANCED);
            {
                XFORM xform = { 0 };
                xform.eM11 = -1.0f;
                xform.eM12 = 0.0f;
                xform.eM21 = 0.0f;
                xform.eM22 = -1.0f;
                xform.eDx = width;
                xform.eDy = height;
                SetWorldTransform(hdc, &xform);
            }
            break;

        case 270:
            SetGraphicsMode(hdc, GM_ADVANCED);
            {
                XFORM xform = { 0 };
                xform.eM11 = 0.0f;
                xform.eM12 = -1.0f;
                xform.eM21 = 1.0f;
                xform.eM22 = 0.0f;
                xform.eDx = 0.0f;
                xform.eDy = width;
                SetWorldTransform(hdc, &xform);
            }
            break;

        default:
            std::cerr << "Invalid orientation value: " << orientation << std::endl;
            break;
        }

        std::cout << "Orientation set to: " << orientation << " degrees" << std::endl;
        break;
    }


    case GET_WIDTH_OPCODE: {
        std::cout << "Display width: " << width << std::endl;
        break;
    }
    case GET_HEIGHT_OPCODE: {
        std::cout << "Display height: " << height << std::endl;
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
