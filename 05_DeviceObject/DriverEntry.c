#include <ntddk.h>

// ���������� - ��R3ʶ��� ��R0 \\??\\xxx��ʽ ��R3\\..\\xxx��ʽ
#define SYMLINK_NAME L"\\??\\nSkyDriver"
// �豸����
#define DEIVER_NAME  L"\\Device\\nSkyDevice"

/*
ntdll!_DEVICE_OBJECT
   +0x000 Type             : Int2B                // �豸����
   +0x002 Size             : Uint2B               // �豸�����С
   +0x004 ReferenceCount   : Int4B                // I/O������������׷������豸������������豸 - ���򿪵ľ������
   +0x008 DriverObject     : Ptr32 _DRIVER_OBJECT // �豸����������������
   +0x00c NextDevice       : Ptr32 _DEVICE_OBJECT // ָ����һ����ͬһ�������򴴽����豸����
   +0x010 AttachedDevice   : Ptr32 _DEVICE_OBJECT // �����豸ָ�룬������豸����û�й����豸����ó�ԱΪNULL
   +0x014 CurrentIrp       : Ptr32 _IRP           // �豸����ĵ�ǰ IRP
   +0x018 Timer            : Ptr32 _IO_TIMER      // ��ʱ��ָ��
   +0x01c Flags            : Uint4B               // �豸����ı�ʶ DO_BUFFERED_IO ���� DO_DIRECT_IO
   +0x020 Characteristics  : Uint4B               // �豸���������
   +0x024 Vpb              : Ptr32 _VPB           // ����豸������صľ������ָ�루VPB��
   +0x028 DeviceExtension  : Ptr32 Void           // �豸��չָ��
   +0x02c DeviceType       : Uint4B               // ��IoCreateDevice����IoCreatDeviceSecure�������趨
   +0x030 StackSize        : Char                 // �豸IRPջ��С
   +0x034 Queue            : __unnamed            // 
   +0x05c AlignmentRequirement : Uint4B           // �ó�Ա������ݵĴ���ʱָ���豸�ĵ�ַ��������
   +0x060 DeviceQueue      : _KDEVICE_QUEUE       // �豸����Ķ��С�
   +0x074 Dpc              : _KDPC                // �ӳٴ�����á�
   +0x094 ActiveThreadCount : Uint4B              // �������ã��ó�Ա���ɼ���
   +0x098 SecurityDescriptor : Ptr32 Void         // ���豸���󴴽�ʱָ����ȫ������
   +0x09c DeviceLock       : _KEVENT              // ��I/O������������ͬ���¼�����
   +0x0ac SectorSize       : Uint2B               // ����豸�����Ǳ�ʾһ�����ó�Ա���0��
                                                     ����Ǳ�ʾһ�����ó�Ա��ʾ���Ǿ�ķ������ֽ���
   +0x0ae Spare1           : Uint2B               // ������ϵͳ���ã��ó�Ա���ɼ�
   +0x0b0 DeviceObjectExtension : Ptr32 _DEVOBJ_EXTENSION // �豸������չ
   +0x0b4 Reserved         : Ptr32 Void           // ����

*/

// ж�������ص�����
VOID DriverUnload(PDRIVER_OBJECT pDriver);

// ������R3�������豸����
NTSTATUS CreateDevice(PDRIVER_OBJECT pDriver);


// IRP Ĭ�ϻص�����
NTSTATUS IoDefaultDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

// R3 - CreateFile��Ӧ����
NTSTATUS IoCreateDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

// R3 - CloseFile��Ӧ����
NTSTATUS IoCloseDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

// R3 - ReadFile��Ӧ����
NTSTATUS IoReadDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

// R3 - WriteFile��Ӧ����
NTSTATUS IoWriteDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

#pragma code_seg("INIT")
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);
  pDriver->DriverUnload = DriverUnload;

  // 1��Ϊ��ǰ�����µ������豸��������IRPĬ�ϻص�����
  for (int i = 0; i < 28; ++i)
    pDriver->MajorFunction[i] = IoDefaultDispath;

  // 2�������⺯������ IRP �ص� - ���ڴ���R3ָ��������������Ϣ
  pDriver->MajorFunction[IRP_MJ_CREATE] = IoCreateDispath;			// CreateFile
  pDriver->MajorFunction[IRP_MJ_CLOSE]  = IoCloseDispath;				// CloseFile
  pDriver->MajorFunction[IRP_MJ_READ]   = IoReadDispath;				// ReadFile
  pDriver->MajorFunction[IRP_MJ_WRITE]  = IoWriteDispath;				// WriteFile

  // 3������һ���豸�������ʧ���򷵻ش�����
  NTSTATUS status = CreateDevice(pDriver);

  return status;
}


#pragma code_seg("PAGE")
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
  UNREFERENCED_PARAMETER(pDriver);
  KdPrint(("Driver unloading...\n"));
}


// 1. ��ʼ���豸������ڲ��������ڴ����豸���󣬲����ڵ�ָ��������������
// 2. ��ʼ���豸�ⲿ�����������ƣ����ڰ��豸����
NTSTATUS CreateDevice(PDRIVER_OBJECT pDriver)
{
  NTSTATUS status = STATUS_SUCCESS;
  PDEVICE_OBJECT pDevice = NULL;

  // 1����ʼ���豸��������ƣ�Ҫ���ʽ������ \\Device\\xxx ����ʽ
  UNICODE_STRING wZDeviceName = RTL_CONSTANT_STRING(DEIVER_NAME);

  // 2�������豸����ʹ�� IoCreateDevice������ɹ����� STATUS_SUCCESS
  status = IoCreateDevice(
    pDriver,                // �豸����������������
    0,                      // �豸����չ�ռ��С������Ŀռ���� DeviceExtension �ֶ�ָ��
    &wZDeviceName,          // �豸��������ƣ�������ϸ�ʽ \\Device\\DeviceName
    FILE_DEVICE_UNKNOWN,		// �豸��������ͣ���ָӲ���޹ص������豸����
    FILE_DEVICE_SECURE_OPEN,// �豸�������ԣ�һ����FILE_DEVICE_SECURE_OPEN
    FALSE,						      // �Ƿ����ö�ռģʽ��ͬһʱ���ܱ��򿪼���
    &pDevice                // �����õ��豸���󱣴浽����
  );
  if (!NT_SUCCESS(status))
  {
    // �豸���󴴽�ʧ��
    KdPrint(("�豸���󴴽�ʧ�ܣ����ԭ�򣬴�����(%08X)\n", status));
    return status;
  }
  KdPrint(("�豸���󴴽��ɹ�\n"));


  // 3��������������
  // �豸���������ֻ�����ں��б�ֱ�ӵĽ�����Ϊ�� R3 �ܹ�ʶ�𲢲����豸������Ҫ
	// �������豸����ֱ�ӹ����ķ���������������д��: \\DosDevices\\xxx �� \\??\\xxx
  UNICODE_STRING wZSymLinkName = RTL_CONSTANT_STRING(SYMLINK_NAME);
  status = IoCreateSymbolicLink(&wZSymLinkName, &wZDeviceName);
  if (!NT_SUCCESS(status))
  {
    // �������Ӵ���ʧ��
    IoDeleteDevice(pDevice);
    KdPrint(("�������Ӵ���ʧ�ܣ����ԭ�򣬴�����(%08X)\n", status));
    return status;
  }
  KdPrint(("�������Ӵ����ɹ�\n"));
  return status;
}

#pragma code_seg(" ")
NTSTATUS IoDefaultDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // ʹ��δ���õĲ���ʹ��
  UNREFERENCED_PARAMETER(pDevice);


  // ����������ֵ
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("IoDefaultDispath\n"));

  // ���õ�ǰ IRP ��Ϣ�Ĵ���״̬������ɹ����뷵�� STATUS_SUCCESS�����ķ���ֵӰ�� R3 �� GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // ���õ�ǰ����Ϣ�����˶��ٸ��ֽڵ����ݣ�Ӱ�� ReadFile �з��ص�ʵ�ʶ�д�ֽ���
  pIrp->IoStatus.Information = 0;

  // ֪ͨ I/O ��������ǰ�� IRP �Ѿ�����ɹ�����Ҫ���ظ� R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}

// R3 - CreateFile��Ӧ����
NTSTATUS IoCreateDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // ʹ��δ���õĲ���ʹ��
  UNREFERENCED_PARAMETER(pDevice);

  // ����ֵ
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("CreateDevice\n"));

  // ���õ�ǰ IRP ��Ϣ�Ĵ���״̬������ɹ����뷵�� STATUS_SUCCESS�����ķ���ֵӰ�� R3 �� GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // ���õ�ǰ����Ϣ�����˶��ٸ��ֽڵ����ݣ�Ӱ�� ReadFile �з��ص�ʵ�ʶ�д�ֽ���
  pIrp->IoStatus.Information = 0;

  // ֪ͨ I/O ��������ǰ�� IRP �Ѿ�����ɹ�����Ҫ���ظ� R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}

// R3 - CloseFile��Ӧ����
NTSTATUS IoCloseDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // ʹ��δ���õĲ���ʹ��
  UNREFERENCED_PARAMETER(pDevice);

  // ����ֵ
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("CloseDevice\n"));

  // ���õ�ǰ IRP ��Ϣ�Ĵ���״̬������ɹ����뷵�� STATUS_SUCCESS�����ķ���ֵӰ�� R3 �� GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // ���õ�ǰ����Ϣ�����˶��ٸ��ֽڵ����ݣ�Ӱ�� ReadFile �з��ص�ʵ�ʶ�д�ֽ���
  pIrp->IoStatus.Information = 0;

  // ֪ͨ I/O ��������ǰ�� IRP �Ѿ�����ɹ�����Ҫ���ظ� R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}

// R3 - ReadFile��Ӧ����
NTSTATUS IoReadDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // ʹ��δ���õĲ���ʹ��
  UNREFERENCED_PARAMETER(pDevice);

  // ����ֵ
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("ReadDevice\n"));

  // ���õ�ǰ IRP ��Ϣ�Ĵ���״̬������ɹ����뷵�� STATUS_SUCCESS�����ķ���ֵӰ�� R3 �� GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // ���õ�ǰ����Ϣ�����˶��ٸ��ֽڵ����ݣ�Ӱ�� ReadFile �з��ص�ʵ�ʶ�д�ֽ���
  pIrp->IoStatus.Information = 0;

  // ֪ͨ I/O ��������ǰ�� IRP �Ѿ�����ɹ�����Ҫ���ظ� R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}

// R3 - WriteFile��Ӧ����
NTSTATUS IoWriteDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  // ʹ��δ���õĲ���ʹ��
  UNREFERENCED_PARAMETER(pDevice);

  // ����ֵ
  NTSTATUS status;
  status = STATUS_SUCCESS;

  KdPrint(("WriteDevice\n"));

  // ���õ�ǰ IRP ��Ϣ�Ĵ���״̬������ɹ����뷵�� STATUS_SUCCESS�����ķ���ֵӰ�� R3 �� GetLastError
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // ���õ�ǰ����Ϣ�����˶��ٸ��ֽڵ����ݣ�Ӱ�� ReadFile �з��ص�ʵ�ʶ�д�ֽ���
  pIrp->IoStatus.Information = 0;

  // ֪ͨ I/O ��������ǰ�� IRP �Ѿ�����ɹ�����Ҫ���ظ� R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}