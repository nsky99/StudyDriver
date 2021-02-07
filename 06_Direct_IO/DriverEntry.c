#include <wdm.h>

// 卸载函数
VOID DriverUnload(PDRIVER_OBJECT pDriver);

// 默认IRP分发
NTSTATUS IrpDefaultDispath(PDEVICE_OBJECT pDevice, PIRP pIrp);

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
  UNREFERENCED_PARAMETER(pRegPath);

  // 设置卸载函数
  pDriver->DriverUnload = DriverUnload;

  // DriverEntry返回值
  NTSTATUS status = STATUS_SUCCESS;




  return status;
}

NTSTATUS IrpDefaultDispath(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
  UNREFERENCED_PARAMETER(pDevice);

  // DriverEntry返回值
  NTSTATUS status = STATUS_SUCCESS;

  // 返回给R3的状态和数据长度
  pIrp->IoStatus.Information = 0;
  pIrp->IoStatus.Status = STATUS_SUCCESS;

  // 通知 I/O 管理器当前的 IRP 已经处理成功，需要返回给 R3
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return status;
}
