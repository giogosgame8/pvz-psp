/* PSP CFW user-mode model query. NID and ABI are interface constants.
   References: pspdev KUBridge; PSP-Archive/ARK-4 common/include/systemctrl.h. */
.set noreorder
#include "pspimport.s"
IMPORT_START "KUBridge",0x40090000
IMPORT_FUNC "KUBridge",0x24331850,kuKernelGetModel
