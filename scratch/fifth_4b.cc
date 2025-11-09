#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FifthScriptExample");

static std::ofstream cwndStream("tcp_cwnd_data.csv");
static std::ofstream lossStream("tcp_loss_data.csv");

// Callback pour la fenêtre de congestion
static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    *stream->GetStream() << Simulator::Now().GetMilliSeconds() << "," << newCwnd << std::endl;
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s\tCWND: " << newCwnd);
}

// Callback pour les pertes
static void
RxDrop(Ptr<const Packet> p)
{
    lossStream << Simulator::Now().GetMilliSeconds() << "," << p->GetSize() << std::endl;
    NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds());
}

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // Configuration TCP
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // Modèle d'erreur
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(0.0001));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Serveur
    uint16_t sinkPort = 8080;
    Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), sinkPort));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", sinkAddress);
    ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get(1));
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(20.0));

    // Client avec BulkSend (plus simple)
    BulkSendHelper source("ns3::TcpSocketFactory", sinkAddress);
    source.SetAttribute("MaxBytes", UintegerValue(0));
    ApplicationContainer sourceApps = source.Install(nodes.Get(0));
    sourceApps.Start(Seconds(1.0));
    sourceApps.Stop(Seconds(20.0));

    // Configuration des traces
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("tcp_cwnd_data.csv");
    
    // Récupérer le socket de la première application source
    Ptr<BulkSendApplication> bulkApp = DynamicCast<BulkSendApplication>(sourceApps.Get(0));
    Ptr<Socket> socket = bulkApp->GetSocket();
    
    if (socket) {
        socket->TraceConnectWithoutContext("CongestionWindow", 
                                         MakeBoundCallback(&CwndChange, stream));
    }

    // Trace des pertes
    devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));

    // En-têtes des fichiers
    *stream->GetStream() << "time_ms,cwnd" << std::endl;
    lossStream << "time_ms,packet_size" << std::endl;

    std::cout << "Démarrage simulation TCP..." << std::endl;

    Simulator::Stop(Seconds(20));
    Simulator::Run();

    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApps.Get(0));
    std::cout << "Octets reçus: " << sink->GetTotalRx() << std::endl;
    std::cout << "Débit: " << sink->GetTotalRx() * 8 / 20.0 / 1000000.0 << " Mbps" << std::endl;

    Simulator::Destroy();
    return 0;
}
