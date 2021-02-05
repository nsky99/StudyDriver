#include <ntddk.h>

/*
* 项目配置：
*   C/C++ --> Specture Mitigation - Enabled (/Qspectre)
*     - 没有安装Specture缓解器
*   
*   Driver settings --> Target OS Version - 驱动运行的目标平台
*   
*   删除Driver Files下的 xxxx.inf文件
* 
*/

// 卸载驱动，用于驱动卸载的清理工作
VOID DriverUnload(PDRIVER_OBJECT pDriver);

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);

  pDriver->DriverUnload = DriverUnload;

  /*
  * KdPrint是一个宏：
  * #if DBG
  * #define KdPrint(_x_) DbgPrint _x_
  * #else
  * #define KdPrint(_x_)
  * 
  * 意思是如果是DBG模式编译就输出，否则就不输出
  */
  KdPrint(("Driver Loading...\n"));
  
  return STATUS_SUCCESS;
}

// 卸载驱动，用于驱动卸载的清理工作
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  UNREFERENCED_PARAMETER(pDriver);
  KdPrint(("Driver Unloading...\n"));
}