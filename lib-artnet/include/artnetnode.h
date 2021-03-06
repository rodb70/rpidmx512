/**
 * @file artnetnode.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETNODE_H_
#define ARTNETNODE_H_

#include <stdint.h>
#include <time.h>

#include "artnet.h"
#include "packets.h"

#include "lightset.h"
#include "ledblink.h"

#include "artnettimecode.h"
#include "artnettimesync.h"
#include "artnetrdm.h"
#include "artnetipprog.h"
#include "artnetstore.h"
#include "artnetdisplay.h"
#include "artnetdmx.h"
#include "artnettrigger.h"

#include "artnet4handler.h"

enum TArtNetNodeMaxPorts {
	ARTNET_NODE_MAX_PORTS_OUTPUT = ArtNet::MAX_PORTS * ArtNet::MAX_PAGES,
	ARTNET_NODE_MAX_PORTS_INPUT = ArtNet::MAX_PORTS
};


/**
 * Table 3 – NodeReport Codes
 * The NodeReport code defines generic error, advisory and status messages for both Nodes and Controllers.
 * The NodeReport is returned in ArtPollReply.
 */
enum TArtNetNodeReportCode {
	ARTNET_RCDEBUG,			///<
	ARTNET_RCPOWEROK,		///<
	ARTNET_RCPOWERFAIL,		///<
	ARTNET_RCSOCKETWR1,		///<
	ARTNET_RCPARSEFAIL,		///<
	ARTNET_RCUDPFAIL,		///<
	ARTNET_RCSHNAMEOK,		///<
	ARTNET_RCLONAMEOK,		///<
	ARTNET_RCDMXERROR,		///<
	ARTNET_RCDMXUDPFULL,	///<
	ARTNET_RCDMXRXFULL,		///<
	ARTNET_RCSWITCHERR,		///<
	ARTNET_RCCONFIGERR,		///<
	ARTNET_RCDMXSHORT,		///<
	ARTNET_RCFIRMWAREFAIL,	///<
	ARTNET_RCUSERFAIL     	///<
};

enum TNodeStatus {
	ARTNET_OFF,		///<
	ARTNET_STANDBY,	///<
	ARTNET_ON		///<
};

struct TArtNetNodeState {
	uint32_t ArtPollReplyCount;			///< ArtPollReply : NodeReport : decimal counter that increments every time the Node sends an ArtPollResponse.
	uint32_t IPAddressDiagSend;			///< ArtPoll : Destination IPAddress for the ArtDiag
	uint32_t IPAddressArtPoll;			///< ArtPoll : IPAddress for the ArtPoll package
	TArtNetNodeReportCode reportCode;	///< See \ref TArtNetNodeReportCode
	TNodeStatus status;					///< See \ref TNodeStatus
	uint32_t nNetworkDataLossTimeoutMillis;
	uint32_t nArtSyncMillis;			///< Latest ArtSync received time
	bool SendArtPollReplyOnChange;		///< ArtPoll : TalkToMe Bit 1 : 1 = Send ArtPollReply whenever Node conditions change.
	bool SendArtDiagData;				///< ArtPoll : TalkToMe Bit 2 : 1 = Send me diagnostics messages.
	bool IsMultipleControllersReqDiag;	///< ArtPoll : Multiple controllers requesting diagnostics
	bool IsSynchronousMode;				///< ArtSync received
	bool IsMergeMode;
	bool IsChanged;
	bool bDisableMergeTimeout;
	bool bIsReceivingDmx;
	uint8_t nActiveOutputPorts;
	uint8_t nActiveInputPorts;
	uint8_t Priority;					///< ArtPoll : Field 6 : The lowest priority of diagnostics message that should be sent.
};

struct TArtNetNode {
	uint32_t IPAddressLocal;
	uint32_t IPAddressBroadcast;
	uint32_t IPSubnetMask;
	uint32_t IPAddressRemote;						///< The remote IP Address
	uint32_t IPAddressTimeCode;
	uint8_t MACAddressLocal[ArtNet::MAC_SIZE];
	uint8_t NetSwitch[ArtNet::MAX_PAGES];			///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
	uint8_t SubSwitch[ArtNet::MAX_PAGES];			///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
	uint8_t Oem[2];
	char ShortName[ArtNet::SHORT_NAME_LENGTH];
	char LongName[ArtNet::LONG_NAME_LENGTH];
	uint8_t TalkToMe;								///< Behavior of Node
	uint8_t Status1;
	uint8_t Status2;
};

struct TGenericPort {
	uint16_t nPortAddress;		///< One of the 32,768 possible addresses to which a DMX frame can be directed. The Port-Address is a 15 bit number composed of Net+Sub-Net+Universe.
	uint8_t nDefaultAddress;	///< the address set by the hardware
	uint8_t nStatus;
};

struct TOutputPort {
	uint8_t data[ArtNet::DMX_LENGTH];	///< Data sent
	uint16_t nLength;					///< Length of sent DMX data
	uint8_t dataA[ArtNet::DMX_LENGTH];	///< The data received from Port A
	uint32_t nMillisA;					///< The latest time of the data received from Port A
	uint32_t ipA;						///< The IP address for port A
	uint8_t dataB[ArtNet::DMX_LENGTH];	///< The data received from Port B
	uint32_t nMillisB;					///< The latest time of the data received from Port B
	uint32_t ipB;						///< The IP address for Port B
	artnet::Merge mergeMode;				///< \ref artnet::Merge
	bool IsDataPending;					///< ArtDMX received and waiting for ArtSync
	bool bIsEnabled;					///< Is the port enabled ?
	TGenericPort port;					///< \ref TGenericPort
	artnet::PortProtocol tPortProtocol;		///< Art-Net 4
};

struct TInputPort {
	bool bIsEnabled;
	TGenericPort port;
	uint8_t nSequence;
	uint32_t nDestinationIp;
};

class ArtNetNode {
public:
	ArtNetNode(uint8_t nVersion = 3, uint8_t nPages = 1);
	~ArtNetNode();

	void Start();
	void Stop();

	void Run();

	uint8_t GetVersion() {
		return m_nVersion;
	}

	uint8_t GetPages() const {
		return m_nPages;
	}

	void SetOutput(LightSet *pLightSet) {
		m_pLightSet = pLightSet;
	}
	LightSet *GetOutput() const {
		return m_pLightSet;
	}

	uint8_t GetActiveInputPorts() const {
		return m_State.nActiveInputPorts;
	}

	uint8_t GetActiveOutputPorts() const {
		return m_State.nActiveOutputPorts;
	}

	void SetDirectUpdate(bool bDirectUpdate) {
		m_bDirectUpdate = bDirectUpdate;
	}
	bool GetDirectUpdate() const {
		return m_bDirectUpdate;
	}

	void SetShortName(const char *);
	const char *GetShortName() const {
		return m_Node.ShortName;
	}

	void SetLongName(const char *);
	const char *GetLongName() const {
		return m_Node.LongName;
	}

	int SetUniverse(uint8_t nPortIndex, artnet::PortDir dir, uint16_t nAddress);

	int SetUniverseSwitch(uint8_t nPortIndex, artnet::PortDir dir, uint8_t nAddress);
	bool GetUniverseSwitch(uint8_t nPortIndex, uint8_t &nAddress,artnet::PortDir dir) const;

	void SetNetSwitch(uint8_t nAddress, uint8_t nPage);
	uint8_t GetNetSwitch(uint8_t nPage) const;

	void SetSubnetSwitch(uint8_t nAddress, uint8_t nPage);
	uint8_t GetSubnetSwitch(uint8_t nPage) const;

	bool GetPortAddress(uint8_t nPortIndex, uint16_t &nAddress,artnet::PortDir dir) const;

	void SetMergeMode(uint8_t nPortIndex, artnet::Merge tMergeMode);
	artnet::Merge GetMergeMode(uint8_t nPortIndex) const;

	void SetPortProtocol(uint8_t nPortIndex, artnet::PortProtocol tPortProtocol);
	artnet::PortProtocol GetPortProtocol(uint8_t nPortIndex) const;

	void SetOemValue(const uint8_t *);
	const uint8_t *GetOemValue() const {
		return m_Node.Oem;
	}

	void SetNetworkTimeout(uint32_t nNetworkDataLossTimeout) {
		m_State.nNetworkDataLossTimeoutMillis = nNetworkDataLossTimeout * 1000;
	}
	uint32_t GetNetworkTimeout() const {
		return m_State.nNetworkDataLossTimeoutMillis / 1000;
	}

	void SetDisableMergeTimeout(bool bDisable) {
		m_State.bDisableMergeTimeout = bDisable;
	}
	bool GetDisableMergeTimeout() const {
		return m_State.bDisableMergeTimeout;
	}

	void SendDiag(const char *, TPriorityCodes);
	void SendTimeCode(const struct TArtNetTimeCode *);

	void SetTimeCodeHandler(ArtNetTimeCode *pArtNetTimeCode) {
		m_pArtNetTimeCode = pArtNetTimeCode;
	}

	void SetTimeCodeIp(uint32_t nDestinationIp);
	void SetTimeSyncHandler(ArtNetTimeSync *pArtNetTimeSync) {
		m_pArtNetTimeSync = pArtNetTimeSync;
	}

	void SetRdmHandler(ArtNetRdm *, bool isResponder = false);
	void SetIpProgHandler(ArtNetIpProg *);

	void SetArtNetStore(ArtNetStore *pArtNetStore) {
		m_pArtNetStore = pArtNetStore;
	}

	void SetArtNetDisplay(ArtNetDisplay *pArtNetDisplay) {
		m_pArtNetDisplay = pArtNetDisplay;
	}

	void SetArtNetTrigger(ArtNetTrigger *pArtNetTrigger) {
		m_pArtNetTrigger = pArtNetTrigger;
	}

	void SetArtNetDmx(ArtNetDmx *pArtNetDmx) {
		m_pArtNetDmx = pArtNetDmx;
	}
	ArtNetDmx *GetArtNetDmx() const {
		return m_pArtNetDmx;
	}

	void SetDestinationIp(uint8_t nPortIndex, uint32_t nDestinationIp);
	uint32_t GetDestinationIp(uint8_t nPortIndex) const {
		if (nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT) {
			return m_InputPorts[nPortIndex].nDestinationIp;
		}

		return 0;
	}

	void SetArtNet4Handler(ArtNet4Handler *pArtNet4Handler);

	void Print();

	static ArtNetNode* Get() {
		return s_pThis;
	}

private:
	void FillPollReply();
#if defined ( ENABLE_SENDDIAG )
	void FillDiagData(void);
#endif

	void GetType();

	void HandlePoll();
	void HandleDmx();
	void HandleSync();
	void HandleAddress();
	void HandleTimeCode();
	void HandleTimeSync();
	void HandleTodRequest();
	void HandleTodControl();
	void HandleRdm();
	void HandleIpProg();
	void HandleDmxIn();
	void HandleTrigger();

	uint16_t MakePortAddress(uint16_t, uint8_t nPage = 0);

	bool IsMergedDmxDataChanged(uint8_t, const uint8_t *, uint16_t);
	void CheckMergeTimeouts(uint8_t);
	bool IsDmxDataChanged(uint8_t, const uint8_t *, uint16_t);

	void SendPollRelply(bool);
	void SendTod(uint8_t nPortId = 0);

	void SetNetworkDataLossCondition();

private:
	uint8_t m_nVersion;
	uint8_t m_nPages;
	int32_t m_nHandle { -1 };
	LightSet *m_pLightSet { nullptr };

	ArtNetTimeCode *m_pArtNetTimeCode { nullptr };
	ArtNetTimeSync *m_pArtNetTimeSync { nullptr };
	ArtNetRdm *m_pArtNetRdm { nullptr };
	ArtNetIpProg *m_pArtNetIpProg { nullptr };
	ArtNetDmx *m_pArtNetDmx { nullptr };
	ArtNetTrigger *m_pArtNetTrigger { nullptr };

	ArtNetStore *m_pArtNetStore { nullptr };
	ArtNetDisplay *m_pArtNetDisplay { nullptr };

	ArtNet4Handler *m_pArtNet4Handler { nullptr };

	struct TArtTimeCode *m_pTimeCodeData { nullptr };
	struct TArtTodData *m_pTodData { nullptr };
	struct TArtIpProgReply *m_pIpProgReply { nullptr };

	struct TArtNetNode m_Node;
	struct TArtNetNodeState m_State;

	struct TArtNetPacket m_ArtNetPacket;
	struct TArtPollReply m_PollReply;
#if defined ( ENABLE_SENDDIAG )
	struct TArtDiagData m_DiagData;
#endif

	struct TOutputPort m_OutputPorts[ARTNET_NODE_MAX_PORTS_OUTPUT];
	struct TInputPort m_InputPorts[ARTNET_NODE_MAX_PORTS_INPUT];

	bool m_bDirectUpdate { false };

	uint32_t m_nCurrentPacketMillis { 0 };
	uint32_t m_nPreviousPacketMillis { 0 };

	TOpCodes m_tOpCodePrevious;

	bool m_IsLightSetRunning[ARTNET_NODE_MAX_PORTS_OUTPUT];
	bool m_IsRdmResponder { false };

	char m_aSysName[16];
	char m_aDefaultNodeLongName[ArtNet::LONG_NAME_LENGTH];

	static ArtNetNode *s_pThis;
};

#endif /* ARTNETNODE_H_ */
