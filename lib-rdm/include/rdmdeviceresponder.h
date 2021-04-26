/**
 * @file rdmdeviceresponder.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef RDMDEVICERESPONDER_H_
#define RDMDEVICERESPONDER_H_

#include <stdint.h>

#include "rdm.h"
#include "rdmdevice.h"
#include "rdmpersonality.h"
#include "rdmsensors.h"
#include "rdmsubdevices.h"
#include "rdmfactorydefaults.h"

#include "lightset.h"

enum {
	RDM_DEFAULT_CURRENT_PERSONALITY = 1
};

class RDMDeviceResponder: public RDMDevice {
public:
	RDMDeviceResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet);

	void Init();
	void Print();

	// E120_DEVICE_INFO				0x0060
	struct TRDMDeviceInfo *GetDeviceInfo(uint16_t nSubDevice = RDM_ROOT_DEVICE);

	// E120_DEVICE_LABEL			0x0082
	void GetLabel(uint16_t nSubDevice, struct TRDMDeviceInfoData *pInfo);
	void SetLabel(uint16_t nSubDevice, const char *pLabel, uint8_t nLabelLength);

	// E120_FACTORY_DEFAULTS		0x0090
	bool GetFactoryDefaults();
	void SetFactoryDefaults();

	// E120_LANGUAGE				0x00B0
	const char* GetLanguage() const {
		return m_aLanguage;
	}
	void SetLanguage(const char aLanguage[2]);

	// E120_SOFTWARE_VERSION_LABEL	0x00C0
	const char* GetSoftwareVersion() const {
		return m_pSoftwareVersion;
	}
	uint8_t GetSoftwareVersionLength() const {
		return m_nSoftwareVersionLength;
	}

	// E120_DMX_START_ADDRESS		0x00F0
	uint16_t GetDmxStartAddress(uint16_t nSubDevice = RDM_ROOT_DEVICE);
	void SetDmxStartAddress(uint16_t nSubDevice, uint16_t nDmxStartAddress);

	// E120_SLOT_INFO				0x0120
	bool GetSlotInfo(uint16_t nSubDevice,uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo);

	uint16_t GetDmxFootPrint(uint16_t nSubDevice = RDM_ROOT_DEVICE);

	// Personalities
	RDMPersonality* GetPersonality(uint16_t nSubDevice = RDM_ROOT_DEVICE, uint8_t nPersonality = 1);
	uint8_t GetPersonalityCount(uint16_t nSubDevice = RDM_ROOT_DEVICE);
	uint8_t GetPersonalityCurrent(uint16_t nSubDevice = RDM_ROOT_DEVICE);
	void SetPersonalityCurrent(uint16_t nSubDevice, uint8_t nPersonality);

	// Handler
	void SetRDMFactoryDefaults(RDMFactoryDefaults *pRDMFactoryDefaults) {
		m_pRDMFactoryDefaults = pRDMFactoryDefaults;
	}

	static RDMDeviceResponder* Get() {
		return s_pThis;
	}

private:
	uint16_t CalculateChecksum();

private:
	RDMSensors m_RDMSensors;
	RDMSubDevices m_RDMSubDevices;
	RDMPersonality *m_pRDMPersonality;
	LightSet* m_pLightSet;
	char* m_pSoftwareVersion;
	uint8_t m_nSoftwareVersionLength;
	struct TRDMDeviceInfo m_tRDMDeviceInfo;
	struct TRDMDeviceInfo m_tRDMSubDeviceInfo;
	char m_aLanguage[2];
	//
	bool m_IsFactoryDefaults;
	uint16_t m_nCheckSum;
	uint16_t m_nDmxStartAddressFactoryDefault;
	uint8_t m_nCurrentPersonalityFactoryDefault;
	//
	RDMFactoryDefaults *m_pRDMFactoryDefaults;

	static RDMDeviceResponder *s_pThis;
};

#endif /* RDMDEVICERESPONDER_H_ */
