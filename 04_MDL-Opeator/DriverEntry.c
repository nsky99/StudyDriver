#include <ntddk.h>
// 卸载驱动
VOID DriverUnload(PDRIVER_OBJECT pDriver);

// MDL 操作
VOID MDLOpeator();

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);
  pDriver->DriverUnload = DriverUnload;
  MDLOpeator();
  return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  KdPrint(("Driver Unloading...\n"));
  UNREFERENCED_PARAMETER(pDriver);
}

VOID MDLOpeator()
{

  /* MDL结构体
  * struct _MDL {
    struct _MDL *Next;        // 下一个MDL
    CSHORT Size;              // MDL的大小
    CSHORT MdlFlags;          // 指明MDL的映射方式
    struct _EPROCESS *Process;// 指明此MDL属于哪个进程
    PVOID MappedSystemVa;     // 重新映射的虚拟地址( 页的开始位置 )
    PVOID StartVa;            // 需要重新映射的虚拟地址( 页的开始位置 )
    ULONG ByteCount;          // 此MDL所描述的需要重新映射的有多少个字节
    ULONG ByteOffset;         // 需要映射的数据距离页的起始位置的偏移
    } MDL, * PMDL;
  */
  // 创建一个常量指针，指向不可修改的常量字符串
  LPWSTR szBuffer1 = L"ABCDEFGHIJKLMN";

  // 1. 分配一个 MDL 描述符， 用于映射指定的一个虚拟地址
  // 最后一个参数是指向要关联的 - IRP
  PMDL pMdl = IoAllocateMdl(szBuffer1, sizeof L"ABCDEFGHIJKLMN", FALSE, FALSE, NULL);
  
  _asm int 3;
  // 2. 对MDL进行配置
  // 对于分页内存，需要使用下面的这个函数进行锁住，暂时性的不允许交换
  // 对于非分页内存，应该使用 MmBuildMdlForNonPagedPool 创建MDL
  MmProbeAndLockPages(pMdl, KernelMode, IoModifyAccess);

  // 3. 设置 MDL 结构体中的 MdlFlags 字段，添加新的属性，能够允许新的映射可读写
  pMdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

  // 4. 重新映射虚拟内存 - 将MDL描述的物理页映射到虚拟地址
  // MmMapLockedPages - 弱版
  PWCHAR szBuffer2 = 
    MmMapLockedPagesSpecifyCache(
      pMdl,                     // 需要重新映射的虚拟内存
      KernelMode,               // 指定映射 MDL 所处的模式 kernel/user
      MmCached,                 // 指示用于映射 MDL 的缓存属性 - 是否支持缓存
      NULL,                     // 请求将物理地址映射到哪一个新的虚拟地址上，如果是 NULL，表示随机分配
      FALSE,                    // 驱动程序必须将此参数设置为FALSE
      NormalPagePriority);      // 当 PTE 稀缺时,内存分配状态 是否能够分配成功
  // 检查MDL虚拟内存是否映射成功
  if (!szBuffer2)
  {
    return;
  }

  // 这个时候，实际上szBuffer2的和szBuffer1 挂了同一个物理页
  szBuffer2[0] = L'0';
  KdPrint(("szBuffer1: %ws\n", szBuffer1));
  KdPrint(("szBuffer2: %ws\n", szBuffer2));

  // 5. 取消对MDL结构体中虚拟内存的映射
  MmUnmapLockedPages(szBuffer2, pMdl);
  
  // 6. 取消MDL分页内存的锁定
  MmUnlockPages(pMdl);

  // 7. 释放MDL所占内存
  IoFreeMdl(pMdl);
}
