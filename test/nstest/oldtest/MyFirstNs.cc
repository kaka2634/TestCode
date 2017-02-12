#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
//#include <iostream>
using namespace ns3;

//Add Log for echo, while LogComponent can use LogComponentEnable method to output
NS_LOG_COMPONENT_DEFINE ("MyFirstNsLog");

int main (int argc,char *argv[])
{
  //hooking your own Values
  uint32_t nPackets =1;

  //add CommandLine to the terminal
  CommandLine cmd;
  cmd.AddValue("nPackets","Number of packet to echo", nPackets);
  cmd.Parse (argc,argv);

  Time::SetResolution(Time::NS);

  //这里定义的log会输出在屏幕上
  LogComponentEnable("UdpEchoClientApplication",LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication",LOG_LEVEL_INFO);



  NodeContainer nodes;
  nodes.Create(2);

  //这里输出在MyFirstLog中，因为这个不是LogComponent，需要赋予LOG_的level才看能不能显示
  NS_LOG_INFO ("Creating Topology");

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate",StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay",StringValue("2ms"));
  //PCAP tracing


  //ASCII tracing
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("myfirst.tr"));


  NetDeviceContainer devices;
  devices = pointToPoint.Install(nodes);
  
  //shoud after devices so it can enable all devices
  pointToPoint.EnablePcapAll(std::string ("myFirst"));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0","255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get(1));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop (Seconds(10.0));

  UdpEchoClientHelper echoclient (interfaces.GetAddress (1),9);

  echoclient.SetAttribute ("MaxPackets", UintegerValue(nPackets));
  echoclient.SetAttribute ("Interval",TimeValue(Seconds (1.0)));
  echoclient.SetAttribute ("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps =echoclient.Install (nodes.Get(0));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));

  Simulator::Run();
  Simulator::Destroy();
  //also can use c++ standard
  //std::cout<<"hello"<<std::endl;
  return 0;

}
