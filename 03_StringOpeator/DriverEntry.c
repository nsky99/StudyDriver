#include <ntddk.h>

/* Windows 中内存的分类( 重点 )
* 1、分页内存:   能够被交换到文件(磁盘)的内存 - 访问可能会产生缺页异常
* 2、非分页内存: 不会被交换到文件的内存       - 不会产生缺页异常
*/

/* IRQL( 中断请求级别 )
*  Dispath(DPC) : 软件层面的最高优先级，同意时刻只会运行一个，不能访问分页内存。
*                 缺页异常和DPC的IRQL同级，不能发生中断
*  APC : 比DPC低一个级别，可以访问分页内存
*  Passive : 最低优先级，大多数代码运行的级别
*/

// 创建全局变量，用于接受申请到的非分页空间
PVOID g_NonPage = NULL;

// 卸载驱动
VOID DriverUnload(PDRIVER_OBJECT pDriver);

// 字符操作
VOID StringOpeator();

#pragma code_seg("INIT")	
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);
  pDriver->DriverUnload = DriverUnload;

  // 字符串操作
  StringOpeator();
  return STATUS_SUCCESS;
}

// 表示当前的函数被放置在分页内存中，如果
// 短时间内不会用到，可以放置到页交换文件中
#pragma code_seg("PAGE")						
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  /*
   * 内核程序和普通应用程序不同，即使驱动程序被卸载了，使用的内存
   * 也不会被自动的释放，所以需要在卸载的时候释放掉所有的内存，否
   * 则会导致内存泄露，直至操作系统重新启动
  */
  ExFreePoolWithTag(g_NonPage, 'NSKY');
  UNREFERENCED_PARAMETER(pDriver);
}

VOID StringOpeator()
{
  /* UNICODE_STRING 字符串
  *  在内核中一般不采用C风格字符串，原因是可能发生溢出
  *  下面的字符串结构体就是内核中所采用的表示字符串的方式
  *  没有使用空字符结尾，而是通过其中的一些字段进行描述。
  *  typedef struct _UNICODE_STRING {
  *  USHORT Length;         // 字符串长度
  *  USHORT MaximumLength;  // 字符串最大长度
  *  PWSTR  Buffer;         // 字符串缓冲区
  * } UNICODE_STRING;
  */

  // 用于调试
  _asm int 3;

  // 初始化方法1：在创建的时候直接使用常量进行初始化操作
  // RTL_CONSTANT_STRING实际上就是按照顺序初始化结构体
  UNICODE_STRING wZString1 = RTL_CONSTANT_STRING(L"CONSTANT_STRING Init\n");
  KdPrint(("wZString1: %wZ\n", &wZString1));


  // 初始化方法2: 使用特定的字符串初始化函数进程初始化 - RtlInitUnicodeString
  UNICODE_STRING wZString2 = { 0 };
  RtlInitUnicodeString(&wZString2, L"RtlInitUnicodeString Init\n");
  KdPrint(("wZString2: %wZ\n", &wZString2));


  // 初始化方法3：使用自定义的空间关联到字符串结构体中
  // 自定义空间
  g_NonPage = ExAllocatePoolWithTag(
    NonPagedPool,         // 要分配的池内存的类型 NonPagedPool - PagedPool
    0x100,                // 要分配的字节数
    'NSKY');              // 用于分配内存的池标记
  // 检查内存是否分配成功
  if (NULL == g_NonPage)
  {
    return;
  }
  // 填充内存
  RtlFillMemory(g_NonPage, 0x100, 0xAA);
  RtlCopyMemory(g_NonPage, L"ExAllocatePoolWithTag", sizeof L"ExAllocatePoolWithTag");
  // 填充UNICODE_STRING结构体
  UNICODE_STRING wZString3 = { sizeof L"ExAllocatePoolWithTag",0x100  , g_NonPage };
  KdPrint(("wZString3: %wZ\n", &wZString3));
}
