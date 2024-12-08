
#ifdef _WIN32
// WINDOWS implementations go here

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Strmiids.lib")

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "icudt.lib")
#pragma comment(lib, "icuin.lib")
#pragma comment(lib, "icuuc.lib")

#else
// LINUX implementations go here

#endif
