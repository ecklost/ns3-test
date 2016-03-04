// LEO to GEO to Ground
// Input examples I found
//		1) LEO to GEO: 3 Mb/s, 110 ms
//		2) GEO to GND: 20 Mb/s, 125 ms
// The way Kul wants me to do it is LEO to GEO to Ground.
// First attempt should be using Wes Eddy's approach with using PointToPointHelper.
// 

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-global-routing-helper.h"

int main (int argc, char *argv[])
{
	NodeContainer satellites;
	satellites.Create (2);
	NodeContainer ground;
	ground.Create (1);
	
	PointToPointHelper leoToGeo;
	leoToGeo.SetDeviceAttribute ("DataRate", StringValue ("3Mbps"));
	leoToGeo.SetChannelAttribute ("Delay", StringValue ("110ms"));
	PointToPointHelper geoToGnd;
	geoToGnd.SetDeviceAttribute ("DataRate", StringValue ("20Mbps"));
	geoToGnd.SetChannelAttribute ("Delay", StringValue ("125ms"));
	
	NetDeviceContainer devices;
	devices = leoToGeo.Install (satellites.Get (0), satellites.Get (1));
	
	Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
	em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
	devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
	
	InternetStackHelper stack;
	stack.InstallAll ();
	Ipv4AddressHelper address;
	address.SetBase ("10.0.0.0", "255.255.255.0");
	Ipv4InterfaceContainer interface;

	address.NewNetwork ();
	Ipv4InterfaceContainer moreInterfaces = address.Assign (devices);
	
	devices = geoToGnd.Install (satellites.Get (1), ground.Get (0));
	device.Get (1)->SetAttribute ("ReceiverErrorModel", PointerValue (em));
	address.NewNetwork ();
	moreInterfaces = address.Assign (devices);
	
	interface.Add (moreInterfaces.Get (1));
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	uint16_t groundSink = 21000;
	Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), groundSink));
	PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
	
	AddressValue remoteAddress (InetSocketAddress (interface.GetAddress (0,0), groundSink));
	Config::SetDefault ("ns3::TcpSocketFactory::SegmentSize", UintegerValue (128));
	BulkSendHelper ftp ("ns3::TcpSocketFactory", Address ());
	ftp.SetAttribute ("Remote", remoteAddress);
	ftp.SetAttribute ("SendSize", UintegerValue (128));
	ftp.SetAttribute ("MaxBytes", UintegerValue (100000000));
	
	ApplicationContainer sourceApp = ftp.Install (satellites.Get (0));
	sourceApp.Start (Seconds (0));
	sourceApp.Stop (Seconds (10));
	
	sinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
	ApplicationContainer sinkApp = sinkHelper.Install (ground);
	sinkApp.Start (Seconds (0));
	sinkApp.Stop (Seconds (10));
	
	leoToGeo.EnablePcapAll ("ns3-example", true);
	geoToGnd.EnablePcapAll ("ns3-example", true);
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
