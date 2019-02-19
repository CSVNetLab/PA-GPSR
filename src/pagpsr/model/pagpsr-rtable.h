#ifndef PAGPSR_RTABLE_H
#define PAGPSR_RTABLE_H

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
class RTable
{
public:
  /// c-tor
  RTable ();


  /**
   * \brief Adds entry in position table
   */
  void AddEntry (Ipv4Address id, Ipv4Address dest);

  /**
   * \brief Deletes entry in position table
   */
  void DeleteEntry (Ipv4Address id);

  bool FindPacket (Ipv4Address id, Ipv4Address dest);

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

  std::map<Ipv4Address, std::vector< Ipv4Address > >  m_rtable;

};

}   // cgpsr
} // ns3
#endif /* PAGPSR_RTABLE_H */
