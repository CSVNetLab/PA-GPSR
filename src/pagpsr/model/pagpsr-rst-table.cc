#include "pagpsr-rst-table.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("PAGpsrRstTable");


namespace ns3 {
namespace pagpsr {

RstTable::RstTable ()
{

}


/**
 * \brief Adds entry in position table
 */
void 
RstTable::AddEntry (Ipv4Address id, std::pair<std::string, std::pair<uint32_t,Ipv4Address> > m_pair)
{

  std::vector< std::pair<std::string, std::pair<uint32_t,Ipv4Address> > > pvector;

  std::map<Ipv4Address, std::vector< std::pair<std::string, std::pair<uint32_t,Ipv4Address> > > >::iterator i = m_table_rst.find (id);

  if (i != m_table_rst.end () || id.IsEqual (i->first))
    {

      pvector = i->second;

      std::vector<std::pair<uint32_t, Ipv4Address>> temp_pvector(pvector.size());

      for (int i=0 ; i < pvector.size() ; i++)
        temp_pvector[i] = pvector[i].second;

      auto it = find(temp_pvector.begin(), temp_pvector.end(), m_pair.second);
      int distance = std::distance(temp_pvector.begin(),it);

      if (it == temp_pvector.end()){
        pvector.push_back(m_pair);
       
      }

      m_table_rst.erase (id);
      pvector[distance].first = m_pair.first;
      m_table_rst.insert (std::make_pair (id, pvector));

      return;
    }

  pvector.push_back(m_pair);

  if (m_table_rst.size() == 0)
	 m_table_rst.clear();
  m_table_rst.insert (std::make_pair (id, pvector));
    


}

void RstTable::DeleteEntry (Ipv4Address id)
{
  m_table_rst.erase (id);
}

void 
RstTable::Clear ()
{

  m_table_rst.clear ();
}


std::pair<bool, std::string> 
RstTable::FindPacket (Ipv4Address id, std::pair<std::string, std::pair<uint32_t,Ipv4Address> > m_pair)
{

  std::vector< std::pair<std::string, std::pair<uint32_t,Ipv4Address> > > pvector;

  std::map<Ipv4Address, std::vector< std::pair<std::string, std::pair<uint32_t,Ipv4Address> > > >::iterator i = m_table_rst.find (id);

  if (i != m_table_rst.end () || id.IsEqual (i->first))
    {

      pvector = i->second;

      std::vector<std::pair<uint32_t, Ipv4Address>> temp_pvector(pvector.size());

      for (int i=0 ; i < pvector.size() ; i++)
        temp_pvector[i] = pvector[i].second;

      auto it = find(temp_pvector.begin(), temp_pvector.end(), m_pair.second);

      int distance = std::distance(temp_pvector.begin(),it);

      if (it != temp_pvector.end()){

         return std::make_pair(true, pvector[distance].first);     
      }
    }
    return std::make_pair(false, "");     

}

}   // pagpsr
} // ns3
