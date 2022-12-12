#define _WIN32_WINNT 0x0602
#define STRICT
#include <windows.h>
#include <versionhelpers.h>
#include <shlobj.h>
#include <shobjidl.h>

int create_tree(const wchar_t *image_name, wchar_t **wallpaper_path)
{
    HANDLE image;
    HRSRC image_resource_loc;
    HGLOBAL image_resource;
    wchar_t *desktop_path;
    wchar_t *image_path;

    if (!SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Desktop,
                                        0,
                                        NULL,
                                        &desktop_path)))
    {
        return 1;
    }

    image_path = malloc(sizeof(WCHAR) *
                 (wcslen(desktop_path) +
                  wcslen(image_name) +
                  2));
    if (image_path == NULL) {
        CoTaskMemFree(desktop_path);
        return 1;
    }
    wsprintfW(image_path, L"%s\\%s", desktop_path, image_name);
    *wallpaper_path = image_path;

    CoTaskMemFree(desktop_path);

    image = CreateFileW(image_path,
                        FILE_GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (image == INVALID_HANDLE_VALUE) return 1;

    image_resource_loc = FindResourceA(NULL, "DEREVO", "JPG");
    if (image_resource_loc == NULL) return 1;

    image_resource = LoadResource(NULL, image_resource_loc);
    if (image_resource == NULL) return 1;

    if (!WriteFile(image,
                   LockResource(image_resource),
                   SizeofResource(NULL, image_resource_loc),
                   NULL,
                   NULL))
    {
        DeleteFileW(image_path);
        CloseHandle(image);
        return 1;
    }

    if (!CloseHandle(image)) return 1;

    return 0;
}

int set_wallpaper_win8(const wchar_t *image_path)
{
    struct IDesktopWallpaper *wp;

    if (!SUCCEEDED(CoInitialize(NULL))) {
        DeleteFileW(image_path);
        return 1;
    }

    if (!SUCCEEDED(CoCreateInstance(&CLSID_DesktopWallpaper,
                                    NULL,
                                    CLSCTX_ALL,
                                    &IID_IDesktopWallpaper,
                                    (void**)(&wp))))
    {
        CoUninitialize();
        DeleteFileW(image_path);
        return 1;
    }

    if (!SUCCEEDED(wp->lpVtbl->SetWallpaper(wp, NULL, image_path))) {
        CoUninitialize();
        DeleteFileW(image_path);
        return 1;
    }

    CoUninitialize();
    return 0;
}

int set_wallpaper(const wchar_t *image_path)
{
    if (!SystemParametersInfoW(SPI_SETDESKWALLPAPER,
                               0,
                               (void*)image_path,
                               SPIF_UPDATEINIFILE))
    {
        DeleteFileW(image_path);
        return 1;
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR cmdLine,
                   int nShowCmd)
{
    PWSTR image_path;
    int result;

#ifndef VIRUS
    if (MessageBoxA(NULL,
                    "Wise Mystical Tree is about to arrive.\n"
                    "Are you ready?\n"
                    "(All your wallpapers are to change)",
                    "Important event",
                    MB_YESNO | MB_ICONWARNING) != IDYES)
    {
        return 0;
    }
#endif

    image_path = NULL;

    if (create_tree(L"Your Honest Reaction.jpg", &image_path)) return 1;
    result = IsWindows8OrGreater() ?
        set_wallpaper_win8(image_path) : set_wallpaper(image_path);

    free(image_path);
    return result;
}
