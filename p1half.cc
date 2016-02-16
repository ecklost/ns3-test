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

using namespace ns3;

int main (int argc, char *argv[])
{
  /* ** Node List **
   * 
   * 0: Satellite
   * 1: GRC
   * 2: Server
   * 3: NSA Spy Van
   * 4: DoD HQ
   *
   */
  NodeContainer nodes;
  nodes.Create (5);
  
  PointToPointHelper fastLink;
  fastLink.SetDeviceAttribute ("DataRate", StringValue ("32Mbps"));
  fastLink.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  PointToPointHelper serverLink;
  serverLink.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  serverLink.SetChannelAttribute ("Delay", StringValue ("5ms"));
  
  PointToPointHelper spyLink;
  spyLink.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  spyLink.SetChannelAttribute ("Delay", StringValue ("50ms"));
  
  PointToPointHelper voipLink;
  voipLink.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  voipLink.SetChannelAttribute ("Delay", StringValue ("3ms"));
  
  spyLink.SetQueue ("ns3::DropTailQueue", 
					"Mode", StringValue ("QUEUE_MODE_PACKETS"),
					"MaxPackets", UintegerValue (200));
					
  InternetStackHelper stack;
  stack.InstallAll ();
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer spyInterface;
  
  NetDeviceContainer devices;
  devices = fastLink.Install (nodes.Get (0), nodes.Get (1));
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  devices = serverLink.Install (nodes.Get (1), nodes.Get (2));
  address.SetBase ("10.1.3.0", "255.255.255.0");
  interfaces = address.Assign (devices);
  devices = spyLink.Install (nodes.Get (2), nodes.Get (3));
  address.SetBase ("10.1.4.0", "255.255.255.0");
  interfaces = address.Assign (devices);
  devices = voipLink.Install (nodes.Get (3), nodes.Get (4));
  address.SetBase ("10.1.5.0", "255.255.255.0");
  interfaces = address.Assign (devices);
  
  spyInterface.Add (interfaces.Get (1));
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  // TCP first
  uint16_t spyPort = 50000;
  Address DoDAddress (InetSocketAddress (Ipv4Address::GetAny (), spyPort));
  PacketSinkHelper helper ("ns3::TcpSocketFactory", DoDAddress);
  
  AddressValue remoteAddress (InetSocketAddress (spyInterface.GetAddress (0, 0), spyPort));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (128));
  BulkSendHelper ftp ("ns3::TcpSocketFactory", Address ());
  ftp.SetAttribute ("Remote", remoteAddress);
  ftp.SetAttribute ("SendSize", UintegerValue (128));
  ftp.SetAttribute ("MaxBytes", UintegerValue (100000000));
  
  ApplicationContainer sourceApp = ftp.Install (nodes.Get (0));
  sourceApp.Start (Seconds (1.0));
  sourceApp.Stop (Seconds (10.0));
  
  helper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
  ApplicationContainer spyApp = helper.Install (nodes.Get (3));
  spyApp.Start (Seconds (1.0));
  spyApp.Stop (Seconds (10.0));
  
  // Now UDP
  UdpEchoServerHelper dod (9);
  ApplicationContainer dodApp = dod.Install (nodes.Get (4));
  dodApp.Start (Seconds (1.0));
  dodApp.Stop (Seconds (10.0));
  
  UdpEchoClientHelper spyClient (spyInterface.GetAddress (0), 9);
  spyClient.SetAttribute ("MaxPackets", UintegerValue (1));
  spyClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  spyClient.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps = spyClient.Install (nodes.Get (3));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  
  fastLink.EnablePcapAll ("p1half", true);
  serverLink.EnablePcapAll ("p1half", true);
  spyLink.EnablePcapAll ("p1half", true);
  voipLink.EnablePcapAll ("p1half", true);
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
