/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Andrey Silva and Niaz Reza. Adapted from the GPSR code of António Fonseca.
 */
#include "mmgpsr-helper.h"
#include "ns3/mmgpsr.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/node-container.h"
#include "ns3/callback.h"
#include "ns3/udp-l4-protocol.h"
 

namespace ns3 {

MMGpsrHelper::MMGpsrHelper ()
  : Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::mmgpsr::RoutingProtocol");
}

MMGpsrHelper*
MMGpsrHelper::Copy (void) const
{
  return new MMGpsrHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
MMGpsrHelper::Create (Ptr<Node> node) const
{
  //Ptr<Ipv4L4Protocol> ipv4l4 = node->GetObject<Ipv4L4Protocol> ();
  Ptr<mmgpsr::RoutingProtocol> mmgpsr = m_agentFactory.Create<mmgpsr::RoutingProtocol> ();
  node->AggregateObject (mmgpsr);
  return mmgpsr;
}

void
MMGpsrHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}


void 
MMGpsrHelper::Install (void) const
{
  NodeContainer c = NodeContainer::GetGlobal ();
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = (*i);
      Ptr<UdpL4Protocol> udp = node->GetObject<UdpL4Protocol> ();
      Ptr<mmgpsr::RoutingProtocol> mmgpsr = node->GetObject<mmgpsr::RoutingProtocol> ();
      mmgpsr->SetDownTarget (udp->GetDownTarget ());
      udp->SetDownTarget (MakeCallback(&mmgpsr::RoutingProtocol::AddHeaders, mmgpsr));
    }


}


}
