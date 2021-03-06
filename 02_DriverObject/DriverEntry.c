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

typedef struct _LDR_DATA_TABLE_ENTRY
{
  // 模块加载的顺序
  struct _LIST_ENTRY InLoadOrderLinks;
  // 模块在内存的顺序
  struct _LIST_ENTRY InMemoryOrderLinks;
  // 模块初始化顺序
  struct _LIST_ENTRY InInitializationOrderLinks;
  // 模块基址
  VOID* DllBase;
  // 模块入口
  VOID* EntryPoint;
  // 模块大小
  ULONG SizeOfImage;
  // 模块全路径名称\??\ - 内核
  struct _UNICODE_STRING FullDllName;
  // 模块名称
  struct _UNICODE_STRING BaseDllName;
  ULONG Flags;
  USHORT LoadCount;
  USHORT TlsIndex;
  struct _LIST_ENTRY HashLinks;
  VOID* SectionPointer;
  ULONG CheckSum;
  ULONG TimeDateStamp;
  VOID* LoadedImports;
  VOID* EntryPointActivationContext;
  VOID* PatchInformation;
}LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

PLDR_DATA_TABLE_ENTRY g_pSelf;

// 卸载驱动，用于驱动卸载的清理工作
VOID DriverUnload(PDRIVER_OBJECT pDriver);

// 隐藏自身
VOID HideSelf(PDRIVER_OBJECT pDriver);

// 遍历系统所有驱动
VOID EnumAllDriver(PDRIVER_OBJECT pDriver);
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

  KdPrint(("%08X\n", (ULONG)pDriver->DriverInit));
  KdPrint(("Driver Loading...\n"));
  HideSelf(pDriver);
  EnumAllDriver(pDriver);
  
  // 驱动运行过程中需要使用 -  pDriver->DriverSection（自身的LDR_DATA_TABLE_ENTRY）
  pDriver->DriverSection = g_pSelf; // 还原一下

  return STATUS_SUCCESS;
}

// 卸载驱动，用于驱动卸载的清理工作
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  
  UNREFERENCED_PARAMETER(pDriver);
  KdPrint(("Driver Unloading...\n"));
}

VOID HideSelf(PDRIVER_OBJECT pDriver)
{
  // 断链
  /* []代表节点，--->代表前向指针Flink，<---代表后向指针 Blink
  * 断链前:
  * [self]--->head--->[]--->[]--->[self]
  * [self]<---head<---[]<---[]<---[self]
  * 
  * 断链后
  * head--->[]--->[]--->head  [self]--->null
  * head<---[]<---[]<---head  [self]--->null
  * 待删节点后向指针的前向 ---> 待删节点前向指针指向的节点
  * 待删节点前向指针的后向 ---> 待删节点后向指针指向的节点
  */
  // 默认第一项是指向自己驱动对象

  g_pSelf = pDriver->DriverSection;
  LIST_ENTRY pDel = g_pSelf->InLoadOrderLinks;
  pDriver->DriverSection = pDel.Flink; // 这行代码有问题 
                                       // - 指向Flink - 前 ERROR
                                       // - 指向Blink - 后 SUCCESS
  pDel.Blink->Flink = pDel.Flink;
  pDel.Flink->Blink = pDel.Blink;

  pDel.Flink = NULL;
  pDel.Blink = NULL;
}

VOID EnumAllDriver(PDRIVER_OBJECT pDriver)
{
  /*
  * 内核中PLDR_DATA_TABLE_ENTRY中的InMemoryOrderLinks和InInitializationOrderLinks并不是模块链接
  * InLoadOrderLinks
  */

  PLDR_DATA_TABLE_ENTRY cur = pDriver->DriverSection;
  ULONG uIndex = 0;
  
  do
  {
    // 输出每个驱动模块信息
    KdPrint(("[%02d]\n[SizeOfImage]: %08X\n[FullDllName]: %wZ\n[BaseDllName]: %wZ\n\n"
      , uIndex, (ULONG)cur->SizeOfImage, &cur->FullDllName, &cur->BaseDllName));

    // 查找下一个
    uIndex++;
    cur = (PLDR_DATA_TABLE_ENTRY)cur->InLoadOrderLinks.Flink;
  } while (cur != pDriver->DriverSection);
}