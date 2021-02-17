#pragma once
#include <string>
#include<d3d11_1.h>
#include<wrl/client.h>
#include <windows.h>
#include <functional>
#include <DirectXCollision.h>	// 已包含DirectXMath.h
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <algorithm>
#include <set>
//
// 宏相关
//

// 默认开启图形调试器具名化
// 如果不需要该项功能，可通过全局文本替换将其值设置为0
#ifndef GRAPHICS_DEBUGGER_OBJECT_NAME
#define GRAPHICS_DEBUGGER_OBJECT_NAME (1)
#endif

// 安全COM组件释放宏
#define SAFE_RELEASE(p) { if ((p)) { (p)->Release(); (p) = nullptr; } }

namespace YXX
{
	std::wstring StringToWString(const std::string& str);


	std::string WStringToString(const std::wstring& wstr);

	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	template<class T>
	using U = std::unique_ptr<T>;

	template<class T>
	using S = std::shared_ptr<T>;

}
// ------------------------------
// DXTraceW函数
// ------------------------------
// 在调试输出窗口中输出格式化错误信息，可选的错误窗口弹出(已汉化)
// [In]strFile			当前文件名，通常传递宏__FILEW__
// [In]hlslFileName     当前行号，通常传递宏__LINE__
// [In]hr				函数执行出现问题时返回的HRESULT值
// [In]strMsg			用于帮助调试定位的字符串，通常传递L#x(可能为NULL)
// [In]bPopMsgBox       如果为TRUE，则弹出一个消息弹窗告知错误信息
// 返回值: 形参hr
HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr, _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox);


// ------------------------------
// HR宏
// ------------------------------
// Debug模式下的错误提醒与追踪
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)												\
	{															\
		HRESULT hr = (x);										\
		if(FAILED(hr))											\
		{														\
			DXTraceW(__FILEW__, (DWORD)__LINE__, hr, L#x, true);\
		}														\
	}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif 
#endif

