#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"

using namespace ns3;


int
main(int argc ,char* argv [])
{
  NodeContainer nodes;
  nodes.Create(2);
  //https://www.nsnam.org/doxygen/classns3_1_1_csma_helper.html

  CsmaHelper csmaNode;

  //https://www.nsnam.org/doxygen/classns3_1_1_csma_channel.html
/*
  Attributes

  DataRate: The transmission data rate to be provided to devices connected to the channel
  Set with class: DataRateValue
  Underlying type: DataRate
  Initial value: 4294967295bps
  Flags: construct write read
  Delay: Transmission delay through the channel
  Set with class: ns3::TimeValue
  Underlying type: Time –9223372036854775808.0ns:+9223372036854775807.0ns
  Initial value: +0.0ns
  Flags: construct write read*/

 //https://www.nsnam.org/doxygen/classns3_1_1_data_rate.html#a6d5dd08beb900977abaa4599f603c846
  csmaNode.SetChannelAttribute("DataRate",StringValue("100Mbps"));//stringValue
  //https://www.nsnam.org/doxygen/classns3_1_1_time.html#addbf69c7aec0f3fd8c0595426d88622e
  csmaNode.SetChannelAttribute("Delay",StringValue("6560ns"));

  //NetDeviceContainer ns3::CsmaHelper::Install	(	Ptr< Node > 	node	)	const

/*This method creates an ns3::CsmaChannel with the attributes configured by CsmaHelper::SetChannelAttribute, an ns3::CsmaNetDevice with the attributes configured by CsmaHelper::SetDeviceAttribute and then adds the device to the node and attaches the channel to the device.

Parameters
node	The node to install the device in
Returns
A container holding the    added net device.*/

  NetDeviceContainer csmaNetDevice;//CsmaNetDevice ->  NetDeviceContainer
  csmaNetDevice=csmaNode.Install(nodes);


  //https://www.nsnam.org/doxygen/classns3_1_1_internet_stack_helper.html#a14b0da37b1617255bf1078c11a108dce
    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(nodes);



//https://www.nsnam.org/doxygen/classns3_1_1_ipv4_address_helper.html
//
  Ipv4AddressHelper ipv4AddressHelper;
  ipv4AddressHelper.SetBase("10.1.1.0","255.255.255.0");
//Ipv4InterfaceContainer 	Assign (const NetDeviceContainer &c)
//	Assign IP addresses to the net devices specified in the container based on the current network prefix and address base.
//给的是netDevice分配地址.
  Ipv4InterfaceContainer ipv4InterfaceContainer;
  ipv4InterfaceContainer=ipv4AddressHelper.Assign(csmaNetDevice);


/*
  LogComponentEnable("UdpServer",LOG_LEVEL_INFO);
  LogComponentEnable("UdpClient",LOG_LEVEL_INFO);

  UdpServerHelper udpServerHelper;
//https://www.nsnam.org/doxygen/classns3_1_1_node_container.html#a9ed96e2ecc22e0f5a3d4842eb9bf90bf
  ApplicationContainer appsServer=udpServerHelper.Install(nodes.Get(2)); //node index from 0;
//https://www.nsnam.org/doxygen/classns3_1_1_application_container.html#a8eff87926507020bbe3e1390358a54a7
  appsServer.Start(Seconds(1.0));//add Seconds to covert 1.0 to time
  appsServer.Stop(Seconds(10.0));
//https://www.nsnam.org/doxygen/classns3_1_1_udp_client_helper.html
  UdpClientHelper udpClientHelper;
  ApplicationContainer appsClient=udpClientHelper.Install(nodes.Get(0));
  appsClient.Start(Seconds(1.0));
  appsClient.Stop(Seconds(10.0));*/

  LogComponentEnable("UdpClient",LOG_LEVEL_INFO);
  LogComponentEnable("UdpServer",LOG_LEVEL_INFO);

  UdpServerHelper server (9);
  ApplicationContainer serverApps = server.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpClientHelper client (ipv4InterfaceContainer.GetAddress (1), 9);
  client.SetAttribute ("MaxPackets", UintegerValue (2));
  client.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  client.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = client.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  //routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
