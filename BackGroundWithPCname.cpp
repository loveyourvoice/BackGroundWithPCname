#include <windows.h>
#include <gdiplus.h>
#include <urlmon.h>
#include <shlobj.h>
#include <iostream>
#include <string>
#include <atlstr.h>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "urlmon.lib")

using namespace Gdiplus;
using namespace std;

void SetWallpaper(const wstring& imagePath) {
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)imagePath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

void CreateDirectoryIfNotExists(const wstring& path) {
    if (!CreateDirectory(path.c_str(), NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            wcerr << L"Failed to create directory: " << path << endl;
            exit(1);
        }
    }
}

class GdiPlusInit {
public:
    GdiPlusInit() {
        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }

    ~GdiPlusInit() {
        GdiplusShutdown(gdiplusToken);
    }

private:
    ULONG_PTR gdiplusToken;
};

int wmain() {
    // Инициализация GDI+
    GdiPlusInit gdiPlusInit;

    // Получаем разрешение экрана
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    wcout << L"Screen Resolution: " << screenWidth << L" x " << screenHeight << endl;

    // Создаем папку C:\Intel, если ее нет
    wstring folderPath = L"C:\\Intel";
    CreateDirectoryIfNotExists(folderPath);
    wcout << L"Directory exists or created: " << folderPath << endl;

    // Скачиваем изображение по ссылке
    wstring imageUrl = L"https://sun1-89.userapi.com/s/v1/ig2/lj1EF052SA6z4EkSVqFoCz_EpM4Q71zO5zKPen96rxjTvkNfuq246lmb7Jls3n0O8cSRYbZtKpcKIeylQzhldIVT.jpg?size=1024x1024&quality=95&crop=128,128,1024,1024&ava=1";
    wstring imagePath = folderPath + L"\\background.jpg";
    HRESULT hr = URLDownloadToFile(NULL, imageUrl.c_str(), imagePath.c_str(), 0, NULL);
    if (SUCCEEDED(hr)) {
        wcout << L"Image downloaded to: " << imagePath << endl;
    }
    else {
        wcerr << L"Failed to download image" << endl;
        return 1;
    }

    // Загружаем изображение
    unique_ptr<Bitmap> image(Bitmap::FromFile(imagePath.c_str()));
    if (image->GetLastStatus() != Ok) {
        wcerr << L"Failed to load image" << endl;
        return 1;
    }
    wcout << L"Image loaded successfully." << endl;

    int imageWidth = image->GetWidth();
    int imageHeight = image->GetHeight();
    wcout << L"Image Size: " << imageWidth << L" x " << imageHeight << endl;

    // Создаем новое изображение с разрешением экрана и заливаем его черным
    Bitmap wallpaper(screenWidth, screenHeight);
    Graphics graphics(&wallpaper);
    graphics.Clear(Color::Black);

    // Вычисляем позицию для центрирования изображения
    int x = max(0, (screenWidth - imageWidth) / 2);
    int y = max(0, (screenHeight - imageHeight) / 2);
    wcout << L"Drawing image at: X=" << x << L", Y=" << y << endl;
    graphics.DrawImage(image.get(), x, y, imageWidth, imageHeight);

    // Настройки текста
    wstring computerName(MAX_COMPUTERNAME_LENGTH + 1, L'\0');
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(&computerName[0], &size);
    computerName.resize(size);

    wstring text = L"PC Name: " + computerName;
    Font font(L"Arial", 36, FontStyleBold);
    SolidBrush brush(Color::Yellow);

    // Позиция текста
    RectF layoutRect;
    graphics.MeasureString(text.c_str(), -1, &font, PointF(0, 0), &layoutRect);
    wcout << L"Text Size: Width=" << layoutRect.Width << L", Height=" << layoutRect.Height << endl;

    // Координаты текста
    float textX = screenWidth - layoutRect.Width - 20;
    float textY = 20;
    wcout << L"Drawing text at: X=" << textX << L", Y=" << textY << endl;

    // Добавляем текст на изображение
    graphics.DrawString(text.c_str(), -1, &font, PointF(textX, textY), &brush);

    // Путь для сохранения нового изображения
    wstring wallpaperPath = folderPath + L"\\WallpaperWithPCName.jpg";
    CLSID clsid;
    CLSIDFromString(L"{557CF401-1A04-11D3-9A73-0000F81EF32E}", &clsid); // JPEG format
    wallpaper.Save(wallpaperPath.c_str(), &clsid, NULL);
    wcout << L"Wallpaper saved to: " << wallpaperPath << endl;

    // Устанавливаем обои
    SetWallpaper(wallpaperPath);

    wcout << L"Done." << endl;
    return 0;
}
