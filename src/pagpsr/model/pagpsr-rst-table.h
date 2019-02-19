#ifndef PAGPSR_RSTTABLE_H
#define PAGPSR_RSTTABLE_H

#include <map>
#include <cassert>
#include <stdint.h>
#include "ns3/ipv4.h"
#include "ns3/timer.h"
#include <sys/types.h>
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/random-variable-stream.h"
#include <complex>

namespace ns3 {
namespace pagpsr {

/*
 * \ingroup pagpsr
 * \brief Position table used by pagpsr
 */
class RstTable
{
public:
  /// c-tor
  RstTable ();


  /**
   * \brief Adds entry in position table
   */
  void AddEntry (Ipv4Address id, std::pair<std::string, std::pair<uint32_t,Ipv4Address> > m_pair);

  /**
   * \brief Deletes entry in position table
   */
  void DeleteEntry (Ipv4Address id);

  std::pair<bool, std::string>  FindPacket (Ipv4Address id, std::pair<std::string, std::pair<uint32_t,Ipv4Address> > m_pair);

  /**
   * \brief Gets position from position table
   * \param id Ipv4Address to get position from
   * \return Position of that id or NULL if not known
   */

  /**
   * \brief clears all entries
   */
  void Clear ();


private:

  std::map<Ipv4Address, std::vector< std::pair<std::string, std::pair<uint32_t,Ipv4Address> > > >  m_table_rst;

};

}   // pagpsr
} // ns3
#endif /* PAGPSR_RSTTABLE_H */
