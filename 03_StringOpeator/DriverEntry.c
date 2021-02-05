#include <ntddk.h>

/* Windows ���ڴ�ķ���( �ص� )
* 1����ҳ�ڴ�:   �ܹ����������ļ�(����)���ڴ� - ���ʿ��ܻ����ȱҳ�쳣
* 2���Ƿ�ҳ�ڴ�: ���ᱻ�������ļ����ڴ�       - �������ȱҳ�쳣
*/

/* IRQL( �ж����󼶱� )
*  Dispath(DPC) : ��������������ȼ���ͬ��ʱ��ֻ������һ�������ܷ��ʷ�ҳ�ڴ档
*                 ȱҳ�쳣��DPC��IRQLͬ�������ܷ����ж�
*  APC : ��DPC��һ�����𣬿��Է��ʷ�ҳ�ڴ�
*  Passive : ������ȼ���������������еļ���
*/

// ����ȫ�ֱ��������ڽ������뵽�ķǷ�ҳ�ռ�
PVOID g_NonPage = NULL;

// ж������
VOID DriverUnload(PDRIVER_OBJECT pDriver);

// �ַ�����
VOID StringOpeator();

#pragma code_seg("INIT")	
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);
  pDriver->DriverUnload = DriverUnload;

  // �ַ�������
  StringOpeator();
  return STATUS_SUCCESS;
}

// ��ʾ��ǰ�ĺ����������ڷ�ҳ�ڴ��У����
// ��ʱ���ڲ����õ������Է��õ�ҳ�����ļ���
#pragma code_seg("PAGE")						
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  /*
   * �ں˳������ͨӦ�ó���ͬ����ʹ��������ж���ˣ�ʹ�õ��ڴ�
   * Ҳ���ᱻ�Զ����ͷţ�������Ҫ��ж�ص�ʱ���ͷŵ����е��ڴ棬��
   * ��ᵼ���ڴ�й¶��ֱ������ϵͳ��������
  */
  ExFreePoolWithTag(g_NonPage, 'NSKY');
  UNREFERENCED_PARAMETER(pDriver);
}

VOID StringOpeator()
{
  /* UNICODE_STRING �ַ���
  *  ���ں���һ�㲻����C����ַ�����ԭ���ǿ��ܷ������
  *  ������ַ����ṹ������ں��������õı�ʾ�ַ����ķ�ʽ
  *  û��ʹ�ÿ��ַ���β������ͨ�����е�һЩ�ֶν���������
  *  typedef struct _UNICODE_STRING {
  *  USHORT Length;         // �ַ�������
  *  USHORT MaximumLength;  // �ַ�����󳤶�
  *  PWSTR  Buffer;         // �ַ���������
  * } UNICODE_STRING;
  */

  // ���ڵ���
  _asm int 3;

  // ��ʼ������1���ڴ�����ʱ��ֱ��ʹ�ó������г�ʼ������
  // RTL_CONSTANT_STRINGʵ���Ͼ��ǰ���˳���ʼ���ṹ��
  UNICODE_STRING wZString1 = RTL_CONSTANT_STRING(L"CONSTANT_STRING Init\n");
  KdPrint(("wZString1: %wZ\n", &wZString1));


  // ��ʼ������2: ʹ���ض����ַ�����ʼ���������̳�ʼ�� - RtlInitUnicodeString
  UNICODE_STRING wZString2 = { 0 };
  RtlInitUnicodeString(&wZString2, L"RtlInitUnicodeString Init\n");
  KdPrint(("wZString2: %wZ\n", &wZString2));


  // ��ʼ������3��ʹ���Զ���Ŀռ�������ַ����ṹ����
  // �Զ���ռ�
  g_NonPage = ExAllocatePoolWithTag(
    NonPagedPool,         // Ҫ����ĳ��ڴ������ NonPagedPool - PagedPool
    0x100,                // Ҫ������ֽ���
    'NSKY');              // ���ڷ����ڴ�ĳر��
  // ����ڴ��Ƿ����ɹ�
  if (NULL == g_NonPage)
  {
    return;
  }
  // ����ڴ�
  RtlFillMemory(g_NonPage, 0x100, 0xAA);
  RtlCopyMemory(g_NonPage, L"ExAllocatePoolWithTag", sizeof L"ExAllocatePoolWithTag");
  // ���UNICODE_STRING�ṹ��
  UNICODE_STRING wZString3 = { sizeof L"ExAllocatePoolWithTag",0x100  , g_NonPage };
  KdPrint(("wZString3: %wZ\n", &wZString3));
}
