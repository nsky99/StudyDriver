#include <Windows.h>
#include <stdio.h>

// ���������� - ��R3ʶ��� ��R0 \\??\\xxx��ʽ ��R3\\..\\xxx��ʽ
#define SYMLINK_NAME L"\\\\.\\nSkyDriver"

int main(int argc, char* argv[])
{
	// �����������д������豸����󣬿���ʹ�� CreateFile ���ӵ��豸���󣬲�����ͨ��
	//	�豸��������� \\\\.\\ ��ͷ����ǰ�������ʹ�ù���ԱȨ�޴�
	HANDLE hDevice = CreateFileW(SYMLINK_NAME, GENERIC_ALL, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// �жϾ���Ƿ�򿪣����������Ϣ
	if (hDevice == NULL || hDevice == INVALID_HANDLE_VALUE)
	{
		printf("�豸�����ʧ�ܣ�����ԭ��(%08X)\n", GetLastError());
		system("pause"); 
		return 0;
	}

	// ��д����
	DWORD dwOpBytes = 0;
	BOOL bRet = 0;
	bRet = ReadFile(hDevice, NULL, 0, &dwOpBytes, NULL);
	if (!bRet)
	{
		printf("ReadFile������ԭ��(%08X)\n", GetLastError());
		system("pause");
	}
	bRet = WriteFile(hDevice, NULL, 0, &dwOpBytes, NULL);
	if (!bRet)
	{
		printf("WriteFile������ԭ��(%08X)\n", GetLastError());
		system("pause");
	}

	bRet = CloseHandle(hDevice);
	if (!bRet)
	{
		printf("CloseHandle������ԭ��(%08X)\n", GetLastError());
		system("pause");
	}
  return 0;
}