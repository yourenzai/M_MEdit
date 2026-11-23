#include <windows.h>
#include <stdio.h>
#include "WebView2.h"

int main() {
    IID id = __uuidof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler);
    printf("UUID test passed!\n");
    return 0;
}
