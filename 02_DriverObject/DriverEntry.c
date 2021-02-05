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


/*
  驱动对象: 类似于 GUI 程序中的应用程序本身，不能直接和 R3 进行交互，需要依赖设备对象传递 IRP
  typedef struct _DRIVER_OBJECT {
    // 表示结构体的类型，所有的驱动对象都为 4
    CSHORT Type;
    // 驱动对象的大小，所占用的字节数
    CSHORT Size;

    // * 当前驱动对象下所有设备对象组合成的单向链表
    PDEVICE_OBJECT DeviceObject;
    // 为驱动程序对象提供了一个可扩展的标志位置
    ULONG Flags;

    // 当前对象在内存中的起始位置
    PVOID DriverStart;
    // 驱动对象所占用的总空间
    ULONG DriverSize;
    // * 指向 LDR 链表，可以用于遍历系统下的所有驱动对象
    PVOID DriverSection;
    // 指向扩展空间的指针
    PDRIVER_EXTENSION DriverExtension;

    // * 驱动程序名称字段由错误日志线程使用，用于确定I/O请求绑定的驱动程序的名称。
    UNICODE_STRING DriverName;

    // 注册表支持, 是指向注册表中硬件信息路径的指针,和硬件相关
    PUNICODE_STRING HardwareDatabase;

    // 指向驱动程序的备用入口点数组的可选指针，以支持“快速I/O”
    // 快速I/O是通过使用单独的参数直接调用驱动程序例程来执行的，而不是使用标准的IRP调用机制。
    // * 特殊的派遣函数，在文件过滤驱动中会被使用
    PFAST_IO_DISPATCH FastIoDispatch;

    // 指向驱动程序入口函数(DriverEntry的入口GsDriverEntry)，由 I/O 管理器设置
    PDRIVER_INITIALIZE DriverInit;
    // * 用于驱动 IRP 的串行化处理 - IRP通信的方式
    PDRIVER_STARTIO DriverStartIo;
    // * 驱动程序的卸载函数，不提供就无法卸载
    PDRIVER_UNLOAD DriverUnload;
    // * 消息派遣函数组成的列表(类似于消息处理函数)
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];
} DRIVER_OBJECT;
*/

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);

  pDriver->DriverUnload = DriverUnload;

  KdPrint(("%08X", (ULONG)pDriver->DriverInit));
  KdPrint(("Driver Loading...\n"));

  return STATUS_SUCCESS;
}

// 卸载驱动，用于驱动卸载的清理工作
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  UNREFERENCED_PARAMETER(pDriver);
  KdPrint(("Driver Unloading...\n"));
}