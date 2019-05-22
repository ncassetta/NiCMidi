
/// \file
/// Contains the definition of the pure virtual class MIDITickComponent.


#ifndef TICK_H_INCLUDED
#define TICK_H_INCLUDED

#include <atomic>
#include <mutex>
#include "timer.h"

/// \addtogroup GLOBALS
//@{
/// These are the available priorities for a MIDITickComponent. When you add a component to the MIDIManager queue with
/// the MIDIManager::SetComponent() method the order in the queue reflects the priority of the components.
/// \see MIDIManager::AddComponent().
typedef enum {
    PR_FIRST,           ///< The component is inserted as first element in the queue
    PR_PRE_SEQ,         ///< The component is inserted before the sequencer
    PR_SEQ,             ///< The component is the sequencer (you can insert only one in the queue)
    PR_POST_SEQ,        ///< The component is inserted after the sequencer
    PR_LAST             ///< The component is inserted as last element in the queue
} tPriority;
//@}


///
/// A pure virtual class implementing an object which has a callback procedure to be called at every tick
/// of a MIDITimer.
/// You can use this feature to send, receive or manipulate MIDI messages with accurate timing: the MIDISequencer,
/// MIDIThru and MIDIRecorder classes inherit by this.
/// To use a MIDITickComponent object you must add it to the MIDIManager queue with the MIDIManager::AddMIDITick()
/// method; then you can call the Start() and Stop() methods of the object to start and stop the callback. A priority
/// parameter is supplied to determine the position of the MIDITickComponent in the MIDIManager queue.
///
class MIDITickComponent {
    public:
        /// The constructor.
        /// \param pr the priority (see \ref tPriority)
        /// \param func a pointer to the static callback: this should be the address of the StaticTickProc() you
        /// have implemented in your subclass
                                    MIDITickComponent(tPriority pr, MIDITick func) : tick_proc(func),
                                                      dev_time_offset(0), sys_time_offset(0),
                                                      priority(pr), running(false) {}

        /// The destructor.
        /// Before deleting the object it tries to remove its pointer from the MIDIManager queue to prevent the
        /// manager from using an invalid pointer. If a derived class destructor needs to delete something it
        /// should call Stop() as first statement, then do its work.
        virtual                    ~MIDITickComponent();


        /// A pure virtual method reinitializing the class parameters.
        virtual void                Reset() = 0;

        /// Returns the address of the StaticTickProc() method, which will be called by the MIDIManager at every
        /// clock tick.
        MIDITick*                   GetFunc() const                 { return tick_proc; }
        /// Returns the priority.
        tPriority                   GetPriority() const             { return priority; }

        /// Sets the running status as **true** and starts to call the callback. Moreover it set
        /// \ref sys_time_offset to the now time so, at every subsequent call of the callback, you can calculate
        /// the elapsed time. In your derived class you probably will want to redefine this for doing other
        /// things before starting the callback: however in your derived method you must call the base class
        /// method.
        ///
        /// You must call this **after** inserting the object into the MIDIManager queue with the
        /// MIDIManager::AddMIDITick() method, or you will have no effect.
        /// \param dev_offs a time offset you can use for your calculations (for example, in the MIDISequencer
        /// it is used as the start time of the sequencer)
        virtual void                Start(tMsecs dev_offs = 0);
        /// Sets the running status as **false** and stops the callback.
        /// \see Start()
        virtual void                Stop();

        /// Returns **true** if the callback procedure is active.
        bool                        IsPlaying() const               { return running.load(); }


    protected:
        /// This is the static callback procedure which the MIDIManager will call at every MIDITimer tick. The parameters
        /// are automatically set by the %MIDIManager at every function call.
        /// \param sys_time the now system time
        /// \param pt the _this_ pointer of the object instance.
        ///
        /// You must implement it in your subclass and give the function address in the constructor. Typically this should
        /// only cast the void pointer *pt* to a pointer to your object and then call the pt->TickProc(sys_time), i.e\. your
        /// non static procedure.
        static void                 StaticTickProc(tMsecs sys_time, void* pt)   {}
        /// This is the pure virtual function you must implement in your subclass.
        virtual void                TickProc(tMsecs sys_time) = 0;

        /// The pointer to the static callback (probably set by the constructor to StaticTickProc()).
        const MIDITick*             tick_proc;

        /// A time offset set by the Start() method and which you can use for your calculations.
        tMsecs                      dev_time_offset;
        /// The system time of the last call of Start(). You can use this for calculating the time elapsed between the
        /// start of the callback and the actual call of TickProc().
        tMsecs                      sys_time_offset;
        /// A mutex you can use for implementing thread safe methods (it is not used by the base class).
        std::recursive_mutex        proc_lock;

    private:
        const tPriority             priority;
        std::atomic<bool>           running;
};

#endif // TICK_H_INCLUDED
