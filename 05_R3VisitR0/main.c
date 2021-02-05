#include <Windows.h>
#include <stdio.h>

// 符号链接名 - 给R3识别的 在R0 \\??\\xxx形式 在R3\\..\\xxx形式
#define SYMLINK_NAME L"\\\\.\\nSkyDriver"

int main(int argc, char* argv[])
{
	// 在驱动程序中创建了设备对象后，可以使用 CreateFile 连接到设备对象，并进行通信
	//	设备对象必须以 \\\\.\\ 开头，当前程序必须使用管理员权限打开
	HANDLE hDevice = CreateFileW(SYMLINK_NAME, GENERIC_ALL, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// 判断句柄是否打开，输出错误信息
	if (hDevice == NULL || hDevice == INVALID_HANDLE_VALUE)
	{
		printf("设备对象打开失败，错误原因(%08X)\n", GetLastError());
		system("pause"); 
		return 0;
	}

	// 读写函数
	DWORD dwOpBytes = 0;
	BOOL bRet = 0;
	bRet = ReadFile(hDevice, NULL, 0, &dwOpBytes, NULL);
	if (!bRet)
	{
		printf("ReadFile，错误原因(%08X)\n", GetLastError());
		system("pause");
	}
	bRet = WriteFile(hDevice, NULL, 0, &dwOpBytes, NULL);
	if (!bRet)
	{
		printf("WriteFile，错误原因(%08X)\n", GetLastError());
		system("pause");
	}

	bRet = CloseHandle(hDevice);
	if (!bRet)
	{
		printf("CloseHandle，错误原因(%08X)\n", GetLastError());
		system("pause");
	}
  return 0;
}