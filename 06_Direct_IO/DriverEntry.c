#include <wdm.h>

// ж�غ���
VOID DriverUnload(PDRIVER_OBJECT pDriver);

// Ĭ��IRP�ַ�
NTSTATUS IrpDefaultDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);

  // ����ж�غ���
  pDriver->DriverUnload = DriverUnload;

  // DriverEntry����ֵ
  NTSTATUS status = STATUS_SUCCESS;




  return status;
}

NTSTATUS IrpDefaultDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  UNREFERENCED_PARAMETER(pDevice);

  // DriverEntry����ֵ
  NTSTATUS status = STATUS_SUCCESS;

  // ���ظ�R3��״̬�����ݳ���
  pIrp->IoStatus.Information = 0;
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // ֪ͨ I/O ��������ǰ�� IRP �Ѿ�����ɹ�����Ҫ���ظ� R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}
