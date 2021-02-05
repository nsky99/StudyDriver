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


/*
  ��������: ������ GUI �����е�Ӧ�ó���������ֱ�Ӻ� R3 ���н�������Ҫ�����豸���󴫵� IRP
  typedef struct _DRIVER_OBJECT {
    // ��ʾ�ṹ������ͣ����е���������Ϊ 4
    CSHORT Type;
    // ��������Ĵ�С����ռ�õ��ֽ���
    CSHORT Size;

    // * ��ǰ���������������豸������ϳɵĵ�������
    PDEVICE_OBJECT DeviceObject;
    // Ϊ������������ṩ��һ������չ�ı�־λ��
    ULONG Flags;

    // ��ǰ�������ڴ��е���ʼλ��
    PVOID DriverStart;
    // ����������ռ�õ��ܿռ�
    ULONG DriverSize;
    // * ָ�� LDR �����������ڱ���ϵͳ�µ�������������
    PVOID DriverSection;
    // ָ����չ�ռ��ָ��
    PDRIVER_EXTENSION DriverExtension;

    // * �������������ֶ��ɴ�����־�߳�ʹ�ã�����ȷ��I/O����󶨵�������������ơ�
    UNICODE_STRING DriverName;

    // ע���֧��, ��ָ��ע�����Ӳ����Ϣ·����ָ��,��Ӳ�����
    PUNICODE_STRING HardwareDatabase;

    // ָ����������ı�����ڵ�����Ŀ�ѡָ�룬��֧�֡�����I/O��
    // ����I/O��ͨ��ʹ�õ����Ĳ���ֱ�ӵ�����������������ִ�еģ�������ʹ�ñ�׼��IRP���û��ơ�
    // * �������ǲ���������ļ����������лᱻʹ��
    PFAST_IO_DISPATCH FastIoDispatch;

    // ָ������������ں���(DriverEntry�����GsDriverEntry)���� I/O ����������
    PDRIVER_INITIALIZE DriverInit;
    // * �������� IRP �Ĵ��л����� - IRPͨ�ŵķ�ʽ
    PDRIVER_STARTIO DriverStartIo;
    // * ���������ж�غ��������ṩ���޷�ж��
    PDRIVER_UNLOAD DriverUnload;
    // * ��Ϣ��ǲ������ɵ��б�(��������Ϣ������)
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

// ж����������������ж�ص�������
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  UNREFERENCED_PARAMETER(pDriver);
  KdPrint(("Driver Unloading...\n"));
}