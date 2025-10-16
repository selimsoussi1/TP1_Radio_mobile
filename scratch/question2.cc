// scratch/three_nodes_netanim.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThreeNodesNetAnim");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Création des 3 nœuds
    NodeContainer nodes;
    nodes.Create(3);

    // Configuration de la mobilité pour NetAnim
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));    // n0
    positionAlloc->Add(Vector(50.0, 0.0, 0.0));   // n1
    positionAlloc->Add(Vector(100.0, 0.0, 0.0));  // n2
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Configuration des liens
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("10ms"));

    // Installation des liens
    NetDeviceContainer devices01, devices12;
    devices01 = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    devices12 = pointToPoint.Install(nodes.Get(1), nodes.Get(2));

    // Pile Internet
    InternetStackHelper stack;
    stack.Install(nodes);

    // Adressage IP
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);

    // Communication n0 -> n1
    UdpEchoServerHelper echoServer01(9);
    ApplicationContainer serverApps01 = echoServer01.Install(nodes.Get(1));
    serverApps01.Start(Seconds(1.0));
    serverApps01.Stop(Seconds(20.0));

    UdpEchoClientHelper echoClient01(interfaces01.GetAddress(1), 9);
    echoClient01.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient01.SetAttribute("Interval", TimeValue(Seconds(2.0)));
    echoClient01.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps01 = echoClient01.Install(nodes.Get(0));
    clientApps01.Start(Seconds(2.0));
    clientApps01.Stop(Seconds(18.0));

    // Communication n1 -> n2
    UdpEchoServerHelper echoServer12(10);
    ApplicationContainer serverApps12 = echoServer12.Install(nodes.Get(2));
    serverApps12.Start(Seconds(1.0));
    serverApps12.Stop(Seconds(20.0));

    UdpEchoClientHelper echoClient12(interfaces12.GetAddress(1), 10);
    echoClient12.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient12.SetAttribute("Interval", TimeValue(Seconds(2.0)));
    echoClient12.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps12 = echoClient12.Install(nodes.Get(1));
    clientApps12.Start(Seconds(3.0));
    clientApps12.Stop(Seconds(19.0));

    // Configuration NetAnim
    AnimationInterface anim("question2.xml");
    
    // Personnalisation des nœuds
    anim.UpdateNodeDescription(0, "Client n0");
    anim.UpdateNodeDescription(1, "Routeur n1");
    anim.UpdateNodeDescription(2, "Serveur n2");
    
    anim.UpdateNodeColor(0, 0, 255, 0);   // Vert
    anim.UpdateNodeColor(1, 255, 255, 0); // Jaune
    anim.UpdateNodeColor(2, 255, 0, 0);   // Rouge
    
    // Activer les métadonnées des paquets
    anim.EnablePacketMetadata(true);

    // Traçage PCAP
    pointToPoint.EnablePcapAll("question2");

    std::cout << "=== SIMULATION AVEC NETANIM ===" << std::endl;
    std::cout << "Topologie: n0 --- n1 --- n2" << std::endl;
    std::cout << "Fichier d'animation: three_nodes_animation.xml" << std::endl;
    std::cout << "Durée: 20 secondes" << std::endl;
    std::cout << "Paquets: 5 dans chaque direction" << std::endl;

    Simulator::Stop(Seconds(20.0));
    Simulator::Run();
    Simulator::Destroy();
    
    std::cout << "Simulation terminée!" << std::endl;
    std::cout << "Ouvrez three_nodes_animation.xml avec NetAnim" << std::endl;
    return 0;
}
