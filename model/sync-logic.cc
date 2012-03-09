/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
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
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         卞超轶 Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "sync-logic.h"
#include "sync-diff-leaf.h"
#include "sync-full-leaf.h"
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <sys/socket.h>
#include <vector>

using namespace std;
using namespace boost;

namespace Sync
{

SyncLogic::SyncLogic (const string &syncPrefix,
                      LogicCallback fetch,
                      CcnxWrapperPtr ccnxHandle)
  : m_syncPrefix (syncPrefix)
  , m_fetch (fetch)
  , m_ccnxHandle (ccnxHandle)
{
  srandom(time(NULL));
}

SyncLogic::~SyncLogic ()
{

}

void
SyncLogic::processSyncData (const string &name, const string &dataBuffer)
{
  string last = name.substr(name.find_last_of("/") + 1);
  stringstream ss(dataBuffer);

  const LeafContainer &fullLc = m_state.getLeaves();

  if (last == "state")
  {
    FullState full;
    ss >> full;
    BOOST_FOREACH (LeafConstPtr leaf, full.getLeaves().get<ordered>())
    {
      shared_ptr<const FullLeaf> fullLeaf = dynamic_pointer_cast<const FullLeaf>(leaf);
      const NameInfo &info = fullLeaf->getInfo();
      LeafContainer::iterator it = fullLc.find(info);
      NameInfoConstPtr pInfo = StdNameInfo::FindOrCreate(info.toString());
      SeqNo seq = fullLeaf->getSeq();

      if (it == fullLc.end())
      {
	string prefix = info.toString();
	prefix += "/";
	prefix += seq.getSession();
	m_fetch(prefix, 1, seq.getSeq());
	m_state.update(pInfo, seq);
      }
      else
      {
	SeqNo currSeq = (*it)->getSeq();
	if (currSeq < seq)
	{
	  string prefix = info.toString();
	  prefix += "/";
	  prefix += seq.getSession();

	  if (currSeq.getSession() == seq.getSession())
	    m_fetch(prefix, currSeq.getSeq() + 1, seq.getSeq());
	  else
	    m_fetch(prefix, 1, seq.getSeq());

	  m_state.update(pInfo, seq);
	}
      }
    }
  }
  else
  {
    DiffState diff;
    ss >> diff;
    BOOST_FOREACH (LeafConstPtr leaf, diff.getLeaves().get<ordered>())
    {
      shared_ptr<const DiffLeaf> diffLeaf = dynamic_pointer_cast<const DiffLeaf>(leaf);
      const NameInfo &info = diffLeaf->getInfo();
      LeafContainer::iterator it = fullLc.find(info);
      SeqNo seq = diffLeaf->getSeq();

      switch (diffLeaf->getOperation())
      {
	case UPDATE:
	  if (it == fullLc.end())
	  {
	    string prefix = info.toString();
	    prefix += "/";
	    prefix += seq.getSession();
	    m_fetch(prefix, 1, seq.getSeq());

	    NameInfoConstPtr pInfo = StdNameInfo::FindOrCreate(info.toString());
	    m_state.update(pInfo, seq);
	  }
	  else
	  {
	    SeqNo currSeq = (*it)->getSeq();
	    if (currSeq < seq)
	    {
	      string prefix = info.toString();
	      prefix += "/";
	      prefix += seq.getSession();

	      if (currSeq.getSession() == seq.getSession())
	        m_fetch(prefix, currSeq.getSeq() + 1, seq.getSeq());
	      else
	        m_fetch(prefix, 1, seq.getSeq());

	      NameInfoConstPtr pInfo = StdNameInfo::FindOrCreate(info.toString());
	      m_state.update(pInfo, seq);
	    }
	  }
	  break;

	case REMOVE:
	  if (it != fullLc.end())
	  {
	    NameInfoConstPtr pInfo = StdNameInfo::FindOrCreate(info.toString());
  	    m_state.remove(pInfo);
	  }
	  break;

	default:
	  break;
      }
    }
  }

  sendSyncInterest();
}

void
SyncLogic::addLocalNames (const string &prefix, uint32_t session, uint32_t seq)
{
  NameInfoConstPtr info = StdNameInfo::FindOrCreate(prefix);
  SeqNo seqN(session, seq);
  DiffStatePtr diff = make_shared<DiffState>();
  diff->update(info, seqN);
  m_state.update(info, seqN);
  diff->setDigest(m_state.getDigest());

  vector<string> pis = m_syncInterestTable.fetchAll();
  stringstream ss;
  ss << *diff;
  for (vector<string>::iterator ii = pis.begin(); ii != pis.end(); ++ii)
  {
    m_ccnxHandle->publishData(*ii, ss.str(), m_syncResponseFreshness);
  }
}

void
SyncLogic::respondSyncInterest (const string &interest)
{
  string hash = interest.substr(interest.find_last_of("/") + 1);
  Digest digest;
  digest << hash;

  if (*m_state.getDigest() == digest)
  {
    m_syncInterestTable.insert(interest);
    return;
  }
/*
  DiffStateContainer::index<hashed>::type& idx = m_log.get<hashed> ();
  DiffStateContainer::iterator ii = idx.find(digest);

  if (ii != idx.end())
  {
    stringstream ss;
    ss << *(*ii)->diff();
    m_ccnxHandle->publishData(interest, ss.str(), m_syncResponseFreshness);
  }
  else
  {
    int wait = rand() % 100 + 20;
    sleep(wait/1000.0);
  }

  if (*m_state.getDigest() == digest)
  {
    m_syncInterestTable.insert(interest);
    return;
  }

  ii = idx.find(digest);
  if (ii != idx.end())
  {
    stringstream ss;
    ss << *(*ii)->diff();
    m_ccnxHandle->publishData(interest, ss.str(), m_syncResponseFreshness);
  }
  else
  {
    stringstream ss;
    ss << m_state;
    m_ccnxHandle->publishData(interest + "/state", ss.str(), m_syncResponseFreshness);
  }
  */
}

void
SyncLogic::sendSyncInterest ()
{
  ostringstream os;
  os << m_syncPrefix << "/" << m_state.getDigest();

  m_ccnxHandle->sendInterest (os.str (),
                              bind (&SyncLogic::processSyncData, this, _1, _2));
}

}