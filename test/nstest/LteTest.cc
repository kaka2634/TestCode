#include "ns3/core-module.h"
#include "ns3/network-module.h" //NodeContainer
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"

using namespace ns3;

int main (int argc, char * argv)
{
  NodeContainer ueNodes;
  ueNodes.Creat(2);
  NodeContainer enbNodes;
  enbNodes.Creat(1);

  //lte is pointer
  Ptr<LteHelper> lteHelper = CreatObject<LteHelper> ();


  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel")
  mobility.Install(enbNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);

  //LTE same as CSMA
  NetDeviceContainer enbDevs;
  //lteHelper is point so use-> instead of .
  enbDevs = lteHelper->InstallEnbDevice(enbNobes);
  NetDeviceContainer ueDevs;
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  lteHelper->Attach(ueDevs,enbDevs.Get(0));

  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

  lteHelper->EnableLogComponents();
  Simulator::Stop(Seconds(0.005));
  Simulator::Run();
  Simulator::Destroy();
  return 0;


}
