#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shobjidl.h>
#include <filesystem>
#include <string>

inline std::filesystem::path
PathFromShellItem(IShellItem* item)
{
    if (!item)
        return {};
    
    PWSTR rawPath = nullptr;
    if (FAILED(item->GetDisplayName(SIGDN_FILESYSPATH,&rawPath)) || !rawPath)
        return {};

    std::filesystem::path result(rawPath);
    CoTaskMemFree(rawPath);

    return result;
}

inline bool ShowOpenMarkdownDialog(
    HWND owner,
    std::filesystem::path& outPath,
    bool& canceled)
{
    canceled = false;
    outPath.clear();

    IFileOpenDialog* dialog = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog));
    if (FAILED(hr) || !dialog)
        return false;

    FILEOPENDIALOGOPTIONS options = 0;
    if(SUCCEEDED(dialog->GetOptions(&options)))
        dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);

    COMDLG_FILTERSPEC filer[] = {{L""}};
}