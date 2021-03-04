// Tester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ViGEm/Client.h>

#include <cmath>

char device_path[MAX_PATH] = "\\\\?\\hid#vid_0810&pid_0001&col02#6&17f5bc45&0&0001#{4d1e55b2-f16f-11cf-88cb-001111000030}";
HANDLE gamepad = INVALID_HANDLE_VALUE;

DWORD bytes;
BYTE controller_in[16];
BYTE controller_out[16];

int reopen_device(void)
{
	if(gamepad != INVALID_HANDLE_VALUE)
		CloseHandle(gamepad);

	gamepad = CreateFileA(device_path, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	return gamepad != INVALID_HANDLE_VALUE;
}

VOID CALLBACK notification(PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber)
{
	float lm = LargeMotor / 255.0f - 1.0f;
	float sm = SmallMotor / 255.0f - 1.0f;

	//lm = 1.0f - lm * lm;
	//sm = 1.0f - sm * sm;

	lm = sqrt(1.0f - lm * lm);
	sm = sqrt(1.0f - sm * sm);

	controller_out[0] = 0x02;
	controller_out[3] = BYTE(lm*127.0f);
	controller_out[4] = BYTE(sm*127.0f);

	if(!WriteFile(gamepad, controller_out, 5, &bytes, NULL))
		reopen_device();
}

float analog_linearity(float data)
{
	float sign = data < 0 ? -1 : 1;
	return data * data * sign;
}

int hid_guardian_whitelist(void)
{
	char regbuffer[512];

	HANDLE regfile = CreateFileA("hidguardian.reg", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(regfile == INVALID_HANDLE_VALUE) return -1;

	int regcount = wsprintfA(regbuffer,
		"Windows Registry Editor Version 5.00\r\n\r\n"
		"[-HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\HidGuardian\\Parameters\\Whitelist]\r\n"
		"[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\HidGuardian\\Parameters\\Whitelist\\%d]\r\n",
		GetCurrentProcessId());

	DWORD written;
	WriteFile(regfile, regbuffer, regcount, &written, NULL);
	CloseHandle(regfile);

	WinExec("regedit /s hidguardian.reg", 0);
	DeleteFileA("hidguardian.reg");
	return 0;
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, char *cmdline, int cmdshow)
{
	hid_guardian_whitelist();
	reopen_device();

    const auto client = vigem_alloc();
    auto ret = vigem_connect(client);

    const auto x360 = vigem_target_x360_alloc();
	ret = vigem_target_add(client, x360);
	ret = vigem_target_x360_register_notification(client, x360, &notification);

    XUSB_REPORT report;
    XUSB_REPORT_INIT(&report);

    while(1)
    {
		if(!ReadFile(gamepad, controller_in, 9, &bytes, NULL) && !reopen_device())
			break;

		report.wButtons = 0;

		switch(controller_in[5] & 0xF) {
		case 0: report.wButtons |= XUSB_GAMEPAD_DPAD_UP; break;
		case 1: report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
				report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT; break;
		case 2: report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT; break;
		case 3: report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;
				report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN; break;
		case 4: report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN; break;
		case 5: report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
				report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT; break;
		case 6: report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT; break;
		case 7: report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
				report.wButtons |= XUSB_GAMEPAD_DPAD_UP; break;
		}

		if(controller_in[5] & 0x10) report.wButtons |= XUSB_GAMEPAD_Y;
		if(controller_in[5] & 0x20) report.wButtons |= XUSB_GAMEPAD_B;
		if(controller_in[5] & 0x40) report.wButtons |= XUSB_GAMEPAD_A;
		if(controller_in[5] & 0x80) report.wButtons |= XUSB_GAMEPAD_X;

		if(controller_in[6] & 0x01) report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
		if(controller_in[6] & 0x02) report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;
		if(controller_in[6] & 0x10) report.wButtons |= XUSB_GAMEPAD_BACK;
		if(controller_in[6] & 0x20) report.wButtons |= XUSB_GAMEPAD_START;
		if(controller_in[6] & 0x40) report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;
		if(controller_in[6] & 0x80) report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;

		report.bLeftTrigger  = ((controller_in[6] & 0x04) >> 2) * 0xFF;
		report.bRightTrigger = ((controller_in[6] & 0x08) >> 3) * 0xFF;
		report.sThumbLX =  analog_linearity((controller_in[3] - 0x80) / 128.0f) * 32767;
		report.sThumbLY =  analog_linearity((0x80 - controller_in[4]) / 128.0f) * 32767;
		report.sThumbRX =  analog_linearity((controller_in[1] - 0x80) / 128.0f) * 32767;
		report.sThumbRY =  analog_linearity((0x80 - controller_in[2]) / 128.0f) * 32767;

        vigem_target_x360_update(client, x360, report);
    }

    vigem_target_x360_unregister_notification(x360);
    vigem_target_remove(client, x360);
    vigem_target_free(x360);

    vigem_free(client);

	if(gamepad != INVALID_HANDLE_VALUE)
		CloseHandle(gamepad);

	return 0;
}
