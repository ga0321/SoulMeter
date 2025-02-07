#pragma once

#include "pch.h"

#include ".\Third Party\PcapPlusPlus\header\TcpReassembly.h"
#include ".\Third Party\PcapPlusPlus\header\PcapLiveDeviceList.h"
#include ".\Third Party\PcapPlusPlus\header\PcapLiveDevice.h"
#include ".\Third Party\PcapPlusPlus\header\SystemUtils.h"
#include ".\Third Party\PcapPlusPlus\header\PcapPlusPlusVersion.h"
#include ".\Third Party\PcapPlusPlus\header\LRUList.h"
#include ".\Third Party\PcapPlusPlus\header\TCPLayer.h"
using namespace std;

#define NPCAP MyNpcap::getInstance()

// https://npcap.com/guide/wpcap/pcap-filter.html
#define NPCAP_FILTER_RULE "ip and (src port 10200 or dst port 10200)"

#if _DEBUG
#define DEBUG_NPCAP_REASSEMBLY 0
#endif

static BOOL _stopNpcap = FALSE;

struct TcpReassemblyCookie
{
	BOOL _isRecv;
	uint8_t* _remainingData;
	size_t _remainingSize;
	uint32_t _flowKey;
};

struct CaptureInfo
{
	pcpp::TcpReassembly* recvReassembly;
	pcpp::TcpReassembly* sendReassembly;
};

class MyNpcap : public Singleton<MyNpcap> {
private:
	struct ThreadInfo
	{
		MyNpcap* _this;
		pcpp::PcapLiveDevice* dev;
		std::string bpfFilter;
	};

	DWORD LoadNpcapDlls();

	VOID sniffAllInterface(string bpfFilter);

	static DWORD doTcpReassemblyOnLiveTraffic(LPVOID param);

	BOOL _inited = FALSE;

	static DWORD StopSniffThread(LPVOID Param);

	INT32 _stopIfCount = 0;

	mutex _mutex;
public:
	MyNpcap() { }
	~MyNpcap() {
		_stopNpcap = true;
		BOOL a = _mutex.try_lock();
		_mutex.unlock();
	}

	DWORD Init();
	VOID StopSniffAllInterface();
	VOID AddStopIfCount(BOOL clear = FALSE)
	{
		if (_stopNpcap)
			return;

		_mutex.lock();
		{
			if (clear)
				_stopIfCount = 0;
			else
				_stopIfCount++;
			_mutex.unlock();
		}
	}
};

static void onPacketArrives(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* tcpReassemblyCookie);

static VOID tcpReassemblyMsgReadyCallback(int8_t sideIndex, const pcpp::TcpStreamData& tcpData, void* userCookie);
static VOID tcpReassemblyConnectionStartCallback(const pcpp::ConnectionData& connectionData, void* userCookie);
static VOID tcpReassemblyConnectionEndCallback(const pcpp::ConnectionData& connectionData, pcpp::TcpReassembly::ConnectionEndReason reason, void* userCookie);