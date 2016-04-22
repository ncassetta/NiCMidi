#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>


using namespace std::chrono;

std::string asString (const system_clock::time_point& tp)
{
    time_t t = system_clock::to_time_t(tp);        // convert to system time
    std::string ts = ctime(&t);                    // convert to calendar time
    ts.resize(ts.size()-1);                        // skip trailing newline
    return ts;
}


class MyClock {
    public:

        MyClock() {
            timebase = system_clock::now();
            std::cout << asString(timebase); }

        unsigned long get_msecs() { return duration_cast<milliseconds>(system_clock::now() - timebase).count(); }

    private:
        time_point<system_clock> timebase;
};




template <typename C>
void printClockData ()
{
    using namespace std;

    cout << "- precision: ";
    // if time unit is less or equal one millisecond
    typedef typename C::period P;// type of time unit
    if (ratio_less_equal<P,milli>::value) {
       // convert to and print as milliseconds
       typedef typename ratio_multiply<P,kilo>::type TT;
       cout << fixed << double(TT::num)/TT::den
            << " milliseconds" << endl;
    }
    else {
        // print as seconds
        cout << fixed << double(P::num)/P::den << " seconds" << endl;
    }
    cout << "- is_steady: " << boolalpha << C::is_steady << endl;
}


int main()
{

    MyClock clock;

    for(int i = 0; i < 1000; i++)
        std::cout << clock.get_msecs() << std::endl;
    /*
    std::cout << "system_clock: " << std::endl;
    printClockData<std::chrono::system_clock>();
    std::cout << "\nhigh_resolution_clock: " << std::endl;
    printClockData<std::chrono::high_resolution_clock>();
    std::cout << "\nsteady_clock: " << std::endl;
    printClockData<std::chrono::steady_clock>();
    */
}





/*
	This is part of CFugue, a C++ Runtime for MIDI Score Programming
	Copyright (C) 2009 Gopalakrishna Palem

	For links to further information, or to contact the author,
	see <http://cfugue.sourceforge.net/>.

    $LastChangedDate: 2014-05-12 12:58:33 +0530 (Mon, 12 May 2014) $
    $Rev: 212 $


#ifndef __ALSADRIVER_H__9857EC39_234E_411E_9558_EFDA218796AA__
#define __ALSADRIVER_H__9857EC39_234E_411E_9558_EFDA218796AA__

/** @file AlsaDriver.h
 * \brief Declares MIDIDriverAlsa class for CFugue
 */
#include "rtmidi/RtMidi.h"
#include "jdkmidi/msg.h"
#include "jdkmidi/driver.h"
#include "jdkmidi/sequencer.h"

#include <thread> // std::thread
#include <future> // std::future

namespace CFugue
{
	///<Summary>MIDI Driver for Linux Alsa based machines</Summary>
	class MIDIDriverAlsa : public jdkmidi::MIDIDriver
	{
		RtMidiIn*		    m_pMidiIn;
		RtMidiOut*	        m_pMidiOut;
        std::future<bool>   m_bgTaskResult;
	public:
		MIDIDriverAlsa ( int queue_size );
		virtual ~MIDIDriverAlsa();

		void ResetMIDIOut();

        /// <Summary>
        /// Creates a background thread to pump MIDI events
        /// at the supplied timer resolution.
        /// Use WaitTillDone() to wait till the background processing completes.
        /// Use StopTimer() after the background processing is completed, to release resources.
        /// @param resolution_ms MIDI Timer resolution in milliseconds
        /// @return false if background thread cannot be started
        /// </Summary>
		bool StartTimer ( int resolution_ms );

		/// Waits (blocks) till the background thread created with StartTimer()
		/// completes its processing.
		/// After StartTimer() succeeds, use WaitTillDone() followed by StopTimer().
		/// Returns immediately if no background thread is running.
		void WaitTillDone();

        /// Call StopTimer() to release the resources used by the background
        /// procedure created with StartTimer(). StopTimer() Should be called
        /// <i>after</i> the background procedure is done (indicated by BGThreadStatus::COMPLETED).
        /// If background procedure is still running while StopTimer() is called, caller gets
        /// blocked till the background procedure completes.
        /// If no background procedure exists, returns immediately.
		void StopTimer();

		/// Opens the MIDI input port with the given ID
		/// @return false if the given input port cannot be opened
		bool OpenMIDIInPort ( int id );

        /// Opens the MIDI output port with the given ID
        /// @return false if the given output port cannot be opened
		bool OpenMIDIOutPort ( int id );

		/// Closed any previously opened MIDI Input port
		void CloseMIDIInPort();

		/// Closed any previously opened MIDI Output port
		void CloseMIDIOutPort();

		enum BGThreadStatus {   RUNNING,    ///< Async procedure is running - use WaitTillDone() to wait for completion
                                COMPLETED,  ///< Async procedure completed running - use StopTimer() to finish
                                INVALID     ///< No background procedure running - use StartTimer() to start one
                            };
	protected:
		bool HardwareMsgOut ( const jdkmidi::MIDITimedBigMessage &msg );

		std::thread* m_pThread;

		int timer_id;
		int timer_res;

		//bool in_open;
		//bool out_open;
		//bool timer_open;
	};

} // namespace CFugue

#endif // __ALSADRIVER_H__9857EC39_234E_411E_9558_EFDA218796AA__
/*
	This is part of CFugue, a C++ Runtime for MIDI Score Programming
	Copyright (C) 2011 Gopalakrishna Palem

	For links to further information, or to contact the author,
	see <http://cfugue.sourceforge.net/>.
*/
#if defined _WIN32 || defined WIN32

#else	// only if not Windows

#include "AlsaDriver.h"
#include "MidiTimer.h"

using namespace jdkmidi;

namespace CFugue
{
	MIDIDriverAlsa::MIDIDriverAlsa ( int queue_size )
		:
		MIDIDriver ( queue_size ),
		m_pMidiIn ( 0 ),
		m_pMidiOut ( 0 ),
		m_pThread ( NULL )
	{
	}

	MIDIDriverAlsa::~MIDIDriverAlsa()
	{
		StopTimer();
		CloseMIDIInPort();
		CloseMIDIOutPort();
	}

	bool MIDIDriverAlsa::OpenMIDIInPort ( int id )
	{
		if(m_pMidiIn != NULL) // close any open port
		{
			delete m_pMidiIn;
			m_pMidiIn = NULL;
		}
		if(m_pMidiIn == NULL)
		{
			try
			{
				m_pMidiIn = new RtMidiIn("MIDIDriverAlsa Client");
			}
			catch ( RtError &error )
			{
				error.printMessage();
				return false;
			}
		}
		if(m_pMidiIn != NULL)
		{
			try
			{
				m_pMidiIn->openPort(id);
			}
			catch(RtError& error)
			{
				error.printMessage();
				return false;
			}
		}
		return true;
	}


	bool MIDIDriverAlsa::OpenMIDIOutPort ( int id )
	{
		if(m_pMidiOut != NULL) // close any open port
		{
			delete m_pMidiOut;
			m_pMidiOut = NULL;
		}
		if(m_pMidiOut == NULL)
		{
			try
			{
				m_pMidiOut = new RtMidiOut("MIDIDriverAlsa Client");
			}
			catch(RtError &error)
			{
				error.printMessage();
				return false;
			}
		}
		if(m_pMidiOut != NULL)
		{
			try
			{
				m_pMidiOut->openPort(id);
			}
			catch(RtError &error)
			{
				error.printMessage();
				return false;
			}
		}
		return true;
	}

    bool MIDIDriverAlsa::HardwareMsgOut ( const jdkmidi::MIDITimedBigMessage &msg )
    {
        if(m_pMidiOut != NULL)
        {
            unsigned char status = msg.GetStatus();

            // dont send sysex or meta-events

            if ( status <0xff && status !=0xf0 )
            {
                unsigned char msgBytes[] = {status, msg.GetByte1(), msg.GetByte2(), msg.GetByte3()};

                std::vector<unsigned char> vec(msgBytes, msgBytes+3);

                m_pMidiOut->sendMessage(&vec);
            }

            return true;
        }
        return false;
    }

    // This is thread procedure to pump MIDI events
    // We maintain the supplied Timer Resolution by adjusting the sleep duration
	bool AlsaDriverThreadProc(MIDIDriverAlsa* pAlsaDriver, int nTimerResMS)
	{
		MidiTimer::TimePoint tBefore, tAfter;
	    unsigned long nElapsed, nTimeToSleep;

	    while(true)
	    {
            tBefore = MidiTimer::Now();

            if(pAlsaDriver->TimeTick(tBefore) == false) break;

            tAfter = MidiTimer::Now();

			nElapsed = std::chrono::duration_cast<MidiTimer::Duration>(tAfter - tBefore).count();

            nTimeToSleep = (nElapsed > nTimerResMS ? 0 : nTimerResMS - nElapsed);

            MidiTimer::Sleep(nTimeToSleep);
	    }

        return true;
	}

	bool MIDIDriverAlsa::StartTimer ( int res )
	{
	    if(m_bgTaskResult.valid()) // Already running
            return false;

        m_bgTaskResult = std::async(std::launch::async, &AlsaDriverThreadProc, this, res);

        return m_bgTaskResult.valid();
	}

	void MIDIDriverAlsa::WaitTillDone()
	{
	    if(m_bgTaskResult.valid() == false) return; // if not running

        auto waitStatus = m_bgTaskResult.wait_for(std::chrono::milliseconds(0));

        while(waitStatus != std::future_status::ready)
        {
             waitStatus = m_bgTaskResult.wait_for(std::chrono::milliseconds(500));
        }
	}

	void MIDIDriverAlsa::StopTimer()
	{
	    // std::future requires get() to be called before it can be used again.
	    // valid() keeps returning true till get() is called. And get() can be
	    // called only once. Once it is called valid() becomes false again.
	    if(m_bgTaskResult.valid())
            m_bgTaskResult.get();
	}

	void MIDIDriverAlsa::CloseMIDIInPort()
	{
	    if(m_pMidiIn != NULL)
	    {
	        m_pMidiIn->closePort();
	        delete m_pMidiIn;
	        m_pMidiIn = NULL;
	    }
	}

	void MIDIDriverAlsa::CloseMIDIOutPort()
	{
	    if(m_pMidiOut != NULL)
	    {
	        m_pMidiOut->closePort();
	        delete m_pMidiOut;
	        m_pMidiOut = NULL;
	    }
	}

} // namespace CFugue



#endif // _ifndef _Win32

