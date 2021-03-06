/**
 * @file rtpmidireader.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <cassert>

#include "h3/rtpmidireader.h"

#include "arm/synchronize.h"
#include "h3.h"
#include "h3_timer.h"
#include "irq_timer.h"

// Output
#include "artnetnode.h"
#include "midi.h"
#include "h3/ltcsender.h"
#include "h3/ltcoutputs.h"

// ARM Generic Timer
static volatile uint32_t nUpdatesPerSecond = 0;
static volatile uint32_t nUpdatesPrevious = 0;
static volatile uint32_t nUpdates = 0;

static uint8_t qf[8] __attribute__ ((aligned (4))) = { 0, 0, 0, 0, 0, 0, 0, 0 };

static void irq_arm_handler() {
	nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
	nUpdatesPrevious = nUpdates;
}

inline static void itoa_base10(int nArg, char *pBuffer) {
	char *p = pBuffer;

	if (nArg == 0) {
		*p++ = '0';
		*p = '0';
		return;
	}

	*p++ = '0' + (nArg / 10);
	*p = '0' + (nArg % 10);
}

RtpMidiReader::RtpMidiReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs) : m_ptLtcDisabledOutputs(pLtcDisabledOutputs) {
	assert(m_ptLtcDisabledOutputs != nullptr);

	Ltc::InitTimeCode(m_aTimeCode);
}

RtpMidiReader::~RtpMidiReader() {
	Stop();
}

void RtpMidiReader::Start() {
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(irq_arm_handler));
	irq_timer_init();

	LtcOutputs::Get()->Init();

	LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
}

void RtpMidiReader::Stop() {
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
}

void RtpMidiReader::MidiMessage(const struct midi::Message *ptMidiMessage) {
	switch (static_cast<midi::Types>(ptMidiMessage->tType)) {
	case midi::Types::TIME_CODE_QUARTER_FRAME:
		HandleMtcQf(ptMidiMessage);
		nUpdates++;
		break;
	case midi::Types::SYSTEM_EXCLUSIVE: {
		const auto *pSystemExclusive = ptMidiMessage->aSystemExclusive;
		if ((pSystemExclusive[1] == 0x7F) && (pSystemExclusive[2] == 0x7F) && (pSystemExclusive[3] == 0x01)) {
			HandleMtc(ptMidiMessage);
			nUpdates++;
		}
	}
		break;
	case midi::Types::CLOCK: {
		uint32_t nBPM;
		if (m_MidiBPM.Get(ptMidiMessage->nTimestamp, nBPM)) {
			LtcOutputs::Get()->ShowBPM(nBPM);
		}
	}
	default:
		break;
	}
}

void RtpMidiReader::HandleMtc(const struct midi::Message *ptMidiMessage) {
	const auto *pSystemExclusive = ptMidiMessage->aSystemExclusive;

	m_nTimeCodeType = static_cast<midi::TimecodeType>((pSystemExclusive[5] >> 5));

	itoa_base10((pSystemExclusive[5] & 0x1F), &m_aTimeCode[0]);
	itoa_base10(pSystemExclusive[6], &m_aTimeCode[3]);
	itoa_base10(pSystemExclusive[7], &m_aTimeCode[6]);
	itoa_base10(pSystemExclusive[8], &m_aTimeCode[9]);

	m_tLtcTimeCode.nFrames = pSystemExclusive[8];
	m_tLtcTimeCode.nSeconds = pSystemExclusive[7];
	m_tLtcTimeCode.nMinutes = pSystemExclusive[6];
	m_tLtcTimeCode.nHours = pSystemExclusive[5] & 0x1F;
	m_tLtcTimeCode.nType = static_cast<uint8_t>(m_nTimeCodeType);

	Update();
}

void RtpMidiReader::HandleMtcQf(const struct midi::Message *ptMidiMessage) {
	const auto nData1 = ptMidiMessage->nData1;
	const auto nPart = (nData1 & 0x70) >> 4;

	qf[nPart] = nData1 & 0x0F;

	m_nTimeCodeType = static_cast<midi::TimecodeType>((qf[7] >> 1));

	if (!m_ptLtcDisabledOutputs->bMidi) {
		Midi::Get()->SendQf(ptMidiMessage->nData1);
	}

	if ((nPart == 7) || (m_nPartPrevious == 7)) {
	} else {
		m_bDirection = (m_nPartPrevious < nPart);
	}

	if ( (m_bDirection && (nPart == 7)) || (!m_bDirection && (nPart == 0)) ) {
		itoa_base10(qf[6] | ((qf[7] & 0x1) << 4), &m_aTimeCode[0]);
		itoa_base10(qf[4] | (qf[5] << 4), &m_aTimeCode[3]);
		itoa_base10(qf[2] | (qf[3] << 4), &m_aTimeCode[6]);
		itoa_base10(qf[0] | (qf[1] << 4), &m_aTimeCode[9]);

		m_tLtcTimeCode.nFrames = qf[0] | (qf[1] << 4);
		m_tLtcTimeCode.nSeconds = qf[2] | (qf[3] << 4);
		m_tLtcTimeCode.nMinutes = qf[4] | (qf[5] << 4);
		m_tLtcTimeCode.nHours = qf[6] | ((qf[7] & 0x1) << 4);
		m_tLtcTimeCode.nType = static_cast<uint8_t>(m_nTimeCodeType);

		Update();
	}

	m_nPartPrevious = nPart;
}

void RtpMidiReader::Update() {
	if (!m_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct TLtcTimeCode*>(&m_tLtcTimeCode));
	}

	if (!m_ptLtcDisabledOutputs->bArtNet) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<struct TArtNetTimeCode*>(&m_tLtcTimeCode));
	}

	LtcOutputs::Get()->Update(reinterpret_cast<const struct TLtcTimeCode*>(&m_tLtcTimeCode));
}

void RtpMidiReader::Run() {
	LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct TLtcTimeCode*>(&m_tLtcTimeCode));

	dmb();
	if (nUpdatesPerSecond != 0) {
		LedBlink::Get()->SetFrequency(ltc::led_frequency::DATA);
	} else {
		LtcOutputs::Get()->ShowSysTime();
		LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
	}
}
