#pragma once

#include <windows.h>
#include <functional>
#include <utility>

// ============================================================================
// WebView2Helper.h
// 该辅助模板解决了 MinGW-w64 (GCC) 环境下没有微软 MSVC 专有 <wrl.h> 的问题。
// 它允许你直接用 C++ Lambda 表达式来处理 WebView2 的所有异步事件和回调！
//
// 使用方式例如：
// auto handler = Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
//     [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
//         // 你的逻辑
//         return S_OK;
//     }
// );
// ============================================================================

template<typename TInterface, typename TMemberFn>
struct HandlerHelper;

template<typename TInterface, typename R, typename... Args>
struct HandlerHelper<TInterface, R(STDMETHODCALLTYPE TInterface::*)(Args...)> : public TInterface {
    using CallbackFn = std::function<R(Args...)>;
    CallbackFn m_fn;
    LONG m_refCount = 1;

    explicit HandlerHelper(CallbackFn fn) : m_fn(std::move(fn)) {}

    STDMETHODIMP QueryInterface(REFIID, void** ppv) override {
        if (!ppv) return E_POINTER;
        *ppv = static_cast<TInterface*>(this);
        AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef() override { 
        return InterlockedIncrement(&m_refCount); 
    }

    STDMETHODIMP_(ULONG) Release() override {
        ULONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) delete this;
        return ref;
    }

    R STDMETHODCALLTYPE Invoke(Args... args) override {
        return m_fn(std::forward<Args>(args)...);
    }
};

template<typename TInterface, typename TFn>
TInterface* Callback(TFn&& fn) {
    using Base = HandlerHelper<TInterface, decltype(&TInterface::Invoke)>;
    return new Base(std::forward<TFn>(fn));
}
