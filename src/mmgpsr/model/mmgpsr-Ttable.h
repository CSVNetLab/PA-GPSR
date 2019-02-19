#ifndef MMGPSR_TTABLE_H
#define MMGPSR_TTABLE_H

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
namespace mmgpsr {

class TTable
{
public:
  /// c-tor
  TTable ();


  /**
   * \brief Adds entry in position table
   */
  void AddEntry (Ipv4Address id);

  /**
   * \brief Deletes entry in position table
   */
  void DeleteEntry (Ipv4Address id);

  Time Get (Ipv4Address id);

  /**
   * \brief Gets position from position table
   * \param id Ipv4Address to get position from
   * \return Position of that id or NULL if not known
   */

  /**
   * \brief remove entries with expired lifetime
   */

  /**
   * \brief clears all entries
   */
  void Clear ();

private:

  std::map<Ipv4Address, std::pair<Time, Time> > m_Ttable; // T_cum e T_hello

};

}   // mmgpsr
} // ns3
#endif /* MMGPSR_TTABLE_H */
