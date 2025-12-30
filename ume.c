#define INITGUID
#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <devpkey.h>

typedef struct 
{
    UINT8 bus;
    UINT8 device;
    UINT8 function;
} BDF;

BDF seenBdfs[2048];
UINT8 bdfCount = 0;

UINT32 GetVendorId(WCHAR* hwid)
{
    UINT32 vendorId = 0;
    WCHAR *pVen = wcsstr(hwid, L"VEN_");
    if (pVen) swscanf(pVen, L"VEN_%4x", &vendorId);
    return vendorId;
}

UINT32 GetDeviceId(WCHAR* hwid)
{
    UINT32 devId = 0;
    WCHAR *pDev = wcsstr(hwid, L"DEV_");
    if (pDev) swscanf(pDev, L"DEV_%4x", &devId);
    return devId;
}

BOOL BdfWasSeen(BDF bdf)
{
    BOOL wasSeen = FALSE;

    if(bdfCount >= 2048)
    {
        printf("!!ERROR - seenBdfs[] is full");
        return TRUE;
    }

    for(int i = 0; i < bdfCount; i++ )
    {
        wasSeen = 
        (bdf.bus == seenBdfs[i].bus)
        && (bdf.device == seenBdfs[i].device)
        &&  (bdf.function == seenBdfs[i].function);
        if(wasSeen)
        {
            break;
        }
    }
    if(FALSE == wasSeen)
    {
        seenBdfs[bdfCount] = bdf;
        bdfCount++;
    }

    return wasSeen;
}

BDF GetBdf(HDEVINFO dev_info_set, PSP_DEVINFO_DATA pDevice_info)
{
    BDF bdf = {0};
    const DEVPROPKEY address_property_key = DEVPKEY_Device_Address;
    const DEVPROPKEY bus_property_key = DEVPKEY_Device_BusNumber;
    DEVPROPTYPE property_type;
    UINT32 address;
    BOOL status;
    UINT32 bus;

    /*
    WINSETUPAPI BOOL SetupDiGetDevicePropertyW(
      [in]            HDEVINFO         DeviceInfoSet,
      [in]            PSP_DEVINFO_DATA DeviceInfoData,
      [in]            const DEVPROPKEY *PropertyKey,
      [out]           DEVPROPTYPE      *PropertyType,
      [out, optional] PBYTE            PropertyBuffer,
      [in]            DWORD            PropertyBufferSize,
      [out, optional] PDWORD           RequiredSize,
      [in]            DWORD            Flags
    );
    */
    status = SetupDiGetDevicePropertyW(
        dev_info_set,
        pDevice_info,
        &address_property_key,
        &property_type,
        (PBYTE)&address,
        sizeof(address),
        NULL,
        0
    );

    bdf.function = address & 0x7;
    bdf.device = (address & 0xF8) >> 3; 

    status = SetupDiGetDevicePropertyW(
        dev_info_set,
        pDevice_info,
        &bus_property_key,
        &property_type,
        (PBYTE)&bus,
        sizeof(bus),
        NULL,
        0
    );

    bdf.bus = bus;

    return bdf;
}

int main(void)
{
    /*
    WINSETUPAPI HDEVINFO SetupDiGetClassDevsW(
    [in, optional] const GUID *ClassGuid,
    [in, optional] PCWSTR     Enumerator,
    [in, optional] HWND       hwndParent,
    [in]           DWORD      Flags
    );
    */

    HDEVINFO dev_info_set = SetupDiGetClassDevsW(
        NULL,
        L"PCI",
        NULL,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );

    /*
    WINSETUPAPI BOOL SetupDiEnumDeviceInfo(
    [in]  HDEVINFO         DeviceInfoSet,
    [in]  DWORD            MemberIndex,
    [out] PSP_DEVINFO_DATA DeviceInfoData
    );
    */
    SP_DEVINFO_DATA device_info = {0};
    device_info.cbSize = sizeof(SP_DEVINFO_DATA);
    DWORD memberIdx = 0;
    while(SetupDiEnumDeviceInfo(dev_info_set, memberIdx, &device_info))
    {
        /*
        WINSETUPAPI BOOL SetupDiGetDeviceRegistryPropertyW(
          [in]            HDEVINFO         DeviceInfoSet,
          [in]            PSP_DEVINFO_DATA DeviceInfoData,
          [in]            DWORD            Property,
          [out, optional] PDWORD           PropertyRegDataType,
          [out, optional] PBYTE            PropertyBuffer,
          [in]            DWORD            PropertyBufferSize,
          [out, optional] PDWORD           RequiredSize
        );
        */

        BOOL status;
        DWORD regtype = 0;
        WCHAR hwid[4096];
        //get hardwareID
        status = SetupDiGetDeviceRegistryPropertyW(
            dev_info_set,
            &device_info,
            SPDRP_HARDWAREID,
            &regtype,
            (PBYTE)hwid,
            sizeof(hwid),
            NULL
        );

        UINT32 vendorId = GetVendorId(hwid);
        UINT32 deviceId = GetDeviceId(hwid);

        //get the description
        WCHAR desc[255];
        status = SetupDiGetDeviceRegistryPropertyW(
            dev_info_set,
            &device_info,
            SPDRP_DEVICEDESC,
            &regtype,
            (PBYTE)desc,
            sizeof(desc),
            NULL
        );

        BDF bdf = GetBdf(dev_info_set, &device_info);

        if(!BdfWasSeen(bdf))
        {
            GetVendorId(hwid);
            GetDeviceId(hwid);
            printf("%x:%x:%x  ", bdf.bus, bdf.device, bdf.function);
            printf("%x:%x  ", vendorId, deviceId);
            wprintf(desc);
            printf("\n");
            //printf("*************************************\n");
        }

        // get ready for the next device
        memberIdx++;
        memset(&device_info, 0, sizeof(SP_DEVINFO_DATA));
        device_info.cbSize = sizeof(SP_DEVINFO_DATA);
    };

    DWORD error = GetLastError();
    if(error == ERROR_NO_MORE_ITEMS)
    {
        printf("Done... %d devices found\n", memberIdx-1);
    }
    else
    {
        printf("!!FAIL - SetupDiEnumDeviceInfo failed %d\n", error);
    }

    SetupDiDestroyDeviceInfoList(dev_info_set);
    return 0;
}