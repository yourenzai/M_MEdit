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

    COMDLG_FILTERSPEC filer[] = {{L"Markdown 文件 (*.md)", L"*.md"}};

    dialog->SetFileTypes(ARRAYSIZE(filer),filer);
    dialog->SetFileTypeIndex(1);
    dialog->SetDefaultExtension(L"md");
    dialog->SetTitle(L"打开 Markdown 文件");

    hr = dialog->Show(owner);

    bool ok = false;
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        canceled = true;
    }
    else if (SUCCEEDED(hr))
    {
        IShellItem* item = nullptr;
        if (SUCCEEDED(dialog->GetResult(&item)))
        {
            outPath = PathFromShellItem(item);
            item->Release();
            ok = !outPath.empty();
        }
    }

    dialog->Release();
    return ok;
}

inline bool ShowSaveMarkdownDidlog(
    HWND owner,
    const std::wstring& defaultName,
    std::filesystem::path& outPath,
    bool& canceled)
{
    canceled = false;
    outPath.clear();
}