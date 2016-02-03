/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
/*
 * TCP Source   10ms       20ms       10ms   TCP Sink
 *          O -------- O -------- O -------- O
 *              5Mb		   1Mb        5Mb
 *
 * The middlemost portion is a bottleneck link.
 * Goodput refers to data received at sink divided by simulation time.
 */
 
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

NS_LOG_COMPONENT_DEFINE ("P1 Information");
 
int
main (int argc, char *argv[])
{
  std::string transport_prot = "TcpTahoe";
  
  /*
   * Node List
   * 0 : TCP Source
   * 1 : Bottleneck Begin
   * 2 : Bottleneck Exit
   * 3 : TCP Sink
   */
  NS_LOG_INFO ("Node Creation");
  NodeContainer source;
  source.Create (1);
  NodeContainer bottleneck;
  bottleneck.Create (2);
  NodeContainer sink;
  sink.Create (1);
   
  NS_LOG_INFO ("Point-to-point Creation");
  PointToPointHelper sourceLink;
  sourceLink.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  sourceLink.SetChannelAttribute ("Delay", StringValue ("10ms"));
   
  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  bottleneckLink.SetChannelAttribute ("Delay", StringValue ("20ms"));
   
  PointToPointHelper sinkLink;
  sinkLink.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  sinkLink.SetChannelAttribute ("Delay", StringValue ("10ms"));
   
  bottleneckLink.SetQueue ("ns3::DropTailQueue", 
						   "Mode", StringValue ("QUEUE_MODE_PACKETS"), 
						   "MaxPackets", UintegerValue (2000));
   
  InternetStackHelper stack;
  stack.InstallAll ();
  
  Ipv4AddressHelper address; 
  address.SetBase ("10.0.0.0", "255.255.255.0");
  
  Ipv4InterfaceContainer sinkInterface;
   
  // Connecting nodes
  NetDeviceContainer devices;
   
  devices = sourceLink.Install (source.Get (0), bottleneck.Get (0));
  address.NewNetwork ();
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  devices = bottleneckLink.Install (bottleneck.Get (0), bottleneck.Get(1));
  address.NewNetwork ();
  interfaces = address.Assign (devices);
  devices = sinkLink.Install (bottleneck.Get(1), sink.Get (0));
  address.NewNetwork ();
  interfaces = address.Assign (devices);
   
  sinkInterface.Add (interfaces.Get (1));
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
   
  uint16_t sinkPort = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
   
   // Configure app.
  AddressValue remoteAddress (InetSocketAddress (sinkInterface.GetAddress (0, 0), sinkPort));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (128));
  BulkSendHelper ftp ("ns3::TcpSocketFactory", Address ());
  ftp.SetAttribute ("Remote", remoteAddress);
  ftp.SetAttribute ("SendSize", UintegerValue (128));
  ftp.SetAttribute ("MaxBytes", UintegerValue (100000000));
   
  ApplicationContainer sourceApp = ftp.Install (source.Get (0));
  sourceApp.Start (Seconds (0));
  sourceApp.Stop  (Seconds (0.1));
   
  sinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
  ApplicationContainer sinkApp = sinkHelper.Install (sink);
  sinkApp.Start (Seconds (0));
  sinkApp.Stop  (Seconds (0.2));
  
  sourceLink.EnablePcapAll ("p1", true);
  bottleneckLink.EnablePcapAll ("p1", true);
  sinkLink.EnablePcapAll ("p1", true);
   
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
