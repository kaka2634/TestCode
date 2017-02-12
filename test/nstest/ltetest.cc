#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>

using namespace ns3;

int main(int argc, char const *argv[]) {
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  //configure LTE MAC scheduler
  lteHelper->SetSchedulerType ("ns3::FdMtFfMacScheduler");    // FD-MT scheduler

  NodeContainer enbNodes;
  enbNodes.Create(1);
  NodeContainer ueNodes;
  ueNodes.Create(2);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);

  NetDeviceContainer enbDevs;
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueDevs;
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  lteHelper->Attach(ueDevs,enbDevs.Get(0));

  enum EpsBearer::Qci q= EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer(q);
  lteHelper->ActivateDataRadioBearer (ueDevs,bearer);

 //Simulation Output
  // lteHelper->EnablePhyTraces ();
  // lteHelper->EnableMacTraces ();
  // lteHelper->EnableRlcTraces ();
  // lteHelper->EnablePdcpTraces ();

  Simulator::Stop(Seconds(0.005));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
