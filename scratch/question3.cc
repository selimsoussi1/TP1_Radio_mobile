// scratch/question3_netanim.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Question3NetAnim");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(3);

    // Configuration mobilité pour NetAnim
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));    // n0
    positionAlloc->Add(Vector(50.0, 0.0, 0.0));   // n1
    positionAlloc->Add(Vector(100.0, 0.0, 0.0));  // n2
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("10ms"));

    NetDeviceContainer devices01 = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer devices12 = pointToPoint.Install(nodes.Get(1), nodes.Get(2));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);

    // CONFIGURATION ROUTAGE
    Ptr<Ipv4StaticRouting> staticRouting0;
    staticRouting0 = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (
        nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol());
    staticRouting0->AddNetworkRouteTo(Ipv4Address("10.1.2.0"), Ipv4Mask("255.255.255.0"),
                                     Ipv4Address("10.1.1.2"), 1);

    Ptr<Ipv4StaticRouting> staticRouting1;
    staticRouting1 = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (
        nodes.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol());
    staticRouting1->AddNetworkRouteTo(Ipv4Address("10.1.1.0"), Ipv4Mask("255.255.255.0"),
                                     Ipv4Address("10.1.1.1"), 1);
    staticRouting1->AddNetworkRouteTo(Ipv4Address("10.1.2.0"), Ipv4Mask("255.255.255.0"),
                                     Ipv4Address("10.1.2.2"), 2);

    // Applications
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(interfaces12.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(3));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(2.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(8.0));

    // NetAnim
    AnimationInterface anim("question3_netanim.xml");
    anim.UpdateNodeDescription(0, "Client n0");
    anim.UpdateNodeDescription(1, "Routeur n1");
    anim.UpdateNodeDescription(2, "Serveur n2");
    anim.UpdateNodeColor(0, 0, 255, 0);   // Vert
    anim.UpdateNodeColor(1, 255, 255, 0); // Jaune
    anim.UpdateNodeColor(2, 255, 0, 0);   // Rouge
    anim.EnablePacketMetadata(true);

    pointToPoint.EnablePcapAll("question3_netanim");

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
