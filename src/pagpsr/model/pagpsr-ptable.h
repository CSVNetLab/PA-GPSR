#ifndef PAGPSR_PTABLE_H
#define PAGPSR_PTABLE_H

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
#include "pagpsr-rst-table.h"
#include "pagpsr-rtable.h"

namespace ns3 {
namespace pagpsr {


class PositionTable
{
public:
  /// c-tor
  PositionTable ();

  /**
   * \brief Gets the last time the entry was updated
   * \param id Ipv4Address to get time of update from
   * \return Time of last update to the position
   */
  Time GetEntryUpdateTime (Ipv4Address id);

  /**
   * \brief Adds entry in position table
   */
void AddEntry (Ipv4Address id, Vector position, bool trustStatus);

  /**
   * \brief Deletes entry in position table
   */
  void DeleteEntry (Ipv4Address id);

  /**
   * \brief Gets position from position table
   * \param id Ipv4Address to get position from
   * \return Position of that id or NULL if not known
   */
  Vector GetPosition (Ipv4Address id);

  /**
   * \brief Checks if a node is a neighbour
   * \param id Ipv4Address of the node to check
   * \return True if the node is neighbour, false otherwise
   */
  bool isNeighbour (Ipv4Address id);

  /**
   * \brief remove entries with expired lifetime
   */
  void Purge ();

  /**
   * \brief clears all entries
   */
  void Clear ();

  /**
   * \Get Callback to ProcessTxError
   */
  Callback<void, WifiMacHeader const &> GetTxErrorCallback () const
  {
    return m_txErrorCallback;
  }

  /**
   * \brief Gets next hop according to pagpsr protocol
   * \param position the position of the destination node
   * \param nodePos the position of the node that has the packet
   * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
   */
  Ipv4Address BestNeighbor (Vector position, Vector nodePos, std::pair<std::string, std::pair<uint32_t,Ipv4Address> > m_pair, Ipv4Address dest);

  Ipv4Address BestRecoveryNeighbor (Ipv4Address previousHopIp, Vector position, Vector nodePos, Vector PreviousPos, Vector recPos, std::pair<std::string, std::pair<uint32_t,Ipv4Address> > m_pair, bool NodeRec);
Ipv4Address SelectedNeighbor (double m_dmax, Vector position);

  void ChangeTrustStatus (Ipv4Address id, Ipv4Address dest);

  void ResetTrustStatus (Ipv4Address id);

  Ipv4Address GetPreviousIp (Vector previousHop, Vector nodePos);

  bool IsInSearch (Ipv4Address id);

  bool HasPosition (Ipv4Address id);

  static Vector GetInvalidPosition ()
  {
    return Vector (-1, -1, 0);
  }

  /**
   * \brief Gets next hop according to pagpsr recovery-mode protocol (right hand rule)
   * \param previousHop the position of the node that sent the packet to this node
   * \param nodePos the position of the destination node
   * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
   */
  Ipv4Address BestAngle (Vector previousHop, Vector nodePos);

  //Gives angle between the vector CentrePos-Refpos to the vector CentrePos-node counterclockwise
  double GetAngle (Vector centrePos, Vector refPos, Vector node);

double GetAngleOld (Vector centrePos, Vector refPos, Vector node);

private:
  Time m_entryLifeTime;
  std::map<Ipv4Address, std::pair< std::pair<Vector, Time>, bool> > m_table;
  // TX error callback
  Callback<void, WifiMacHeader const &> m_txErrorCallback;
  // Process layer 2 TX error notification
  void ProcessTxError (WifiMacHeader const&);
  RstTable m_rst;
  RTable m_rtable;



};

}   // pagpsr
} // ns3
#endif /* PAGPSR_PTABLE_H */
