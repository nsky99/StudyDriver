#include <ntddk.h>

// 符号链接名 - 给R3识别的 在R0 \\??\\xxx形式 在R3\\..\\xxx形式
#define SYMLINK_NAME L"\\??\\nSkyDriver"
// 设备名称
#define DEIVER_NAME  L"\\Device\\nSkyDevice"

/*
ntdll!_DEVICE_OBJECT
   +0x000 Type             : Int2B                // 设备类型
   +0x002 Size             : Uint2B               // 设备对象大小
   +0x004 ReferenceCount   : Int4B                // I/O管理器用它来追踪与该设备对象相关联的设备 - 被打开的句柄数量
   +0x008 DriverObject     : Ptr32 _DRIVER_OBJECT // 设备对象所属驱动对象
   +0x00c NextDevice       : Ptr32 _DEVICE_OBJECT // 指向下一个由同一驱动程序创建的设备对象
   +0x010 AttachedDevice   : Ptr32 _DEVICE_OBJECT // 挂载设备指针，如果该设备对象没有挂载设备，则该成员为NULL
   +0x014 CurrentIrp       : Ptr32 _IRP           // 设备对象的当前 IRP
   +0x018 Timer            : Ptr32 _IO_TIMER      // 计时器指针
   +0x01c Flags            : Uint4B               // 设备对象的标识 DO_BUFFERED_IO 或者 DO_DIRECT_IO
   +0x020 Characteristics  : Uint4B               // 设备对象的属性
   +0x024 Vpb              : Ptr32 _VPB           // 与该设备对象相关的卷参数块指针（VPB）
   +0x028 DeviceExtension  : Ptr32 Void           // 设备拓展指针
   +0x02c DeviceType       : Uint4B               // 由IoCreateDevice或者IoCreatDeviceSecure函数是设定
   +0x030 StackSize        : Char                 // 设备IRP栈大小
   +0x034 Queue            : __unnamed            // 
   +0x05c AlignmentRequirement : Uint4B           // 该成员针对数据的传输时指定设备的地址对齐请求，
   +0x060 DeviceQueue      : _KDEVICE_QUEUE       // 设备对象的队列。
   +0x074 Dpc              : _KDPC                // 延迟处理调用。
   +0x094 ActiveThreadCount : Uint4B              // 保留备用，该成员不可见。
   +0x098 SecurityDescriptor : Ptr32 Void         // 当设备对象创建时指定安全描述表
   +0x09c DeviceLock       : _KEVENT              // 由I/O管理器创建的同步事件对象
   +0x0ac SectorSize       : Uint2B               // 如果设备对象不是表示一个卷，该成员设成0，
                                                     如果是表示一个卷，该成员表示的是卷的分区的字节数
   +0x0ae Spare1           : Uint2B               // 保留供系统备用，该成员不可见
   +0x0b0 DeviceObjectExtension : Ptr32 _DEVOBJ_EXTENSION // 设备对象扩展
   +0x0b4 Reserved         : Ptr32 Void           // 保留

*/

// 卸载驱动回调函数
VOID DriverUnload(PDRIVER_OBJECT pDriver);

// 创建和R3交互的设备对象
NTSTATUS CreateDevice(PDRIVER_OBJECT pDriver);


// IRP 默认回调函数
NTSTATUS IoDefaultDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

// R3 - CreateFile响应函数
NTSTATUS IoCreateDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

// R3 - CloseFile响应函数
NTSTATUS IoCloseDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

// R3 - ReadFile响应函数
NTSTATUS IoReadDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

// R3 - WriteFile响应函数
NTSTATUS IoWriteDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

#pragma code_seg("INIT")
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);
  pDriver->DriverUnload = DriverUnload;

  // 1、为当前驱动下的所有设备对象，设置IRP默认回调函数
  for (int i = 0; i < 28; ++i)
    pDriver->MajorFunction[i] = IoDefaultDispath;

  // 2、对特殊函数设置 IRP 回调 - 用于处理R3指定函数发出的消息
  pDriver->MajorFunction[IRP_MJ_CREATE] = IoCreateDispath;			// CreateFile
  pDriver->MajorFunction[IRP_MJ_CLOSE]  = IoCloseDispath;				// CloseFile
  pDriver->MajorFunction[IRP_MJ_READ]   = IoReadDispath;				// ReadFile
  pDriver->MajorFunction[IRP_MJ_WRITE]  = IoWriteDispath;				// WriteFile

  // 3、创建一个设备对象，如果失败则返回错误码
  NTSTATUS status = CreateDevice(pDriver);

  return status;
}


#pragma code_seg("PAGE")
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  UNREFERENCED_PARAMETER(pDriver);
  KdPrint(("Driver unloading...\n"));
}


// 1. 初始化设备对象的内部名称用于创建设备对象，并挂在到指定的驱动对象中
// 2. 初始化设备外部符号链接名称，用于绑定设备对象
NTSTATUS CreateDevice(PDRIVER_OBJECT pDriver)
{
  NTSTATUS status = STATUS_SUCCESS;
  PDEVICE_OBJECT pDevice = NULL;

  // 1、初始化设备对象的名称，要求格式必须是 \\Device\\xxx 的形式
  UNICODE_STRING wZDeviceName = RTL_CONSTANT_STRING(DEIVER_NAME);

  // 2、创建设备对象使用 IoCreateDevice，如果成功返回 STATUS_SUCCESS
  status = IoCreateDevice(
    pDriver,                // 设备对象所属驱动对象
    0,                      // 设备的扩展空间大小，分配的空间会由 DeviceExtension 字段指向
    &wZDeviceName,          // 设备对象的名称，必须符合格式 \\Device\\DeviceName
    FILE_DEVICE_UNKNOWN,		// 设备对象的类型，特指硬件无关的虚拟设备对象
    FILE_DEVICE_SECURE_OPEN,// 设备对象属性，一般填FILE_DEVICE_SECURE_OPEN
    FALSE,						      // 是否启用独占模式，同一时刻能被打开几次
    &pDevice                // 创建好的设备对象保存到哪里
  );
  if (!NT_SUCCESS(status))
  {
    // 设备对象创建失败
    KdPrint(("设备对象创建失败，检查原因，错误码(%08X)\n", status));
    return status;
  }
  KdPrint(("设备对象创建成功\n"));


  // 3、创建符号链接
  // 设备对象的名称只能在内核中被直接的解析，为了 R3 能够识别并操作设备对象，需要
	// 创建与设备名称直接关联的符号链接名，必须写作: \\DosDevices\\xxx 或 \\??\\xxx
  UNICODE_STRING wZSymLinkName = RTL_CONSTANT_STRING(SYMLINK_NAME);
  status = IoCreateSymbolicLink(&wZSymLinkName, &wZDeviceName);
  if (!NT_SUCCESS(status))
  {
    // 符号链接创建失败
    IoDeleteDevice(pDevice);
    KdPrint(("符号链接创建失败，检查原因，错误码(%08X)\n", status));
    return status;
  }
  KdPrint(("符号链接创建成功\n"));
  return status;
}

#pragma code_seg(" ")
NTSTATUS IoDefaultDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // 使用未引用的参数使用
  UNREFERENCED_PARAMETER(pDevice);


  // 本函数返回值
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("IoDefaultDispath\n"));

  // 设置当前 IRP 消息的处理状态，如果成功必须返回 STATUS_SUCCESS，它的返回值影响 R3 的 GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // 设置当前的消息处理了多少个字节的数据，影响 ReadFile 中返回的实际读写字节数
  pIrp->IoStatus.Information = 0;

  // 通知 I/O 管理器当前的 IRP 已经处理成功，需要返回给 R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}

// R3 - CreateFile响应函数
NTSTATUS IoCreateDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // 使用未引用的参数使用
  UNREFERENCED_PARAMETER(pDevice);

  // 返回值
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("CreateDevice\n"));

  // 设置当前 IRP 消息的处理状态，如果成功必须返回 STATUS_SUCCESS，它的返回值影响 R3 的 GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // 设置当前的消息处理了多少个字节的数据，影响 ReadFile 中返回的实际读写字节数
  pIrp->IoStatus.Information = 0;

  // 通知 I/O 管理器当前的 IRP 已经处理成功，需要返回给 R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}

// R3 - CloseFile响应函数
NTSTATUS IoCloseDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // 使用未引用的参数使用
  UNREFERENCED_PARAMETER(pDevice);

  // 返回值
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("CloseDevice\n"));

  // 设置当前 IRP 消息的处理状态，如果成功必须返回 STATUS_SUCCESS，它的返回值影响 R3 的 GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // 设置当前的消息处理了多少个字节的数据，影响 ReadFile 中返回的实际读写字节数
  pIrp->IoStatus.Information = 0;

  // 通知 I/O 管理器当前的 IRP 已经处理成功，需要返回给 R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}

// R3 - ReadFile响应函数
NTSTATUS IoReadDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // 使用未引用的参数使用
  UNREFERENCED_PARAMETER(pDevice);

  // 返回值
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("ReadDevice\n"));

  // 设置当前 IRP 消息的处理状态，如果成功必须返回 STATUS_SUCCESS，它的返回值影响 R3 的 GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // 设置当前的消息处理了多少个字节的数据，影响 ReadFile 中返回的实际读写字节数
  pIrp->IoStatus.Information = 0;

  // 通知 I/O 管理器当前的 IRP 已经处理成功，需要返回给 R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}

// R3 - WriteFile响应函数
NTSTATUS IoWriteDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // 使用未引用的参数使用
  UNREFERENCED_PARAMETER(pDevice);

  // 返回值
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("WriteDevice\n"));

  // 设置当前 IRP 消息的处理状态，如果成功必须返回 STATUS_SUCCESS，它的返回值影响 R3 的 GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // 设置当前的消息处理了多少个字节的数据，影响 ReadFile 中返回的实际读写字节数
  pIrp->IoStatus.Information = 0;

  // 通知 I/O 管理器当前的 IRP 已经处理成功，需要返回给 R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}