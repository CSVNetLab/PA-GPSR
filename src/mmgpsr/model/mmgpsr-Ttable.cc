#include "mmgpsr-Ttable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("MMGpsrTTable");


namespace ns3 {
namespace mmgpsr {


TTable::TTable ()
{
}


void 
TTable::AddEntry (Ipv4Address id)
{

  Time T_cum_old;
  Time T_cum;
  Time t_hello;
  Time t_hello_old;

  std::map<Ipv4Address, std::pair<Time , Time> >::iterator i = m_Ttable.find (id);
  if (i != m_Ttable.end () || id.IsEqual (i->first))
    {
      T_cum_old = i->second.first;
      t_hello_old = i->second.second;
      t_hello = Simulator::Now (); 
      m_Ttable.erase (id);
      
      T_cum = T_cum_old  + t_hello - t_hello_old;
      m_Ttable.insert (std::make_pair (id, std::make_pair (T_cum, t_hello)));
      return;
    }
  t_hello = Simulator::Now ();
  T_cum = t_hello;
  m_Ttable.insert (std::make_pair (id, std::make_pair (T_cum, t_hello)));

}

Time 
TTable::Get (Ipv4Address id)
{

  std::map<Ipv4Address, std::pair<Time , Time> >::iterator i = m_Ttable.find (id);

  return i->second.first;
}

void TTable::DeleteEntry (Ipv4Address id)
{
  m_Ttable.erase (id);
}


void 
TTable::Clear ()
{
  m_Ttable.clear ();
}


}   // mmgpsr
} // ns3
