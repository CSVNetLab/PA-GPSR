#include "pagpsr-ptable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("PAGpsrTable");


namespace ns3 {
namespace pagpsr {

PositionTable::PositionTable ()
{
  m_txErrorCallback = MakeCallback (&PositionTable::ProcessTxError, this);
  m_entryLifeTime = Seconds (2); //FIXME fazer isto parametrizavel de acordo com tempo de hello
  m_rst = RstTable();
  m_rtable = RTable();
}

Time 
PositionTable::GetEntryUpdateTime (Ipv4Address id)
{
  if (id == Ipv4Address::GetZero ())
    {
      return Time (Seconds (0));
    }
  std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> >::iterator i = m_table.find (id);
  return i->second.first.second;
}

void 
PositionTable::AddEntry (Ipv4Address id, Vector position, bool trustStatus)
{

  std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
    {

      m_table.erase (id);
      m_table.insert (std::make_pair (id, std::make_pair (std::make_pair (position, Simulator::Now ()),trustStatus)));

      return;
    }
  
  m_table.insert (std::make_pair (id, std::make_pair (std::make_pair (position, Simulator::Now ()),trustStatus)));
}

void 
PositionTable::ChangeTrustStatus (Ipv4Address id, Ipv4Address dest)
{
  m_rtable.AddEntry(id, dest);
  
}

void 
PositionTable::ResetTrustStatus (Ipv4Address id)
{
  m_rtable.DeleteEntry(id);
  
}

/**
 * \brief Deletes entry in position table
 */
void PositionTable::DeleteEntry (Ipv4Address id)
{
  m_table.erase (id);
}

/**
 * \brief Gets position from position table
 * \param id Ipv4Address to get position from
 * \return Position of that id or NULL if not known
 */
Vector 
PositionTable::GetPosition (Ipv4Address id)
{

  NodeList::Iterator listEnd = NodeList::End ();
  for (NodeList::Iterator i = NodeList::Begin (); i != listEnd; i++)
    {
      Ptr<Node> node = *i;
      if (node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () == id)
        {
          return node->GetObject<MobilityModel> ()->GetPosition ();
        }
    }
  return PositionTable::GetInvalidPosition ();

}

/**
 * \brief Checks if a node is a neighbour
 * \param id Ipv4Address of the node to check
 * \return True if the node is neighbour, false otherwise
 */
bool
PositionTable::isNeighbour (Ipv4Address id)
{

 std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
    {
      return true;
    }

  return false;
}


/**
 * \brief remove entries with expired lifetime
 */
void 
PositionTable::Purge ()
{

  if(m_table.empty ())
    {
      return;
    }

  std::list<Ipv4Address> toErase;

  std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> >::iterator i = m_table.begin ();
  std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> >::iterator listEnd = m_table.end ();
  
  for (; !(i == listEnd); i++)
    {

      if (m_entryLifeTime + GetEntryUpdateTime (i->first) <= Simulator::Now ())
        {
          toErase.insert (toErase.begin (), i->first);

        }
    }
  toErase.unique ();

  std::list<Ipv4Address>::iterator end = toErase.end ();

  for (std::list<Ipv4Address>::iterator it = toErase.begin (); it != end; ++it)
    {
      m_rst.DeleteEntry (*it);
      m_table.erase (*it);

    }
}

/**
 * \brief clears all entries
 */
void 
PositionTable::Clear ()
{
  m_table.clear ();
  m_rst.Clear();
}

/**
 * \brief Gets next hop according to pagpsr protocol
 * \param position the position of the destination node
 * \param nodePos the position of the node that has the packet
 * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
 */
Ipv4Address 
PositionTable::BestNeighbor (Vector position, Vector nodePos, std::pair<std::string, std::pair<uint32_t,Ipv4Address> > m_pair, Ipv4Address dest)
{

  Purge ();

  double initialDistance = CalculateDistance (nodePos, position);
  std::pair<bool,std::string> sent;
  bool WasInRec;

 /* Vector bestN;
  double dBD;
  double dSB;
  double lambda = 0.3;
  double dmax;
*/

  if (m_table.empty ())
    {
      NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << position);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  Ipv4Address bestFoundID = m_table.begin ()->first;
  double bestFoundDistance = CalculateDistance (m_table.begin ()->second.first.first, position);
  std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> >::iterator i;

  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      sent = m_rst.FindPacket(i->first, m_pair);
      WasInRec = m_rtable.FindPacket(i->first,dest);
      if (sent.first){
	continue;
      }
      if (bestFoundDistance > CalculateDistance (i->second.first.first, position))
        {
	  if(WasInRec == false){
          	bestFoundID = i->first;
	  	bestFoundDistance = CalculateDistance (i->second.first.first, position);
	  }else{
		continue;
	  }
	 
        }
    }

  if (m_pair.second.second == Ipv4Address ("102.102.102.102")){
	m_pair.second.first = 0;
	m_pair.second.second = Ipv4Address ("0.0.0.0");
  }

  if(initialDistance > bestFoundDistance){

    	m_rst.AddEntry(bestFoundID, m_pair);

    return bestFoundID;
  }
  else
    return Ipv4Address::GetZero (); //so it enters Recovery-mode

}

Ipv4Address
PositionTable::BestAngle (Vector previousHop, Vector nodePos)
{
  Purge ();

  if (m_table.empty ())
    {
      NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << nodePos);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  double tmpAngle;
  Ipv4Address bestFoundID = Ipv4Address::GetZero ();
  double bestFoundAngle = 360;
  std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> >::iterator i;

  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      tmpAngle = GetAngleOld(nodePos, previousHop, i->second.first.first);
      if (bestFoundAngle > tmpAngle && tmpAngle != 0)
	{
	  bestFoundID = i->first;
	  bestFoundAngle = tmpAngle;
	}
    }
  if(bestFoundID == Ipv4Address::GetZero ()) //only if the only neighbour is who sent the packet
    {
      bestFoundID = m_table.begin ()->first;
    }
  return bestFoundID;
}

Ipv4Address
PositionTable::GetPreviousIp (Vector previousHop, Vector nodePos)
{
  Purge ();

  if (m_table.empty ())
    {
      NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << nodePos);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  double tmpAngle;
  double bestFoundAngle = 360;
  Ipv4Address bestFoundID = m_table.begin ()->first;

  std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> >::iterator i;

  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      tmpAngle = GetAngle(nodePos, previousHop, i->second.first.first);
      if (bestFoundAngle > tmpAngle)
	{
	  bestFoundID = i->first;
          bestFoundAngle = tmpAngle;
	}
    }
  return bestFoundID;
}


//Gives angle between the vector CentrePos-Refpos to the vector CentrePos-node counterclockwise
double 
PositionTable::GetAngle (Vector centrePos, Vector refPos, Vector node){
  double const PI = 4*atan(1);

  std::complex<double> A = std::complex<double>(centrePos.x,centrePos.y);
  std::complex<double> B = std::complex<double>(node.x,node.y);
  std::complex<double> C = std::complex<double>(refPos.x,refPos.y);   //Change B with C if you want angles clockwise

  std::complex<double> AB; //reference edge
  std::complex<double> AC;
  std::complex<double> tmp;
  std::complex<double> tmpCplx;

  std::complex<double> Angle;

  AB = B - A;
  AB = (real(AB)/norm(AB)) + (std::complex<double>(0.0,1.0)*(imag(AB)/norm(AB)));

  AC = C - A;
  AC = (real(AC)/norm(AC)) + (std::complex<double>(0.0,1.0)*(imag(AC)/norm(AC)));

  tmp = log(AC/AB);
  tmpCplx = std::complex<double>(0.0,-1.0);
  Angle = tmp*tmpCplx;
  Angle *= (180/PI);
  if (real(Angle)<0){
    Angle = abs(real(Angle)) ;
  }

  return real(Angle);

}


Ipv4Address 
PositionTable::BestRecoveryNeighbor (Ipv4Address previousHopIp, Vector position, Vector nodePos, Vector PreviousPos, Vector recPos, std::pair<std::string, std::pair<uint32_t,Ipv4Address> > m_pair, bool NodeRec)
{
  Purge ();

  if (m_table.empty ())
    {
      NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << position);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  Ipv4Address bestFoundID = Ipv4Address::GetZero ();
  double bestFoundDistance = std::numeric_limits<double>::max();
  double bestFoundAngle_right = 360;
  double bestFoundAngle_left = 0;
  std::pair<bool, std::string> sent;
  double tmpAngle;
  bool right = false;
  bool left = false;


	if(m_pair.first == "L"){
	    left = true;
	}else{
	    right = true;
	}


  std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> >::iterator i;
  for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      tmpAngle = GetAngleOld(nodePos, PreviousPos, i->second.first.first);
      sent = m_rst.FindPacket(i->first, m_pair);

      if (sent.first){
	if(m_pair.first == sent.second){

		continue;
	}else{
		if (NodeRec == true){
			continue;
		}else{
			return Ipv4Address::GetZero ();
		}
	}
      }
      if (i->first == previousHopIp){

	continue;
      }

       if (right){
	       if (bestFoundAngle_right > tmpAngle)
		{
		  bestFoundID = i->first;
		  bestFoundAngle_right = tmpAngle;
		}
	}else{
		if (bestFoundAngle_left < tmpAngle)
		{
		  bestFoundID = i->first;
		  bestFoundAngle_left = tmpAngle;
		}
	}

    }
if (m_pair.second.second == Ipv4Address ("102.102.102.102")){
	m_pair.second.first = 0;
	m_pair.second.second = Ipv4Address ("0.0.0.0");
  }
	if(bestFoundID == Ipv4Address::GetZero ()) 
		return bestFoundID;
    	m_rst.AddEntry(bestFoundID, m_pair);
   
    return bestFoundID;

}

double 
PositionTable::GetAngleOld (Vector centrePos, Vector refPos, Vector node){
  double const PI = 4*atan(1);

  std::complex<double> A = std::complex<double>(centrePos.x,centrePos.y);
  std::complex<double> B = std::complex<double>(node.x,node.y);
  std::complex<double> C = std::complex<double>(refPos.x,refPos.y);   //Change B with C if you want angles clockwise

  std::complex<double> AB; //reference edge
  std::complex<double> AC;
  std::complex<double> tmp;
  std::complex<double> tmpCplx;

  std::complex<double> Angle;

  AB = B - A;
  AB = (real(AB)/norm(AB)) + (std::complex<double>(0.0,1.0)*(imag(AB)/norm(AB)));

  AC = C - A;
  AC = (real(AC)/norm(AC)) + (std::complex<double>(0.0,1.0)*(imag(AC)/norm(AC)));

  tmp = log(AC/AB);
  tmpCplx = std::complex<double>(0.0,-1.0);
  Angle = tmp*tmpCplx;
  Angle *= (180/PI);
  if (real(Angle)<0)
    Angle = 360+real(Angle);

  return real(Angle);

}

/**
 * \ProcessTxError
 */
void PositionTable::ProcessTxError (WifiMacHeader const & hdr)
{
}


bool PositionTable::IsInSearch (Ipv4Address id)
{
  return false;
}

bool PositionTable::HasPosition (Ipv4Address id)
{
  return true;
}


}   // pagpsr
} // ns3
