/*
 * MOOSAsyncCommClient.cpp
 *
 *  Created on: Sep 18, 2012
 *      Author: pnewman
 */

#include <cmath>
#include <string>
#include <set>
#include <limits>
#include <iostream>
#include <iomanip>
#include <cassert>


#include "MOOS/libMOOS/Utils/MOOSUtils.h"
#include "MOOS/libMOOS/Utils/MOOSException.h"
#include "MOOS/libMOOS/Utils/MOOSScopedLock.h"
#include "MOOS/libMOOS/Utils/ConsoleColours.h"

#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"
#include "MOOS/libMOOS/Comms/XPCTcpSocket.h"

namespace MOOS
{

#define TIMING_MESSAGE_PERIOD 1.0

bool AsyncCommsReaderDispatch(void * pParam)
{
	MOOSAsyncCommClient *pMe = (MOOSAsyncCommClient*)pParam;
	return pMe->ReadingLoop();
}

bool AsyncCommsWriterDispatch(void * pParam)
{
	MOOSAsyncCommClient *pMe = (MOOSAsyncCommClient*)pParam;

	return pMe->WritingLoop();
}

///default constructor
MOOSAsyncCommClient::MOOSAsyncCommClient()
{
	m_dfLastTimingMessage = 0.0;
	m_dfOutGoingDelay = 0.0;
}
///default destructor
MOOSAsyncCommClient::~MOOSAsyncCommClient()
{
	Close();
}


std::string MOOSAsyncCommClient::HandShakeKey()
{
	return "asynchronous";
}

void MOOSAsyncCommClient::DoBanner()
{
    if(m_bQuiet)
        return ;

	MOOSTrace("----------------------------------------------------\n");
	MOOSTrace("|       This is an Asynchronous MOOS Client        |\n");
	MOOSTrace("|       c. P. Newman U. Oxford 2001-2012           |\n");
	MOOSTrace("----------------------------------------------------\n");

}



bool MOOSAsyncCommClient::StartThreads()
{
	m_bQuit = false;

    if(!WritingThread_.Initialise(AsyncCommsWriterDispatch,this))
        return false;

    if(!ReadingThread_.Initialise(AsyncCommsReaderDispatch,this))
            return false;

    if(!WritingThread_.Start())
        return false;

    if(!ReadingThread_.Start())
        return false;

    return true;
}

bool MOOSAsyncCommClient::Close(bool  )
{

	BASE::Close();

	if(!ReadingThread_.Stop())
		return false;

	if(!WritingThread_.Stop())
	    return false;


	return true;
}


bool MOOSAsyncCommClient::Flush()
{
	return true;
}



bool MOOSAsyncCommClient::Post(CMOOSMsg & Msg,bool bKeepMsgSourceName)
{


	BASE::Post(Msg,bKeepMsgSourceName);

	m_OutLock.Lock();
	{
		if(OutGoingQueue_.Size()>OUTBOX_PENDING_LIMIT)
		{
			std::cerr<<MOOS::ConsoleColours::red()<<"WARNING "<<MOOS::ConsoleColours::reset()<<
					"MOOSAsyncCommClient::Outbox is very full "
					"- ditching half of the unsent mail\n";

			while(OutGoingQueue_.Size()>OUTBOX_PENDING_LIMIT/2)
				OutGoingQueue_.Pop();
		}
		OutGoingQueue_.AppendToMeInConstantTime(m_OutBox);
		//std::cerr<<"OutGoingQueue_ : "<<OutGoingQueue_.Size()<<"\n";
	}
	m_OutLock.UnLock();

	return true;
}

bool MOOSAsyncCommClient::OnCloseConnection()
{
	return BASE::OnCloseConnection();
}

bool MOOSAsyncCommClient::IsAsynchronous()
{
	return true;
}

bool MOOSAsyncCommClient::WritingLoop()
{
	//we want errors not signals!
#ifndef _WIN32
    signal(SIGPIPE,SIG_IGN);
#endif


	while(!WritingThread_.IsQuitRequested())
	{

		//this is the connect loop...
		m_pSocket = new XPCTcpSocket(m_lPort);

		//reset counters
		m_nBytesSent = 0;
		m_nBytesReceived = 0;


		if(ConnectToServer())
		{
			int nMSToWait  = 1000;

			m_dfLastSendTime = MOOSLocalTime();

			while(!WritingThread_.IsQuitRequested() && IsConnected() )
			{
				if(OutGoingQueue_.Size()==0)
				{
					//this may time out in which case we DoWriting() which may send
					//a timing message (heart beat) in Do Writing...
					OutGoingQueue_.WaitForPush(nMSToWait);
				}

				if(!DoWriting())
				{
					OnCloseConnection();
				}
			}
		}
		else
		{
			//this is bad....if ConnectToServer() returns false
			//it wasn't simply that we could not get hold of the server
			//it was that we misbehaved badly. We should quit..
			OnCloseConnection();
			break;
		}

	}
	//clean up on exit....
	if(m_pSocket!=NULL)
	{
		if(m_pSocket)
			delete m_pSocket;
		m_pSocket = NULL;
	}

	if(m_bQuiet)
		MOOSTrace("CMOOSAsyncCommClient::WritingLoop() quits\n");

	m_bConnected = false;

	return true;
}




bool MOOSAsyncCommClient::DoWriting()
{

	//this is the IO Loop
	try
	{

		if(!IsConnected())
			return false;

		MOOSMSG_LIST StuffToSend;

		OutGoingQueue_.AppendToOtherInConstantTime(StuffToSend);

		for(MOOSMSG_LIST::iterator q = StuffToSend.begin();q!=StuffToSend.end();q++)
		{
			if(q->IsType(MOOS_TERMINATE_CONNECTION))
			{
				std::cout<<"writing thread receives terminate connection request from sibling reader thread\n";
				return false;
			}
		}

		//and once in a while we shall send a timing
		//message (this is the new style of timing
		if((MOOSLocalTime()-m_dfLastTimingMessage)>TIMING_MESSAGE_PERIOD  )
		{
			CMOOSMsg Msg(MOOS_TIMING,"_async_timing",0.0,MOOSLocalTime());
//			std::cerr<<"forming timing message "<<std::fixed<<std::setprecision(4)<<Msg.GetTime()<<std::endl;
			StuffToSend.push_front(Msg);
			m_dfLastTimingMessage= Msg.GetTime();
		}

		if(StuffToSend.empty())
		{
			return true;
		}

		//convert our out box to a single packet
		CMOOSCommPkt PktTx;
		try
		{
			PktTx.Serialize(StuffToSend,true);
			m_nBytesSent+=PktTx.GetStreamLength();
		}
		catch (const CMOOSException & e)
		{
			//clear the outbox
			throw CMOOSException("Serialisation Failed - this must be a lot of mail...");
		}

		//finally the send....
		SendPkt(m_pSocket,PktTx);

		MonitorAndLimitWriteSpeed();

	}
	catch(const CMOOSException & e)
	{
		MOOSTrace("Exception in DoWriting() : %s\n",e.m_sReason);
		return false;//jump out to connect loop....
	}

	return true;


}

bool MOOSAsyncCommClient::MonitorAndLimitWriteSpeed()
{
	unsigned int sleep_ms = m_dfOutGoingDelay*1000;
	if(sleep_ms>0)
	{
		MOOSPause(sleep_ms);
	}

	return true;
}

bool MOOSAsyncCommClient::ReadingLoop()
{
	//note we will rely on our sibling writing thread to handle
	//the connected and reconnecting...
#ifndef _WIN32
    signal(SIGPIPE,SIG_IGN);
#endif	

	while(!ReadingThread_.IsQuitRequested())
	{
		if(IsConnected())
		{
			if(!DoReading())
			{
				OutGoingQueue_.Push(CMOOSMsg(MOOS_TERMINATE_CONNECTION,"-quit-",0)   );

				std::cout<<"reading failed!\n";

				while(IsConnected())//wait for connection to terminate...
					MOOSPause(200);
			}
		}
		else
		{
			//we arent connected so do nothing...
			MOOSPause(100);
		}
	}
	std::cout<<"READING LOOP quiting...\n";
	return true;
}

bool MOOSAsyncCommClient::DoReading()
{

	try
	{
		CMOOSCommPkt PktRx;

		ReadPkt(m_pSocket,PktRx);

		m_nBytesReceived+=PktRx.GetStreamLength();


		double dfLocalRxTime =MOOSLocalTime();

		m_InLock.Lock();
		{
			if(m_InBox.size()>m_nInPendingLimit)
			{
				MOOSTrace("Too many unread incoming messages [%d] : purging\n",m_InBox.size());
				MOOSTrace("The user must read mail occasionally");
				m_InBox.clear();
			}

			//how big was inbox before we add new mail?
			unsigned int nur = m_InBox.size();

			//extract... and please leave NULL messages there
			PktRx.Serialize(m_InBox,false,false,NULL);

			//now Serialize simply adds to the front of a list so looking
			//at the first element allows us to check for timing information
			//as supported by the threaded server class
			if(m_bDoLocalTimeCorrection)
			{

				MOOSMSG_LIST::iterator q = m_InBox.begin();
				std::advance(q,nur);

				switch(q->GetType())
				{
					case MOOS_TIMING:
					{
						//we have a fancy new DB upstream...
						//one that supports Asynchronous Clients
						UpdateMOOSSkew(q->GetTime(),
								q->GetDouble(),
								dfLocalRxTime);

						//and we can update the outgoing thread's speed
						//as controlled by the DB.
						m_dfOutGoingDelay = q->GetDoubleAux();


						m_InBox.erase(q);

						break;
					}
					case MOOS_NULL_MSG:
					{
						//looks like we have an old fashioned DB which sends timing
						//info at the front of every packet in a null message
						//we have no corresponding outgoing packet so not much we can
						//do other than imagine it tooks as long to send to the
						//DB as to receive...
						double dfTimeSentFromDB = m_InBox.front().GetDouble();
						double dfSkew = dfTimeSentFromDB-dfLocalRxTime;
						double dfTimeSentToDBApprox =dfTimeSentFromDB+dfSkew;

						m_InBox.pop_front();

						UpdateMOOSSkew(dfTimeSentToDBApprox,
								dfTimeSentFromDB,
								dfLocalRxTime);

						break;

					}
				}
			}

			//here we dispatch to special call backs managed by threads
			MOOSMSG_LIST::iterator t = m_InBox.begin();
			while(t!=m_InBox.end())
			{
				std::map<std::string, MOOS::ActiveMailQueue* >::iterator q = ActiveQueues_.find(t->GetKey());
				if(q!=ActiveQueues_.end())
				{
					q->second->Push(*t);
					t = m_InBox.erase(t);
				}
				else
				{
					t++;
				}
			}

			m_bMailPresent = !m_InBox.empty();


		}
		m_InLock.UnLock();

		//and here we can optionally give users an indication
		//that mail has arrived...
		if(m_pfnMailCallBack!=NULL && m_bMailPresent)
		{
			bool bUserResult = (*m_pfnMailCallBack)(m_pMailCallBackParam);
			if(!bUserResult)
				MOOSTrace("user mail callback returned false..is all ok?\n");
		}
	}
	catch(const CMOOSException & e)
	{
		MOOSTrace("Exception in DoReading() : %s\n",e.m_sReason);
		return false;
	}

	return true;
}

bool MOOSAsyncCommClient::IsRunning()
{
	return WritingThread_.IsThreadRunning() || ReadingThread_.IsThreadRunning();
}


};

