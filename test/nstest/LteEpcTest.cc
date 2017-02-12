#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/config-store.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("LteEpcTest");
//The use of EPC allows to use IPv4 networking with LTE devices.

int main(int argc, char const *argv[]) {
  NS_LOG_INFO ("Creating 1");
  Ptr<LteHelper> lteHelper= CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper=CreateObject<PointToPointEpcHelper>();
  //tell the LTE helper that the EPC will be used
  lteHelper->SetEpcHelper (epcHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode();

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);

  Ptr<Node> remoteHost =remoteHostContainer.Get(0);
  InternetStackHelper internet;
  internet.Install(remoteHostContainer);

  //Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices=p2ph.Install(pgw,remoteHost);

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0","255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces=ipv4h.Assign(internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  // Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  //It’s important to specify routes so that the remote host can reach LTE UEs.
  //One way of doing this is by exploiting the fact that the PointToPointEpcHelper will
  //by default assign to LTE UEs an IP address in the 7.0.0.0 network
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer enbNodes;
  enbNodes.Create(1);
  NodeContainer ueNodes;
  ueNodes.Create(2);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);

  //why this way failed? Program received signal SIGSEGV, Segmentation fault.
  // NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
  // NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice(enbNodes);

  //原因：必须先基站后ue的顺序，否则就会出错
  NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

   InternetStackHelper internetue;
   internetue.Install(ueNodes);


  Ptr<Node> ue;
  Ptr<NetDevice> ueLteDevice;
  Ptr<Ipv4StaticRouting> ueStaticRouting;
  Ipv4InterfaceContainer ueIpIface;
  for(uint32_t u=0;u<ueNodes.GetN();++u)
  {
    ue=ueNodes.Get(u);
    ueLteDevice = ueLteDevs.Get(u);
    ueIpIface=epcHelper->AssignUeIpv4Address(NetDeviceContainer (ueLteDevice));
    ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

  uint16_t dlPort = 1234;
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory",
                                   InetSocketAddress (Ipv4Address::GetAny (), dlPort));
  ApplicationContainer serverApps = packetSinkHelper.Install (ue);
  serverApps.Start (Seconds (0.01));
  serverApps.Stop (Seconds (9.0));

  UdpClientHelper client (ueIpIface.GetAddress (0), dlPort);
  ApplicationContainer clientApps = client.Install (remoteHost);
  clientApps.Start (Seconds (0.01));
  clientApps.Stop (Seconds (9.0));

   lteHelper->Attach(ueLteDevs,enbDevs.Get(0));


  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy();
  return 0;
}
