// scratch/first.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // UDP Echo Server on node 1
  UdpEchoServerHelper echoServer (9); // Port 9

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  // UDP Echo Client on node 0
  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  
  // Modified: Send 3 packets with custom size
  echoClient.SetAttribute ("MaxPackets", UintegerValue (3));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (512)); // Custom packet size

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  
  // Set custom fill data for the application after installation
  // Create a custom message pattern
  std::string customMessage = "HELLO_FROM_CLIENT_ACK_NS3_UDP_SIMULATION";
  
  // Set fill for each client application
  for (uint32_t i = 0; i < clientApps.GetN(); ++i) {
    Ptr<Application> app = clientApps.Get(i);
    
    // Method 1: Set fill with string pattern (repeated)
    echoClient.SetFill(app, customMessage);
    
    // Alternative method: Set fill with specific byte pattern
    // uint8_t fillByte = 'A' + (i % 26);
    // echoClient.SetFill(app, fillByte, 512);
  }

  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  // Enable PCAP tracing for Wireshark
  pointToPoint.EnablePcapAll ("my_custom_udp");

  // Enable NetAnim
  AnimationInterface anim ("my_custom_udp_animation.xml");
  
  // Set positions for better visualization
  anim.SetConstantPosition (nodes.Get(0), 10.0, 10.0);
  anim.SetConstantPosition (nodes.Get(1), 50.0, 10.0);
  
  // Enable packet metadata for NetAnim
  anim.EnablePacketMetadata (true);
  
  // Add node descriptions
  anim.UpdateNodeDescription (nodes.Get(0), "Client Node");
  anim.UpdateNodeDescription (nodes.Get(1), "Server Node");
  anim.UpdateNodeColor (nodes.Get(0), 0, 255, 0); // Green for client
  anim.UpdateNodeColor (nodes.Get(1), 255, 0, 0); // Red for server

  // Also enable ASCII tracing
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("my_custom_udp.tr"));

  std::cout << "Starting simulation with custom UDP messages..." << std::endl;
  std::cout << "Client will send custom message: " << customMessage << std::endl;
  std::cout << "Packet size: 512 bytes" << std::endl;
  std::cout << "Number of packets: 3" << std::endl;
  std::cout << "Interval between packets: 1 second" << std::endl;

  Simulator::Run ();
  Simulator::Destroy ();
  
  std::cout << "Simulation completed successfully!" << std::endl;
  std::cout << "Check generated files:" << std::endl;
  std::cout << "- PCAP files: my_custom_udp-0-0.pcap and my_custom_udp-1-0.pcap (for Wireshark)" << std::endl;
  std::cout << "- NetAnim file: my_custom_udp_animation.xml" << std::endl;
  std::cout << "- Trace file: my_custom_udp.tr" << std::endl;

  return 0;
}
