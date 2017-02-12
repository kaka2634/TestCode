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

NS_LOG_COMPONENT_DEFINE ("Epc-lte-urban");
uint16_t packetSend=0;
uint16_t packetReceive=0;
double m_delay=0;
double m_jitter=0;
uint32_t bytesTotal=0;
DelayJitterEstimation m_delayEstimation;
void Rxpacket(std::string context,Ptr< const Packet > packet, const Address &address)
{
   packetReceive++;
   bytesTotal += packet->GetSize ();
   m_delayEstimation.RecordRx(packet);
   m_delay += m_delayEstimation.GetLastDelay().GetSeconds();
   m_jitter += m_delayEstimation.GetLastJitter();
   std::cout<<"recevie a packet"<<std::endl;
}
void TxPacket(std::string context,Ptr<const Packet> pkt)
{
   packetSend++;
   m_delayEstimation.PrepareTx(pkt);
}

int
main (int argc, char *argv[])
{
  RngSeedManager::SetSeed(8);
  uint16_t numberOfenbs = 16;
  uint16_t numberOfues =  68;
  double simTime = 20.5;
  double appstopTime = 20;
  extern uint16_t packetSend;
  extern uint16_t packetReceive;
  extern double m_delay;
  extern double m_jitter;
  extern uint32_t bytesTotal;
  extern DelayJitterEstimation m_delayEstimation;
  // Command line arguments
  CommandLine cmd;
  cmd.Parse(argc, argv);
  RngSeedManager::SetSeed(2);
  Config::SetDefault  ("ns3::OnOffApplication::PacketSize",StringValue ("300"));
  std::string rate ("9600bps");
  Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (rate));
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (2000000));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity",StringValue ("160"));

  //Config::SetDefault ("ns3::FdMtFfMacScheduler::HarqEnabled", BooleanValue (false));

  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (46));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (10));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);//告诉lte要使用EPC。

  lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
  /*lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold",
                                          UintegerValue (30));
  lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset",
                                          UintegerValue (1)); */
  lteHelper->SetSchedulerType ("ns3::FdMtFfMacScheduler");
  lteHelper->SetAttribute ("FfrAlgorithm", StringValue ("ns3::LteFrHardAlgorithm"));
  uint8_t bandwidth = 25;
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth));



  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
  lteHelper->SetUeAntennaModelType("ns3::IsotropicAntennaModel");

  Ptr<Node> pgw = epcHelper->GetPgwNode (); //创建PGW节点并配置，以便它处理来自或者前往lte无线接入网的数据。

  // 创建一个远程主机
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // 创建 internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // 接口 0 为本地主机，1 为 p2p 设备
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  //指定路由，以便远程主机可以到达 LTE 用户。PointToPointEpcHelper 默认会分配 LTE 用户一个7.0.0.0 网络的 IP 地址。
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  //创建UE和eNode B节点
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfenbs);
  ueNodes.Create(numberOfues);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);


  // 在用户上安装 IP 协议栈
  InternetStackHelper internetue;
  internetue.Install (ueNodes);

  // 给用户分配 IP 地址
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> ue = ueNodes.Get (u);
    // 为用户设置默认网关
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }


  // Install and start applications on UEs and remote host
  uint16_t ulPort = 2000;
   ApplicationContainer clientApps;
      ApplicationContainer serverApps;
  OnOffHelper onofful ("ns3::UdpSocketFactory",Address ());
  onofful.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.25]"));
  onofful.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.25]"));



  for (uint32_t u = 0; u < ueNodes.GetN ();u++)
    {
      ulPort++;

      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
      AddressValue remoteAddress1 (InetSocketAddress(internetIpIfaces.GetAddress (1), ulPort));
      onofful.SetAttribute ("Remote", remoteAddress1);
      clientApps.Add (onofful.Install (ueNodes.Get (u)));

    }
      serverApps.Start (Seconds (0.001));
      serverApps.Stop (Seconds (simTime));
      clientApps.Start (Seconds (0.01));
      clientApps.Stop (Seconds (appstopTime));

  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();
  // attach one or more UEs to a strongest cell
  lteHelper->Attach (ueLteDevs);

  // Add X2 inteface
  lteHelper->AddX2Interface (enbNodes);

  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                   MakeCallback (&Rxpacket));
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx",
                   MakeCallback (&TxPacket));

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  std::cout<<"packet send = "<<packetSend<<std::endl;
  std::cout<<"packet receive = "<<packetReceive<<std::endl;
  std::cout<<"packet delivery ratio = "<<(double)packetReceive/packetSend<<std::endl;
  std::cout<<"all delay = "<<m_delay<<std::endl;
  std::cout<<"delay = "<<m_delay/packetReceive<<std::endl;
  std::cout<<"all jitter = "<<m_jitter<<std::endl;
  std::cout<<"jitter = "<<m_jitter/(packetReceive*1000000000.0)<<std::endl;
  std::cout<<"bytesTotal = "<<bytesTotal*8<<std::endl;




  Simulator::Destroy();
  return 0;
}
