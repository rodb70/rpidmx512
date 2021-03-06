/**
 * @file artnetnodehandlesync.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "artnetnode.h"
#include "artnet.h"

#include "hardware.h"

using namespace artnet;

void ArtNetNode::HandleSync() {
	m_State.IsSynchronousMode = true;
	m_State.nArtSyncMillis = Hardware::Get()->Millis();

	for (uint32_t i = 0; i < (m_nPages * ArtNet::MAX_PORTS); i++) {
		if ((m_OutputPorts[i].tPortProtocol == PortProtocol::ARTNET)
				&& ((m_OutputPorts[i].IsDataPending) || (m_OutputPorts[i].bIsEnabled && m_bDirectUpdate))) {
#if defined ( ENABLE_SENDDIAG )
			SendDiag("Send pending data", ARTNET_DP_LOW);
#endif
			m_pLightSet->SetData(i, m_OutputPorts[i].data, m_OutputPorts[i].nLength);

			if (!m_IsLightSetRunning[i]) {
				m_pLightSet->Start(i);
				m_IsLightSetRunning[i] = true;
			}

			m_OutputPorts[i].IsDataPending = false;
		}
	}
}
