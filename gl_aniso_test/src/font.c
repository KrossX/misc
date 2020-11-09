/* Copyright (c) 2020 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

// Font based on the PxPlus IBM CGA Font from
// "The Ultimate Oldschool PC Font Pack" http://int10h.org/oldschool-pc-fonts/
// Rendered into an image (256x56) and converted to char array
//
// The Ultimate Oldschool PC Font Pack is licensed under a Creative Commons
// Attribution-ShareAlike 4.0 International License.
//
// You should have received a copy of the license along with this work. If
//  not, see < http://creativecommons.org/licenses/by-sa/4.0/ >.
//
//                                                           (c) 2016 VileR
#include "main.h"

char pxplus_ibm_cga[128*64];
char pxplus_ibm_cga_enc[1040] =
"0mWDsm00SO0660000000W1W7sOZFZPZ1CmWPC0000036UOpV3C373O06ym000006ym00su16k1W1Oy|F"
"0y30Cm330y7CCi306W1FC0000O0600WDVOsC0m03cn030mm00m00smmOk1066000C003100000000000"
"00000O0000WFCuX7uy37|uX700060OW7ZvmCpmp06CpCpm03C003pC73m0ZDVC0CpC33COmFO0pUCm17"
"p0p7OuXF00m00036lnW1my7CpmmCm0006006CS63pC3CpC33pW13CmmFC0WF|yZ7uvX7CuX3Cm060O03"
"00000000000000W100000u33|mp7|z7Fpu1UdzmOZnnOUOcPsOaHcD33mOc1tTcDxDZP3Oc5MCmCC0ZD"
"6ytRZjtC{CWPUun0|m0CUOmVxDsU|Os0cPX5pD33pOZHhDtO3CZPcPZH6OsCCCZPcDsOsunC|mp7|z0V"
"puX7dztOZn1000000000000000000000|unFUypCpCsOpyd73u120OsCcDJBpCpOZDpO6O06S0WPpOs1"
"CCpCZPpCnO03OO30{CZFEmmCpi67UWX1OWnO0OmEsW33pCpVSm0J603600W1UOsCCCZ7tP33cP0OO000"
"FWpPUunFCCsOUyd70v10000000000000000000000yF30S00u0070S03mSW30000C0W10030s0W100W1"
"C0000WX76u1CUOWRsu0CcnmCVu10mupC{Cp3pu63mO33|DpC0uZP3CpF6CZPC0Z7CytCp0mCcDpC3OWF"
"cnmCsmmQpC30kjZ7kvn3mSc7pSc7ZDZ7000000000y100u1000000000000200000000uWn1k1000000"
"C00000000m06Ci30xusE{upCpCsOpy33Om000OsCkD03pCpQsCJ6700E00WPpOc7CCpC|nnCCm06C000"
"{uZ1mmoCUydD{O23Om000O0CFy16knWDZ1pFuWn100m3u100000000m7000000000000000000000000";

void font_decode_custom_base64(void)
{
	int i, j = 0;

	for(i = 0; i < 1040; i++) {
		char c = pxplus_ibm_cga_enc[i];
		c = c >= 'a'? c - 'a' + 36 : c >= 'A'? c - 'A' + 10 : c - '0';

		pxplus_ibm_cga[j++] = ((c >> 0)&1)*0xFF;
		pxplus_ibm_cga[j++] = ((c >> 1)&1)*0xFF;
		pxplus_ibm_cga[j++] = ((c >> 2)&1)*0xFF;
		pxplus_ibm_cga[j++] = ((c >> 3)&1)*0xFF;
		pxplus_ibm_cga[j++] = ((c >> 4)&1)*0xFF;
		pxplus_ibm_cga[j++] = ((c >> 5)&1)*0xFF;
	}
}

GLuint load_font_texture(void)
{
	GLuint texture = 0;

	font_decode_custom_base64();

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, 128, 64, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pxplus_ibm_cga);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}