/*
   This IPv4 example listens for incoming packets and automatically adds destinations
   for new sources.
*/

#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpsourcedata.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

using namespace jrtplib;

void nvex_set_target_addr(uint32_t ip, uint16_t port);
//
// This function checks if there was a RTP error. If so, it displays an error
// message and exists.
//

void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

//
// The new class routine
//

class MyRTPSession : public RTPSession
{
protected:
	void OnNewSource(RTPSourceData *dat)
	{
		if (dat->IsOwnSSRC())
			return;

		uint32_t ip;
		uint16_t port;
		
		if (dat->GetRTPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort();
		}
		else if (dat->GetRTCPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort()-1;
		}
		else
			return;
		
		RTPIPv4Address dest(ip,port);
		AddDestination(dest);

		struct in_addr inaddr;
		inaddr.s_addr = htonl(ip);
		std::cout << "Adding destination " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;
		// wav_write = std::make_unique<CWaveFileWrite>("server.wav", 48000, 1, 32, true);
	}

	void OnBYEPacket(RTPSourceData *dat)
	{
		if (dat->IsOwnSSRC())
			return;
		
		uint32_t ip;
		uint16_t port;
		
		if (dat->GetRTPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort();
		}
		else if (dat->GetRTCPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort()-1;
		}
		else
			return;
		
		RTPIPv4Address dest(ip,port);
		DeleteDestination(dest);
		
		struct in_addr inaddr;
		inaddr.s_addr = htonl(ip);
		std::cout << "Deleting destination " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;
		//wav_write->commitFile();
		//wav_write.reset();
	}

	void OnRemoveSource(RTPSourceData *dat)
	{
		if (dat->IsOwnSSRC())
			return;
		if (dat->ReceivedBYE())
			return;
		
		uint32_t ip;
		uint16_t port;
		
		if (dat->GetRTPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort();
		}
		else if (dat->GetRTCPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort()-1;
		}
		else
			return;
		
		RTPIPv4Address dest(ip,port);
		DeleteDestination(dest);
		
		struct in_addr inaddr;
		inaddr.s_addr = htonl(ip);
		std::cout << "Deleting destination " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;
		//wav_write->commitFile();
		//wav_write.reset();
	}
};

//
// The main routine
// 

int main(void)
{
#ifdef RTP_SOCKETTYPE_WINSOCK
	WSADATA dat;
	WSAStartup(MAKEWORD(2,2),&dat);
#endif // RTP_SOCKETTYPE_WINSOCK
	
	MyRTPSession sess;
	uint16_t portbase;
	std::string ipstr;
	int status,i,num;

        // First, we'll ask for the necessary information
		
	std::cout << "Enter local portbase:" << std::endl;
	//std::cin >> portbase;
	portbase = 1236;
	std::cout << portbase;
	std::cout << std::endl;
	
	std::cout << std::endl;
	//std::cout << "Number of seconds you wish to wait:" << std::endl;
	//std::cin >> num;
	num = 50000;
	
	// Now, we'll create a RTP session, set the destination
	// and poll for incoming data.
	
	RTPUDPv4TransmissionParams transparams;
	RTPSessionParams sessparams;

	// test
	sessparams.SetUsePollThread(true);
	sessparams.SetSessionBandwidth(48000 * sizeof(float) * 10);
	// ~test
	
	// IMPORTANT: The local timestamp unit MUST be set, otherwise
	//            RTCP Sender Report info will be calculated wrong
	// In this case, we'll be just use 8000 samples per second.
	sessparams.SetOwnTimestampUnit(1.0/8000.0);		
        const int max_send_receive_packet_size_ = 5000;
        sessparams.SetMaximumPacketSize(max_send_receive_packet_size_);
	
	sessparams.SetAcceptOwnPackets(true);
	transparams.SetPortbase(portbase);
	status = sess.Create(sessparams,&transparams);	
	checkerror(status);

	struct pkt_info_t {
	  std::vector<uint8_t> payload;
	  uint32_t sender_ip;
	  uint16_t sender_port;
	};
 
	int stat_received_packets = 0;
	int stat_sent_packets = 0;

	printf("Server started: portbase: %d\n", portbase);
	std::vector<pkt_info_t> pending_list;
	for (i = 1 ; i <= num ; /* i++ */)
	{
	  for (auto& tmp: pending_list) {
	    nvex_set_target_addr(tmp.sender_ip, tmp.sender_port);
	    //status = sess.SendPacket((void*)"0987654321", 10, 0, false, 10);

	    //std::vector<float> fbuf(tmp.size() / sizeof(float));
	    //float *t = (float*) tmp.data();
	    //for (int i = 0; i < tmp.size() / sizeof(float); ++i) {
	      // amplify
	      //t[i] *= 2.0f;
	    //}
	    
	    status = sess.SendPacket(tmp.payload.data(), tmp.payload.size(), 0, false, 10);
	    if (status != 0) {
	      printf("SendPacket failed: sz: %d\n", tmp.payload.size()); 
	    }

	    printf("SendPacket status: %d Sz: %d count: %d ip: %x port: %d\n",
				status, tmp.payload.size(), ++stat_sent_packets, tmp.sender_ip, tmp.sender_port);
	    //if (!(stat_sent_packets % 100))
	    //printf("[selected] SendPacket status: %d Sz: %d count: %d\n", status, tmp.payload.size(), stat_sent_packets);

	    stat_sent_packets++;

	    //printf("SendPacket status: %d\n", status);
	    checkerror(status);
	    //printf("Status checked\n");
	  }
	  pending_list.clear();
	  
	  sess.BeginDataAccess();
		
	  // check incoming packets
	  if (sess.GotoFirstSourceWithData())
		{
			do
			{
				RTPPacket *pack;
				
				while ((pack = sess.GetNextPacket()) != NULL)
				{
					// You can examine the data here
					// printf("Got packet !\n");
					// printf("%d: Got packet. Len: %u !\n", stat_received_packets, pack->GetPayloadLength());
					
					// we don't longer need the packet, so
					// we'll delete it
				  stat_received_packets++;

				  if (!(stat_received_packets % 100))
				    printf("[selected] %d: Got packet. Len: %u !\n", stat_received_packets, pack->GetPayloadLength());

				  pkt_info_t pkt_info;
				  // std::vector<uint8_t> tmp;
				  pkt_info.payload.resize(pack->GetPayloadLength());
				  memcpy(pkt_info.payload.data(), pack->GetPayloadData(), pack->GetPayloadLength());
				  {
					uint32_t ip; uint16_t port;
					auto dat = sess.GetCurrentSourceInfo();
					if (dat->GetRTPDataAddress() != 0)
					{
						const RTPIPv4Address* addr = (const RTPIPv4Address*)(dat->GetRTPDataAddress());
						ip = addr->GetIP();
						port = addr->GetPort();
					}
					else if (dat->GetRTCPDataAddress() != 0)
					{
						const RTPIPv4Address* addr = (const RTPIPv4Address*)(dat->GetRTCPDataAddress());
						ip = addr->GetIP();
						port = addr->GetPort() - 1;
					}
					else {
						printf("Invalid data at %s: %d\n", __FILE__, __LINE__);
						abort();
					}
						pkt_info.sender_ip = ip;
						pkt_info.sender_port = port;
				  }
				  pending_list.push_back(std::move(pkt_info));

				  // save the packet for later echo
				  //std::vector<uint8_t> tmp;
				  //tmp.resize(pack->GetPayloadLength());
				  //memcpy(tmp.data(), pack->GetPayloadData(), pack->GetPayloadLength());
				  //pending_list.push_back(std::move(tmp));
				  
				  sess.DeletePacket(pack);
				  break;
				}
			} while (sess.GotoNextSourceWithData());
		}
		
		sess.EndDataAccess();

#ifndef RTP_SUPPORT_THREAD
		status = sess.Poll();
		checkerror(status);
#endif // RTP_SUPPORT_THREAD
		
		//RTPTime::Wait(RTPTime(1,0));
		RTPTime::Wait(RTPTime(0,1000));
	}
	
	sess.BYEDestroy(RTPTime(10,0),0,0);

#ifdef RTP_SOCKETTYPE_WINSOCK
	WSACleanup();
#endif // RTP_SOCKETTYPE_WINSOCK
	return 0;
}

