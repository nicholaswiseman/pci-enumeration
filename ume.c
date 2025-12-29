#define INITGUID
#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <devpkey.h>

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
        printf("*************************************\n");
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

        printf("HWID:");
        wprintf(hwid);
        printf("\n");

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

        printf("Desc:");
        wprintf(desc);
        printf("\n");

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
        const DEVPROPKEY address_property_key = DEVPKEY_Device_Address;
        DEVPROPTYPE property_type;
        UINT32 value;

        status = SetupDiGetDevicePropertyW(
            dev_info_set,
            &device_info,
            &address_property_key,
            &property_type,
            (PBYTE)&value,
            sizeof(value),
            NULL,
            0
        );

        const DEVPROPKEY bus_property_key = DEVPKEY_Device_BusNumber;
        UINT32 bus;

        status = SetupDiGetDevicePropertyW(
            dev_info_set,
            &device_info,
            &bus_property_key,
            &property_type,
            (PBYTE)&bus,
            sizeof(bus),
            NULL,
            0
        );

        UINT8 function = value & 0x7;
        UINT8 device = (value & 0xF8) >> 3;
        printf("Address:");
        printf("Bus: %d, Device: %d, Function: %d", bus, device, function);
        printf("\n");

        // get ready for the next device
        memberIdx++;
        memset(&device_info, 0, sizeof(SP_DEVINFO_DATA));
        device_info.cbSize = sizeof(SP_DEVINFO_DATA);
        printf("*************************************\n");
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
}