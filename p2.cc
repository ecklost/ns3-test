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
#include "ns3/wifi-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

int main (int argc, char *argv[]) {
  std::string transport_prot = "TcpTahoe";
  
  NodeContainer A10;
  A10.Create (1);
  NodeContainer SAT;
  SAT.Create (2); // 1 is SAT, 2 is GRD SAT
  NodeContainer WPAFB;
  WPAFB.Create (1);
 
  PointToPointHelper groundLink;
  groundLink.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  groundLink.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  groundLink.SetQueue ("ns3::DropTailQueue", 					
					   "Mode", StringValue ("QUEUE_MODE_PACKETS"), 
					   "MaxPackets", UintegerValue (2000));
  
  // Set-up wireless
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
  Ssid ssid = Ssid ("satellite-ssid");
  mac.SetType ("ns3::StaWifiMac",
			   "Ssid", SsidValue (ssid),
			   "ActiveProbing", BooleanValue (false));
			   
  NetDeviceContainer satellite;
  satellite = wifi.Install (phy, mac, SAT.Get (0));
  mac.SetType ("ns3::ApWifiMac",
			   "Ssid", SsidValue (ssid));
  NetDeviceContainer gSatellite;
  gSatellite = wifi.Install (phy, mac, SAT.Get (1));
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
								 "MinX", DoubleValue (0.0),
								 "MinY", DoubleValue (0.0),
								 "DeltaX", DoubleValue (5.0),
								 "DeltaY", DoubleValue (10.0),
								 "GridWidth", UintegerValue (3),
								 "LayoutType", StringValue ("RowFirst"));
	
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
							 "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (SAT.Get (0));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (SAT.Get (1));
  InternetStackHelper stack;
  stack.Install (A10);
  stack.Install (SAT);
  stack.Install (WPAFB);
  
  Ipv4AddressHelper address; 
  
  address.SetBase ("10.1.1.0", "255.255.255.0");  
  Ipv4InterfaceContainer interfaces;
  interfaces = address.Assign (A10);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  address.Assign (satellite);
  address.Assign (gSatellite);
  address.SetBase ("10.1.3.0", "255.255.255.0");
  interfaces = address.Assign (WPAFB);
  
  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (WPAFB.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  
  UdpEchoClientHelper echoClient (interfaces.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (2048));
  
  ApplicationContainer clientApps = echoClient.Install (SAT.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
  
}
