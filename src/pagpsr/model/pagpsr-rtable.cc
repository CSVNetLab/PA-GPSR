#include "pagpsr-rtable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("PAGpsrRTable");


namespace ns3 {
namespace pagpsr {

/*
  PAGPSR 
*/

RTable::RTable ()
{

}


/**
 * \brief Adds entry in position table
 */
void 
RTable::AddEntry (Ipv4Address id, Ipv4Address dest)
{

  std::vector< Ipv4Address > pvector;

  std::map<Ipv4Address, std::vector< Ipv4Address > >::iterator i = m_rtable.find (id);

  if (i != m_rtable.end () || id.IsEqual (i->first))
    {

      pvector = i->second;

      auto it = find(pvector.begin(), pvector.end(), dest);

      if (it == pvector.end()){
        pvector.push_back(dest);

      }
      m_rtable.erase (id);
      m_rtable.insert (std::make_pair (id, pvector));

      return;
    }

  pvector.push_back(dest);
  if (m_rtable.size() == 0)
  	Clear();

  m_rtable.insert (std::make_pair (id, pvector));
    


}

void RTable::DeleteEntry (Ipv4Address id)
{
  m_rtable.erase (id);
}

void 
RTable::Clear ()
{

  m_rtable.clear ();
}


bool
RTable::FindPacket (Ipv4Address id, Ipv4Address dest)
{

  std::vector< Ipv4Address > pvector;

  std::map<Ipv4Address, std::vector< Ipv4Address > >::iterator i = m_rtable.find (id);

  if (i != m_rtable.end () || id.IsEqual (i->first))
    {

      pvector = i->second;

      auto it = find(pvector.begin(), pvector.end(), dest);

      if (it != pvector.end()){

         return true;     
      }
    }
    return false;     

}

}   // pagpsr
} // ns3
