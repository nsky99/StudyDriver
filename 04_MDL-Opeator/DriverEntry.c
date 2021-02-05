#include <ntddk.h>
// ж������
VOID DriverUnload(PDRIVER_OBJECT pDriver);

// MDL ����
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

  /* MDL�ṹ��
  * struct _MDL {
    struct _MDL *Next;        // ��һ��MDL
    CSHORT Size;              // MDL�Ĵ�С
    CSHORT MdlFlags;          // ָ��MDL��ӳ�䷽ʽ
    struct _EPROCESS *Process;// ָ����MDL�����ĸ�����
    PVOID MappedSystemVa;     // ����ӳ��������ַ( ҳ�Ŀ�ʼλ�� )
    PVOID StartVa;            // ��Ҫ����ӳ��������ַ( ҳ�Ŀ�ʼλ�� )
    ULONG ByteCount;          // ��MDL����������Ҫ����ӳ����ж��ٸ��ֽ�
    ULONG ByteOffset;         // ��Ҫӳ������ݾ���ҳ����ʼλ�õ�ƫ��
    } MDL, * PMDL;
  */
  // ����һ������ָ�룬ָ�򲻿��޸ĵĳ����ַ���
  LPWSTR szBuffer1 = L"ABCDEFGHIJKLMN";

  // 1. ����һ�� MDL �������� ����ӳ��ָ����һ�������ַ
  // ���һ��������ָ��Ҫ������ - IRP
  PMDL pMdl = IoAllocateMdl(szBuffer1, sizeof L"ABCDEFGHIJKLMN", FALSE, FALSE, NULL);
  
  _asm int 3;
  // 2. ��MDL��������
  // ���ڷ�ҳ�ڴ棬��Ҫʹ��������������������ס����ʱ�ԵĲ�������
  // ���ڷǷ�ҳ�ڴ棬Ӧ��ʹ�� MmBuildMdlForNonPagedPool ����MDL
  MmProbeAndLockPages(pMdl, KernelMode, IoModifyAccess);

  // 3. ���� MDL �ṹ���е� MdlFlags �ֶΣ�����µ����ԣ��ܹ������µ�ӳ��ɶ�д
  pMdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

  // 4. ����ӳ�������ڴ� - ��MDL����������ҳӳ�䵽�����ַ
  // MmMapLockedPages - ����
  PWCHAR szBuffer2 = 
    MmMapLockedPagesSpecifyCache(
      pMdl,                     // ��Ҫ����ӳ��������ڴ�
      KernelMode,               // ָ��ӳ�� MDL ������ģʽ kernel/user
      MmCached,                 // ָʾ����ӳ�� MDL �Ļ������� - �Ƿ�֧�ֻ���
      NULL,                     // ���������ַӳ�䵽��һ���µ������ַ�ϣ������ NULL����ʾ�������
      FALSE,                    // ����������뽫�˲�������ΪFALSE
      NormalPagePriority);      // �� PTE ϡȱʱ,�ڴ����״̬ �Ƿ��ܹ�����ɹ�
  // ���MDL�����ڴ��Ƿ�ӳ��ɹ�
  if (!szBuffer2)
  {
    return;
  }

  // ���ʱ��ʵ����szBuffer2�ĺ�szBuffer1 ����ͬһ������ҳ
  szBuffer2[0] = L'0';
  KdPrint(("szBuffer1: %ws\n", szBuffer1));
  KdPrint(("szBuffer2: %ws\n", szBuffer2));

  // 5. ȡ����MDL�ṹ���������ڴ��ӳ��
  MmUnmapLockedPages(szBuffer2, pMdl);
  
  // 6. ȡ��MDL��ҳ�ڴ������
  MmUnlockPages(pMdl);

  // 7. �ͷ�MDL��ռ�ڴ�
  IoFreeMdl(pMdl);
}
