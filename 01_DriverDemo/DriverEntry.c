#include <ntddk.h>

/*
* ��Ŀ���ã�
*   C/C++ --> Specture Mitigation - Enabled (/Qspectre)
*     - û�а�װSpecture������
*   
*   Driver settings --> Target OS Version - �������е�Ŀ��ƽ̨
*   
*   ɾ��Driver Files�µ� xxxx.inf�ļ�
* 
*/

// ж����������������ж�ص�������
VOID DriverUnload(PDRIVER_OBJECT pDriver);

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);

  pDriver->DriverUnload = DriverUnload;

  /*
  * KdPrint��һ���꣺
  * #if DBG
  * #define KdPrint(_x_) DbgPrint _x_
  * #else
  * #define KdPrint(_x_)
  * 
  * ��˼�������DBGģʽ��������������Ͳ����
  */
  KdPrint(("Driver Loading...\n"));
  
  return STATUS_SUCCESS;
}

// ж����������������ж�ص�������
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  UNREFERENCED_PARAMETER(pDriver);
  KdPrint(("Driver Unloading...\n"));
}