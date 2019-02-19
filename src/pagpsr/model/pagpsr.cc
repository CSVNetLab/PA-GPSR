/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * pagpsr
 *
 */
#define NS_LOG_APPEND_CONTEXT                                           \
  if (m_ipv4) { std::clog << "[node " << m_ipv4->GetObject<Node> ()->GetId () << "] "; }

#include "pagpsr.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include <algorithm>
#include <limits>


#define PAGPSR_LS_GOD 0

#define PAGPSR_LS_RLS 1

#define PAGPSR_MAXJITTER         (HelloInterval.GetSeconds() / 2)
//#define JITTER (Seconds (m_uniformRandomVariable->GetValue (-PAGPSR_MAXJITTER, PAGPSR_MAXJITTER)))

float InPackets = 0.0;
float OutPackets = 0.0;

NS_LOG_COMPONENT_DEFINE ("PAGpsrRoutingProtocol");

namespace ns3 {
namespace pagpsr {



struct DeferredRouteOutputTag : public Tag
{
  /// Positive if output device is fixed in RouteOutput
  uint32_t m_isCallFromL3;

  DeferredRouteOutputTag () : Tag (),
                              m_isCallFromL3 (0)
  {
  }

  static TypeId GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::pagpsr::DeferredRouteOutputTag").SetParent<Tag> ();
    return tid;
  }

  TypeId  GetInstanceTypeId () const
  {
    return GetTypeId ();
  }

  uint32_t GetSerializedSize () const
  {
    return sizeof(uint32_t);
  }

  void  Serialize (TagBuffer i) const
  {
    i.WriteU32 (m_isCallFromL3);
  }

  void  Deserialize (TagBuffer i)
  {
    m_isCallFromL3 = i.ReadU32 ();
  }

  void  Print (std::ostream &os) const
  {
    os << "DeferredRouteOutputTag: m_isCallFromL3 = " << m_isCallFromL3;
  }
};



NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

/// UDP Port for PAGPSR control traffic, not defined by IANA yet
const uint32_t RoutingProtocol::PAGPSR_PORT = 666;

RoutingProtocol::RoutingProtocol ()
  : HelloInterval (Seconds (1)),
    MaxQueueLen (64),
    MaxQueueTime (Seconds (30)),
    m_queue (MaxQueueLen, MaxQueueTime),
    gps_error(0),
    HelloIntervalTimer (Timer::CANCEL_ON_DESTROY)
{

  m_neighbors = PositionTable ();

//m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::pagpsr::RoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .AddConstructor<RoutingProtocol> ()
    .AddAttribute ("HelloInterval", "HELLO messages emission interval.",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&RoutingProtocol::HelloInterval),
                   MakeTimeChecker ())
    .AddAttribute ("LocationServiceName", "Indicates wich Location Service is enabled",
                   EnumValue (PAGPSR_LS_GOD),
                   MakeEnumAccessor (&RoutingProtocol::LocationServiceName),
                   MakeEnumChecker (PAGPSR_LS_GOD, "GOD",
                                    PAGPSR_LS_RLS, "RLS"))
    .AddAttribute ("GPSDrift", "GPS error",
                   UintegerValue (0),
                   MakeUintegerAccessor (&RoutingProtocol::gps_error),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

RoutingProtocol::~RoutingProtocol ()
{
}

void
RoutingProtocol::DoDispose ()
{
  m_ipv4 = 0;
  Ipv4RoutingProtocol::DoDispose ();
}

Ptr<LocationService>
RoutingProtocol::GetLS ()
{
  return m_locationService;
}
void
RoutingProtocol::SetLS (Ptr<LocationService> locationService)
{
  m_locationService = locationService;
}

bool RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                  UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                  LocalDeliverCallback lcb, ErrorCallback ecb)
{

NS_LOG_FUNCTION (this << p->GetUid () << header.GetDestination () << idev->GetAddress ());
  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No pagpsr interfaces");
      return false;
    }
  NS_ASSERT (m_ipv4 != 0);
  NS_ASSERT (p != 0);
  // Check if input device supports IP
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  int32_t iif = m_ipv4->GetInterfaceForDevice (idev);
  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();


  
  DeferredRouteOutputTag tag; //FIXME since I have to check if it's in origin for it to work it means I'm not taking some tag out...
  if (p->PeekPacketTag (tag) && IsMyOwnAddress (origin))
    {
      Ptr<Packet> packet = p->Copy (); //FIXME ja estou a abusar de tirar tags
      packet->RemovePacketTag(tag);
      DeferredRouteOutput (packet, header, ucb, ecb);
      return true; 
    }


  if (m_ipv4->IsDestinationAddress (dst, iif))
    {
      Ptr<Packet> packet = p->Copy ();
      TypeHeader tHeader (PAGPSRTYPE_POS);
      packet->RemoveHeader (tHeader);
      if (!tHeader.IsValid ())
        {
          NS_LOG_DEBUG ("PAGPSR message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Ignored");
          return false;
        }
      
      if (tHeader.Get () == PAGPSRTYPE_POS)
        {
          PositionHeader phdr;
          packet->RemoveHeader (phdr);

  //uint8_t ttl = header.GetTtl();
//if (m_ipv4->GetAddress (1, 0).GetLocal () == "10.0.0.10" && ttl != 1)
 // std::cout<<"Start: "<<unsigned(ttl)<<"\n";
        }

      if (dst != m_ipv4->GetAddress (1, 0).GetBroadcast ())
        {
          NS_LOG_LOGIC ("Unicast local delivery to " << dst);
        }
      else
        {
          NS_LOG_LOGIC ("Broadcast local delivery to " << dst);
        }

      lcb (packet, header, iif);
      return true;
    }

  return Forwarding (p, header, ucb, ecb);
}


void
RoutingProtocol::DeferredRouteOutput (Ptr<const Packet> p, const Ipv4Header & header,
                                      UnicastForwardCallback ucb, ErrorCallback ecb)
{

  NS_LOG_FUNCTION (this << p << header);
  NS_ASSERT (p != 0 && p != Ptr<Packet> ());

  if (m_queue.GetSize () == 0)
    {
      CheckQueueTimer.Cancel ();
      CheckQueueTimer.Schedule (Time ("500ms"));
    }
  QueueEntry newEntry (p, header, ucb, ecb);
  bool result = m_queue.Enqueue (newEntry);


  m_queuedAddresses.insert (m_queuedAddresses.begin (), header.GetDestination ());
  m_queuedAddresses.unique ();

  if (result)
    {
      NS_LOG_LOGIC ("Add packet " << p->GetUid () << " to queue. Protocol " << (uint16_t) header.GetProtocol ());

    }

}

void
RoutingProtocol::CheckQueue ()
{

  CheckQueueTimer.Cancel ();

  std::list<Ipv4Address> toRemove;

  for (std::list<Ipv4Address>::iterator i = m_queuedAddresses.begin (); i != m_queuedAddresses.end (); ++i)
    {
      if (SendPacketFromQueue (*i))
        {
          //Insert in a list to remove later
          toRemove.insert (toRemove.begin (), *i);
        }

    }

  //remove all that are on the list
  for (std::list<Ipv4Address>::iterator i = toRemove.begin (); i != toRemove.end (); ++i)
    {
      m_queuedAddresses.remove (*i);
    }

  if (!m_queuedAddresses.empty ()) //Only need to schedule if the queue is not empty
    {
      CheckQueueTimer.Schedule (Time ("500ms"));
    }
}

bool
RoutingProtocol::SendPacketFromQueue (Ipv4Address dst)
{

  NS_LOG_FUNCTION (this);
  bool recovery = false;
  RequestQueue m_queue_temp = m_queue;
  QueueEntry queueEntry_temp;
  QueueEntry queueEntry;
  int packet;
  Ipv4Header header = queueEntry.GetIpv4Header ();
  Ipv4Address origin = header.GetSource();
  origin_packet = std::make_pair("", std::make_pair(99999, origin));

  if (m_locationService->IsInSearch (dst))
    {
      return false;
    }

  if (!m_locationService->HasPosition (dst)) // Location-service stoped looking for the dst
    {
      m_queue.DropPacketWithDst (dst);
      NS_LOG_LOGIC ("Location Service did not find dst. Drop packet to " << dst);
      return true;
    }


  uint32_t drift = var->GetValue (-gps_error, gps_error);

  Vector myPos;
  
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();
  myPos.x = MM->GetPosition ().x + drift;
  myPos.y = MM->GetPosition ().y + drift;
  Ipv4Address nextHop;

  if(m_neighbors.isNeighbour (dst))
    {
      nextHop = dst;
    }
  else{
    Vector dstPos = m_locationService->GetPosition (dst);
    nextHop = m_neighbors.BestNeighbor (dstPos, myPos, origin_packet, dst);
    if (nextHop == Ipv4Address::GetZero ())
      {
        NS_LOG_LOGIC ("Fallback to recovery-mode. Packets to " << dst);
        recovery = true;
      }
    if(recovery)
      {
        
        Vector Position;
        Vector previousHop;
        uint32_t updated;
        
        while(m_queue.Dequeue (dst, queueEntry))
          {
            Ptr<Packet> p = ConstCast<Packet> (queueEntry.GetPacket ());
            UnicastForwardCallback ucb = queueEntry.GetUnicastForwardCallback ();
            Ipv4Header header = queueEntry.GetIpv4Header ();
            
            TypeHeader tHeader (PAGPSRTYPE_POS);
            p->RemoveHeader (tHeader);
            if (!tHeader.IsValid ())
              {
                NS_LOG_DEBUG ("GPSR message " << p->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
                return false;     // drop
              }
            if (tHeader.Get () == PAGPSRTYPE_POS)
              {
                PositionHeader hdr;
                p->RemoveHeader (hdr);
                Position.x = hdr.GetDstPosx ();
                Position.y = hdr.GetDstPosy ();
                updated = hdr.GetUpdated (); 
              }
            
            PositionHeader posHeader (Position.x, Position.y,  updated, myPos.x, myPos.y, (uint8_t) 1, Position.x, Position.y, (uint8_t) 0); 
            p->AddHeader (posHeader); //enters in recovery with last edge from Dst
            p->AddHeader (tHeader);
            
            RecoveryMode(dst, p, ucb, header);
          }
        return true;
      }
  }
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  route->SetDestination (dst);
  route->SetGateway (nextHop);

  // FIXME: Does not work for multiple interfaces
  route->SetOutputDevice (m_ipv4->GetNetDevice (1));

  while (m_queue.Dequeue (dst, queueEntry))
    {
      DeferredRouteOutputTag tag;
      Ptr<Packet> p = ConstCast<Packet> (queueEntry.GetPacket ());

      UnicastForwardCallback ucb = queueEntry.GetUnicastForwardCallback ();
      Ipv4Header header = queueEntry.GetIpv4Header ();

      if (header.GetSource () == Ipv4Address ("102.102.102.102"))
        {
          route->SetSource (m_ipv4->GetAddress (1, 0).GetLocal ());
          header.SetSource (m_ipv4->GetAddress (1, 0).GetLocal ());
        }
      else
        {
          route->SetSource (header.GetSource ());
        }
      ucb (route, p, header);
    }
  return true;
}



void 
RoutingProtocol::RecoveryMode(Ipv4Address dst, Ptr<Packet> p, UnicastForwardCallback ucb, Ipv4Header header){

  Vector Position;
  Vector previousHop;
  uint32_t updated;
  uint64_t positionX;
  uint64_t positionY;
  Vector myPos;
  Vector recPos;
  int packet = header.GetIdentification();
  Ipv4Address origin = header.GetSource ();
  uint8_t forward_method = 0;
  uint32_t n_it;
  std::string forwards[2] = {"R","L"};
  std::vector<uint8_t> f_n (2);
  bool NodeRec;
  Ipv4Address previousHopIp;

  origin_packet = std::make_pair("", std::make_pair(packet, origin));

  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();

  uint32_t drift = var->GetValue (-gps_error, gps_error);

  myPos.x = MM->GetPosition ().x + drift;
  myPos.y = MM->GetPosition ().y + drift;   

  TypeHeader tHeader (PAGPSRTYPE_POS);
  p->RemoveHeader (tHeader);
  if (!tHeader.IsValid ())
    {
      NS_LOG_DEBUG ("PAGPSR message " << p->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
      return;     // drop
    }
  if (tHeader.Get () == PAGPSRTYPE_POS)
    {
      PositionHeader hdr;
      p->RemoveHeader (hdr);
      Position.x = hdr.GetDstPosx ();
      Position.y = hdr.GetDstPosy ();
      updated = hdr.GetUpdated (); 
      recPos.x = hdr.GetRecPosx ();
      recPos.y = hdr.GetRecPosy ();
      previousHop.x = hdr.GetLastPosx ();
      previousHop.y = hdr.GetLastPosy ();
      forward_method = hdr.GetForward ();
   }


  switch (forward_method){
         case 0: // packet came from greedy-forward
             n_it = 2;
             f_n[0] = 1;
             f_n[1] = 2;
             previousHop.x = Position.x;
             previousHop.y = Position.y;
             NodeRec = true;
             break;
         case 1: 
             n_it = 1;
             f_n[0] = 1;
             forwards[0] = "R";
             NodeRec = false;
             break;
         case 2: 
             n_it = 1;
             forwards[0] = "L";
             f_n[0] = 2;
             NodeRec = false;
             break;
  }
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  route->SetOutputDevice (m_ipv4->GetNetDevice (1));
  route->SetDestination (dst);
  for (int i =0 ; i<n_it;i++){

          PositionHeader posHeader (Position.x, Position.y,  updated, recPos.x, recPos.y, (uint8_t) 1, myPos.x, myPos.y, (uint8_t) f_n[i]); 
          p->AddHeader (posHeader);
          p->AddHeader (tHeader);
          origin_packet.first = forwards[i];
          if (n_it == 2)
                previousHopIp = header.GetDestination ();
          else
                previousHopIp = m_neighbors.GetPreviousIp (previousHop, myPos);
          Ipv4Address nextHop = m_neighbors.BestRecoveryNeighbor (previousHopIp, Position, myPos, previousHop, recPos, origin_packet, NodeRec);
       
          if (nextHop == Ipv4Address::GetZero ())
            {
              continue;
            }


          route->SetGateway (nextHop);

          route->SetSource (header.GetSource ());

          ucb (route, p, header);
  }
  return;
}


void
RoutingProtocol::NotifyInterfaceUp (uint32_t interface)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (interface, 0).GetLocal ());
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (l3->GetNAddresses (interface) > 1)
    {
      NS_LOG_WARN ("PAGPSR does not work with more then one address per each interface.");
    }
  Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
  if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
    {
      return;
    }

  // Create a socket to listen only on this interface
  Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                             UdpSocketFactory::GetTypeId ());
  NS_ASSERT (socket != 0);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvPAGPSR, this));
  socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), PAGPSR_PORT));
  socket->BindToNetDevice (l3->GetNetDevice (interface));
  socket->SetAllowBroadcast (true);
  socket->SetAttribute ("IpTtl", UintegerValue (1));
  m_socketAddresses.insert (std::make_pair (socket, iface));


  // Allow neighbor manager use this interface for layer 2 feedback if possible
  Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
  if (wifi == 0)
    {
      return;
    }
  Ptr<WifiMac> mac = wifi->GetMac ();
  if (mac == 0)
    {
      return;
    }

  mac->TraceConnectWithoutContext ("TxErrHeader", m_neighbors.GetTxErrorCallback ());

}


void
RoutingProtocol::RecvPAGPSR (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Address sourceAddress;

  Ptr<Packet> packet = socket->RecvFrom (sourceAddress);

  TypeHeader tHeader (PAGPSRTYPE_HELLO);
  packet->RemoveHeader (tHeader);
  if (!tHeader.IsValid ())
    {
      NS_LOG_DEBUG ("PAGPSR message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Ignored");
      return;
    }

  HelloHeader hdr;
  packet->RemoveHeader (hdr);
  Vector Position;
  Position.x = hdr.GetOriginPosx ();
  Position.y = hdr.GetOriginPosy ();
  InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
  Ipv4Address sender = inetSourceAddr.GetIpv4 ();
  Ipv4Address receiver = m_socketAddresses[socket].GetLocal ();

  UpdateRouteToNeighbor (sender, receiver, Position, 0);
  m_neighbors.ResetTrustStatus(sender);

}


void
RoutingProtocol::UpdateRouteToNeighbor (Ipv4Address sender, Ipv4Address receiver, Vector Pos, bool trustStatus)
{

  m_neighbors.AddEntry (sender, Pos, trustStatus);

}



void
RoutingProtocol::NotifyInterfaceDown (uint32_t interface)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (interface, 0).GetLocal ());

  // Disable layer 2 link state monitoring (if possible)
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  Ptr<NetDevice> dev = l3->GetNetDevice (interface);
  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
  if (wifi != 0)
    {
      Ptr<WifiMac> mac = wifi->GetMac ()->GetObject<AdhocWifiMac> ();
      if (mac != 0)
        {
          mac->TraceDisconnectWithoutContext ("TxErrHeader",
                                              m_neighbors.GetTxErrorCallback ());
        }
    }

  // Close socket
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (m_ipv4->GetAddress (interface, 0));
  NS_ASSERT (socket);
  socket->Close ();
  m_socketAddresses.erase (socket);
  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No pagpsr interfaces");
      m_neighbors.Clear ();
      m_locationService->Clear ();
      return;
    }
}


Ptr<Socket>
RoutingProtocol::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const
{
  NS_LOG_FUNCTION (this << addr);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      if (iface == addr)
        {
          return socket;
        }
    }
  Ptr<Socket> socket;
  return socket;
}



void RoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{

  NS_LOG_FUNCTION (this << " interface " << interface << " address " << address);
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (!l3->IsUp (interface))
    {
      return;
    }
  if (l3->GetNAddresses ((interface) == 1))
    {
      Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
      Ptr<Socket> socket = FindSocketWithInterfaceAddress (iface);
      if (!socket)
        {
          if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
            {
              return;
            }
          // Create a socket to listen only on this interface
          Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                     UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvPAGPSR,this));
          // Bind to any IP address so that broadcasts can be received
          socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), PAGPSR_PORT));
          socket->BindToNetDevice (l3->GetNetDevice (interface));
          socket->SetAllowBroadcast (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));

          Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
        }
    }
  else
    {
      NS_LOG_LOGIC ("PAGPSR does not work with more then one address per each interface. Ignore added address");
    }
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
  if (socket)
    {

      m_socketAddresses.erase (socket);
      Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
      if (l3->GetNAddresses (i))
        {
          Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
          // Create a socket to listen only on this interface
          Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                     UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvPAGPSR, this));
          // Bind to any IP address so that broadcasts can be received
          socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), PAGPSR_PORT));
          socket->SetAllowBroadcast (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));

          // Add local broadcast record to the routing table
          Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));

        }
      if (m_socketAddresses.empty ())
        {
          NS_LOG_LOGIC ("No pagpsr interfaces");
          m_neighbors.Clear ();
          m_locationService->Clear ();
          return;
        }
    }
  else
    {
      NS_LOG_LOGIC ("Remove address not participating in PAGPSR operation");
    }
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  float n = float (rand()) / (float (RAND_MAX/PAGPSR_MAXJITTER) );
 
  JITTER  = Seconds(n);
  NS_ASSERT (ipv4 != 0);
  NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4;

  HelloIntervalTimer.SetFunction (&RoutingProtocol::HelloTimerExpire, this);
  HelloIntervalTimer.Schedule (JITTER);


  //Schedule only when it has packets on queue
  CheckQueueTimer.SetFunction (&RoutingProtocol::CheckQueue, this);

  Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

void
RoutingProtocol::HelloTimerExpire ()
{
  float n = float (rand()) / (float (RAND_MAX/PAGPSR_MAXJITTER) );
  float n2 = -(float (rand()) / (float (RAND_MAX/PAGPSR_MAXJITTER) ));
 
  JITTER  = Seconds(n+n2);

  SendHello ();
  HelloIntervalTimer.Cancel ();
  HelloIntervalTimer.Schedule (HelloInterval + JITTER);
}

void
RoutingProtocol::SendHello ()
{
  NS_LOG_FUNCTION (this);
  double positionX;
  double positionY;

  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();

  uint32_t drift = var->GetValue (-gps_error, gps_error);

  positionX = MM->GetPosition ().x + drift;
  positionY = MM->GetPosition ().y + drift; 

  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      HelloHeader helloHeader (((uint64_t) positionX),((uint64_t) positionY));

      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader (helloHeader);
      TypeHeader tHeader (PAGPSRTYPE_HELLO);
      packet->AddHeader (tHeader);
      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
      Ipv4Address destination;
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
        }
      else
        {
          destination = iface.GetBroadcast ();
        }
      socket->SendTo (packet, 0, InetSocketAddress (destination, PAGPSR_PORT));

    }
}

bool
RoutingProtocol::IsMyOwnAddress (Ipv4Address src)
{
  NS_LOG_FUNCTION (this << src);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (src == iface.GetLocal ())
        {
          return true;
        }
    }
  return false;
}




void
RoutingProtocol::Start ()
{
  NS_LOG_FUNCTION (this);
  m_queuedAddresses.clear ();

  //FIXME ajustar timer, meter valor parametrizavel
  Time tableTime ("2s");

  switch (LocationServiceName)
    {
    case PAGPSR_LS_GOD:
      NS_LOG_DEBUG ("GodLS in use");
      m_locationService = CreateObject<GodLocationService> ();
      break;
    case PAGPSR_LS_RLS:
      NS_LOG_UNCOND ("RLS not yet implemented");
      break;
    }

}

Ptr<Ipv4Route>
RoutingProtocol::LoopbackRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif)
{
  NS_LOG_FUNCTION (this << hdr);
  m_lo = m_ipv4->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);
  Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
  rt->SetDestination (hdr.GetDestination ());

  std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
  if (oif)
    {
      // Iterate to find an address on the oif device
      for (j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
        {
          Ipv4Address addr = j->second.GetLocal ();
          int32_t interface = m_ipv4->GetInterfaceForAddress (addr);
          if (oif == m_ipv4->GetNetDevice (static_cast<uint32_t> (interface)))
            {
              rt->SetSource (addr);
              break;
            }
        }
    }
  else
    {
      rt->SetSource (j->second.GetLocal ());
    }
  NS_ASSERT_MSG (rt->GetSource () != Ipv4Address (), "Valid PAGPSR source address not found");
  rt->SetGateway (Ipv4Address ("127.0.0.1"));
  rt->SetOutputDevice (m_lo);
  return rt;
}


int
RoutingProtocol::GetProtocolNumber (void) const
{
  return PAGPSR_PORT;
}

void
RoutingProtocol::AddHeaders (Ptr<Packet> p, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
{

  NS_LOG_FUNCTION (this << " source " << source << " destination " << destination);
 
  Vector myPos;
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();

  uint32_t drift = var->GetValue (-gps_error, gps_error);

  myPos.x = MM->GetPosition ().x + drift;
  myPos.y = MM->GetPosition ().y + drift;   

  uint16_t positionX = 0;
  uint16_t positionY = 0;
  uint32_t hdrTime = 0;

  if(destination != m_ipv4->GetAddress (1, 0).GetBroadcast ())
    {
      positionX = m_locationService->GetPosition (destination).x;
      positionY = m_locationService->GetPosition (destination).y;
      hdrTime = (uint32_t) m_locationService->GetEntryUpdateTime (destination).GetSeconds ();
    }

  PositionHeader posHeader (positionX, positionY,  hdrTime, (uint64_t) 0,(uint64_t) 0, (uint8_t) 0, myPos.x, myPos.y, (uint8_t) 0); 
  p->AddHeader (posHeader);
  TypeHeader tHeader (PAGPSRTYPE_POS);
  p->AddHeader (tHeader);

  m_downTarget (p, source, destination, protocol, route);

}

bool
RoutingProtocol::Forwarding (Ptr<const Packet> packet, const Ipv4Header & header,
                             UnicastForwardCallback ucb, ErrorCallback ecb)
{
  Ptr<Packet> p = packet->Copy ();
  NS_LOG_FUNCTION (this);
  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();
  uint32_t pk = header.GetIdentification();

  uint8_t forward_method = 0;


  origin_packet = std::make_pair("", std::make_pair(pk, origin));

  m_neighbors.Purge ();
  
  uint32_t updated = 0;
  Vector Position;
  Vector RecPosition;
  uint8_t inRec = 0;

  Vector previousHop;

  TypeHeader tHeader (PAGPSRTYPE_POS);
  PositionHeader hdr;
  p->RemoveHeader (tHeader);

  if (!tHeader.IsValid ())
    {
      NS_LOG_DEBUG ("PAGPSR message " << p->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
      return false;     // drop
    }
  if (tHeader.Get () == PAGPSRTYPE_POS)
    {
      p->RemoveHeader (hdr);
      Position.x = hdr.GetDstPosx ();
      Position.y = hdr.GetDstPosy ();
      updated = hdr.GetUpdated ();
      RecPosition.x = hdr.GetRecPosx ();
      RecPosition.y = hdr.GetRecPosy ();
      inRec = hdr.GetInRec ();
      previousHop.x = hdr.GetLastPosx ();
      previousHop.y = hdr.GetLastPosy ();
      forward_method = hdr.GetForward ();
    }
  Vector myPos;
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();

  uint32_t drift = var->GetValue (-gps_error, gps_error);

  myPos.x = MM->GetPosition ().x + drift;
  myPos.y = MM->GetPosition ().y + drift;

  if(inRec == 1 && CalculateDistance (myPos, Position) < CalculateDistance (RecPosition, Position)){
    inRec = 0;
    hdr.SetInRec(0);
    NS_LOG_LOGIC ("No longer in Recovery to " << dst << " in " << myPos);
  }

  if (inRec == 1){
        Ipv4Address previousHopIp = m_neighbors.GetPreviousIp (previousHop, myPos);
        m_neighbors.ChangeTrustStatus (previousHopIp, dst);
  }

  if(inRec){
    p->AddHeader (hdr);
    p->AddHeader (tHeader); //put headers back so that the RecoveryMode is compatible with Forwarding and SendFromQueue
    RecoveryMode (dst, p, ucb, header);
    return true;
  }



  uint32_t myUpdated = (uint32_t) m_locationService->GetEntryUpdateTime (dst).GetSeconds ();
  if (myUpdated > updated) //check if node has an update to the position of destination
    {
      Position.x = m_locationService->GetPosition (dst).x;
      Position.y = m_locationService->GetPosition (dst).y;
      updated = myUpdated;
    }

  Ipv4Address nextHop;

  if(m_neighbors.isNeighbour (dst))
    {
      
      nextHop = dst;
      PositionHeader posHeader (Position.x, Position.y,  updated, (uint64_t) 0, (uint64_t) 0, (uint8_t) 0, myPos.x, myPos.y, (uint8_t) 0);
      p->AddHeader (posHeader);
      p->AddHeader (tHeader);
      Ptr<Ipv4Route> route = Create<Ipv4Route> ();
      route->SetDestination (dst);
      route->SetGateway (nextHop);

     // FIXME: Does not work for multiple interfaces
     route->SetOutputDevice (m_ipv4->GetNetDevice (1));
     route->SetDestination (header.GetDestination ());
     ucb (route, p, header);
     return true;

    }
  else
    {
      if (forward_method == 0)
             origin_packet.first = "G";
      nextHop = m_neighbors.BestNeighbor (Position, myPos, origin_packet, dst);
      
      if (nextHop != Ipv4Address::GetZero ())
        {
          PositionHeader posHeader (Position.x, Position.y,  updated, (uint64_t) 0, (uint64_t) 0, (uint8_t) 0, myPos.x, myPos.y, (uint8_t) 0);
          p->AddHeader (posHeader);
          p->AddHeader (tHeader);
          Ptr<NetDevice> oif = m_ipv4->GetObject<NetDevice> ();
          Ptr<Ipv4Route> route = Create<Ipv4Route> ();
          route->SetDestination (dst);
          route->SetSource (header.GetSource ());
          route->SetGateway (nextHop);

          // FIXME: Does not work for multiple interfaces
          route->SetOutputDevice (m_ipv4->GetNetDevice (1));
          route->SetDestination (header.GetDestination ());
          NS_ASSERT (route != 0);
          NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetOutputDevice ());
          
          
          NS_LOG_LOGIC (route->GetOutputDevice () << " forwarding to " << dst << " from " << origin << " through " << route->GetGateway () << " packet " << p->GetUid ());
          
          ucb (route, p, header);
          return true;
        }
    }
  hdr.SetInRec(1);
  hdr.SetRecPosx (myPos.x);
  hdr.SetRecPosy (myPos.y); 
  hdr.SetLastPosx (previousHop.x); //when entering Recovery, the first edge is the Dst (MUDEI BOTEI PREVIOUS HOP)
  hdr.SetLastPosy (previousHop.y);
  p->AddHeader (hdr);
  p->AddHeader (tHeader);
  RecoveryMode (dst, p, ucb, header);

  NS_LOG_LOGIC ("Entering recovery-mode to " << dst << " in " << m_ipv4->GetAddress (1, 0).GetLocal ());
  return true;
}



void
RoutingProtocol::SetDownTarget (IpL4Protocol::DownTargetCallback callback)
{
  m_downTarget = callback;
}


IpL4Protocol::DownTargetCallback
RoutingProtocol::GetDownTarget (void) const
{
  return m_downTarget;
}






Ptr<Ipv4Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                              Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{

  Ipv4Address origin = header.GetSource ();
  int packet = header.GetIdentification();

  origin_packet = std::make_pair("", std::make_pair(packet, origin));

  NS_LOG_FUNCTION (this << header << (oif ? oif->GetIfIndex () : 0));

  if (!p)
    {
      return LoopbackRoute (header, oif);     // later
    }
  if (m_socketAddresses.empty ())
    {
      sockerr = Socket::ERROR_NOROUTETOHOST;
      NS_LOG_LOGIC ("No pagpsr interfaces");
      Ptr<Ipv4Route> route;
      return route;
    }
  sockerr = Socket::ERROR_NOTERROR;
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  Ipv4Address dst = header.GetDestination ();

  Vector dstPos = Vector (1, 0, 0);

  if (!(dst == m_ipv4->GetAddress (1, 0).GetBroadcast ()))
    {
      dstPos = m_locationService->GetPosition (dst);
    }

  if (CalculateDistance (dstPos, m_locationService->GetInvalidPosition ()) == 0 && m_locationService->IsInSearch (dst))
    {
      DeferredRouteOutputTag tag;
      if (!p->PeekPacketTag (tag))
        {
          p->AddPacketTag (tag);
        }
      return LoopbackRoute (header, oif);
    }

  Vector myPos;
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();
  uint32_t drift = var->GetValue (-gps_error, gps_error);

  myPos.x = MM->GetPosition ().x + drift;
  myPos.y = MM->GetPosition ().y + drift;  

  Ipv4Address nextHop;

  if(m_neighbors.isNeighbour (dst))
    {
      nextHop = dst;

    }
  else
    {
      nextHop = m_neighbors.BestNeighbor (dstPos, myPos,origin_packet, dst);
  if (nextHop != Ipv4Address::GetZero ())
    {
      NS_LOG_DEBUG ("Destination: " << dst);

      route->SetDestination (dst);
      if (header.GetSource () == Ipv4Address ("102.102.102.102"))
        {
          route->SetSource (m_ipv4->GetAddress (1, 0).GetLocal ());
        }
      else
        {
          route->SetSource (header.GetSource ());
        }
       
      route->SetGateway (nextHop);
      route->SetOutputDevice (m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (route->GetSource ())));
      route->SetDestination (header.GetDestination ());
      NS_ASSERT (route != 0);
      NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetSource ());
      if (oif != 0 && route->GetOutputDevice () != oif)
        {
          NS_LOG_DEBUG ("Output device doesn't match. Dropped.");
          sockerr = Socket::ERROR_NOROUTETOHOST;
          return Ptr<Ipv4Route> ();
        }
      return route;
    }
  else
    {
      DeferredRouteOutputTag tag;
      if (!p->PeekPacketTag (tag))
        {
          p->AddPacketTag (tag); 
        }
      return LoopbackRoute (header, oif);     //in RouteInput the recovery-mode is called
    }

}
}
}
