// check if the build environment is Windows, and then include the Windows API header,
// else include SWELL which provides functions with the same names but are implemented
// for another operating system (namely, OS-X)
#ifdef _WIN32
#include <windows.h>
#include "WDL/WDL/win32_utf8.h"
#pragma warning ( disable : 4267 ) // size_t to int
#pragma warning ( disable : 4244 ) // double to int
#pragma warning ( disable : 4800 ) // 'unsigned __int64': forcing value to bool 'true' or 'false' (performance warning)
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include "WDL/WDL/lice/lice.h"
#define REAPERAPI_IMPLEMENT
#include "reaper_plugin/reaper_plugin_functions.h"

#include "utilfuncs.h"
#include "reaper_action_helper.h"
#include "reaper_function_helper.h"

#include <stdio.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>

reaper_plugin_info_t* g_plugin_info = nullptr;
REAPER_PLUGIN_HINSTANCE g_hInst; // handle to the dll instance. could be useful for making win32 API calls
HWND g_parent; // global variable that holds the handle to the Reaper main window, useful for various win32 API calls

#include "main.hpp" 
#include "reascript.hpp" /*** HERE THE FUNCTIONS DO THEIR WORK ***/

// load surface toggling actions
#include "surface_togglers.h" 

void doAction1() {
	ShowMessageBox("Hello World!", "Reaper extension", 0);
}

void doAction2(action_entry& act) {
	// this action does nothing else but toggles the variable that keeps track of the toggle state
	// so it's useless as such but you can see the action state changing in the toolbar buttons and the actions list
	if (act.m_togglestate == ToggleOff)
		act.m_togglestate = ToggleOn;
	else act.m_togglestate = ToggleOff;
	// store new state of toggle action to ini file immediately
	char buf[8];
	// the REAPER api for ini-files only deals with strings, so form a string from the action
	// toggle state number.
	int toggletemp = 0;
	if (act.m_togglestate == ToggleOn)
		toggletemp = 1;
	sprintf(buf, "%d", toggletemp);
	SetExtState("simple_extension", "toggleaction_state", buf, true);
}

void doAction3(action_entry& act) {
	readbg() << "action in cycle state " << act.m_cycle_state << "\n";
	act.m_cycle_state = (act.m_cycle_state + 1) % 3;
}

void doChangeItemPitchesAction(action_entry& act)
{
	int num_sel_items = CountSelectedMediaItems(nullptr);
	for (int i = 0; i < num_sel_items; ++i)
	{
		MediaItem* item = GetSelectedMediaItem(nullptr, i);
		MediaItem_Take* take = GetActiveTake(item);
		if (take != nullptr)
		{
			double pch = bound_value(-12.0,
				map_value(act.m_ex_val, 0, 127, -12.0, 12.0),12.0);
			SetMediaItemTakeInfo_Value(take, "D_PITCH", pch);
		}
	}
	UpdateArrange();
}

void test_track_range()
{
	int sanity = 0;
	for (auto e : reaper_track_range())
	{
		if (e == nullptr)
		{
			readbg() << "should not get nullptr!\n";
			break;
		}
		char buf[1024];
		if (GetSetMediaTrackInfo_String(e, "P_NAME", buf, false))
			readbg() << e << " " << buf << "\n";
		++sanity;
		if (sanity > 10)
		{
			readbg() << "sanity failed\n";
			break;
		}
	}
}

class irp_task : public IParallelTask
{
public:
	// Have to initialize stuff in GUI thread
	irp_task(MediaItem* item, int id) : m_item(item), m_id(id)
	{
		MediaItem_Take* take = GetActiveTake(m_item);
		if (take != nullptr)
		{
			PCM_source* src = GetMediaItemTake_Source(take);
			if (src != nullptr)
			{
				m_src = src->Duplicate();
				m_shifter = ReaperGetPitchShiftAPI(REAPER_PITCHSHIFT_API_VER);
				char cfg[] = { 'e','v','a','w', 32, 0 };
				int nch = src->GetNumChannels();
				int sr = src->GetSampleRate();
				char buf[2048];
				GetProjectPath(buf, 2048);
				std::string outfn = std::string(buf)+"out_" + std::to_string(id) + ".wav";
				readbg() << "sink fn : " << outfn << "\n";
				m_sink = PCM_Sink_Create(outfn.c_str(), cfg, sizeof(cfg), nch, sr, false);
				if (m_sink == nullptr)
				{
					readbg() << "failed to create sink\n";
					return;
				}
				m_shifteroutbuf.resize(nch*m_bufsize);
				m_sinkbuf.resize(nch*m_bufsize);
				m_sinkbufpointers.resize(nch);
				for (int i = 0; i < nch; ++i)
					m_sinkbufpointers[i] = &m_sinkbuf[i*m_bufsize];
			}
		}
	}
	// Have to destroy stuff in GUI thread
	~irp_task()
	{
		delete m_sink;
		delete m_shifter;
		delete m_src;
		//readbg() << "irp task dtor " << m_id << "\n";
	}
	// Multithreading compatible code put in this method
	// Stuff like ShowConsoleMsg, Main_OnCommand, Reaper object creation functions etc can't be used here
	void run() override
	{
		if (m_shifter == nullptr || m_src == nullptr || m_sink == nullptr)
			return;
		double prate = 0.5;
		m_shifter->SetQualityParameter(-1);
		m_shifter->set_nch(m_src->GetNumChannels());
		m_shifter->set_srate(m_src->GetSampleRate());
		m_shifter->set_tempo(prate);
		m_shifter->set_shift(1.0);
		
		int64_t srclenframes = m_src->GetLength()*m_src->GetSampleRate();
		int64_t incounter = 0;
		while (incounter < srclenframes)
		{
			ReaSample* shifter_in_buf = m_shifter->GetBuffer(m_bufsize*prate);
			PCM_source_transfer_t transfer = { 0 };
			transfer.length = m_bufsize*prate;
			transfer.nch = m_src->GetNumChannels();
			transfer.time_s = (double)incounter / m_src->GetSampleRate();
			transfer.samplerate = m_src->GetSampleRate();
			transfer.samples = shifter_in_buf;
			m_src->GetSamples(&transfer);
			int shifted_output = 0;
			int sanity = 0;
			while (shifted_output < m_bufsize)
			{
				
				m_shifter->BufferDone(m_bufsize*prate);
				shifted_output += m_shifter->GetSamples(m_bufsize, m_shifteroutbuf.data());
				
				++sanity;
				if (sanity > 100)
				{
					//readbg() << "sanity failed";
					break;
				}
			}
			int nch = m_src->GetNumChannels();
			for (int i = 0; i < m_bufsize; ++i)
			{
				for (int j = 0; j < nch; ++j)
				{
					m_sinkbufpointers[j][i] = m_shifteroutbuf[i*nch + j];
				}
			}
			m_sink->WriteDoubles(m_sinkbufpointers.data(), m_bufsize, m_src->GetNumChannels(), 0, 1);
			incounter += m_bufsize*prate;
		}
	}
	MediaItem* m_item = nullptr;
	IReaperPitchShift* m_shifter = nullptr;
	PCM_source* m_src = nullptr;
	PCM_sink* m_sink = nullptr;
	int m_bufsize = 16384;
	std::vector<double> m_shifteroutbuf;
	std::vector<double> m_sinkbuf;
	std::vector<double*> m_sinkbufpointers;
	int m_id = 0;
};

void test_irp_render(bool multithreaded)
{
	int numselitems = CountSelectedMediaItems(nullptr);
	if (numselitems < 1)
		return;
	std::vector<std::shared_ptr<IParallelTask>> tasks;
	for (int i = 0; i < numselitems; ++i)
	{
		MediaItem* item = GetSelectedMediaItem(nullptr, i);
		auto task = std::make_shared<irp_task>(item,i);
		tasks.push_back(task);
	}
	double t0 = time_precise();
	execute_parallel_tasks(tasks, multithreaded);
	double t1 = time_precise();
	readbg() << "all done in " << t1 - t0 << " seconds\n";
}

void test_netlib()
{
	JNL_HTTPGet netget;
	std::vector<char> pagedata;
	std::vector<char> tempdata(65536);
	netget.connect("http://www.landoleet.org/reaper512rc2-install.exe");
	while (true)
	{
		int r = netget.run();
		if (r==-1)
		{
			readbg() << "net error\n";
			break;
		}
		if (r==1)
		{
			readbg() << "connection has closed\n";
			break;
		}
		int avail = netget.bytes_available();
		if (avail>0)
		{
			if (tempdata.size()<avail)
				tempdata.resize(avail);
			netget.get_bytes(tempdata.data(), avail);
			for (int i=0;i<avail;++i)
				pagedata.push_back(tempdata[i]);
		}
		//readbg() << "... ";
		Sleep(50);
	}
	//readbg() << "\n";
	if (pagedata.size()>0)
	{
		readbg() << pagedata.size() << " bytes downloaded\n";
		//readbg() << pagedata.data();
	}
}

extern "C"
{
	// this is the only function that needs to be exported by a Reaper extension plugin dll
	// everything then works from function pointers and others things initialized within this
	// function
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec) {
		g_hInst=hInstance;
		if (rec) {
			if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
				return 0; /*todo: proper error*/
			g_plugin_info = rec;
			g_parent = rec->hwnd_main;

			// load all Reaper API functions in one go, byebye ugly IMPAPI macro!
			int error_count = REAPERAPI_LoadAPI(rec->GetFunc);
			if (error_count > 0)
			{
				char errbuf[256];
				sprintf(errbuf, "Failed to load %d expected API function(s)", error_count);
				MessageBox(g_parent, errbuf, "MRP extension error", MB_OK);
				return 0;
			}

#ifndef WIN32
			// Perhaps to get Reaper faders on OSX...
			SWELL_RegisterCustomControlCreator((SWELL_ControlCreatorProc)rec->GetFunc("Mac_CustomControlCreator"));
#endif
			
			/// ------------------------------------------------- PTL changes start -- add some actions
			surface_togglers::ConfigParser configParser;
			for (auto& name : configParser.getTogglerNames())
			{
				surface_togglers::Toggler::registerActions(name);
			}
			surface_togglers::Toggler::registerClearAllTogglersAction();
			
			/// ------------------------------------------------- PTL changes end

			if (!rec->Register("hookcommand2", (void*)hookCommandProcEx)) { 
				MessageBox(g_parent, "Could not register hookcommand2", "MRP extension error", MB_OK);
			}
			if (!rec->Register("toggleaction", (void*)toggleActionCallback)) { 
				MessageBox(g_parent, "Could not register toggleaction", "MRP extension error", MB_OK);
			}
			if (!RegisterExportedFuncs(rec)) { /*todo: error*/ }
		
			start_or_stop_main_thread_executor(false);
			return 1; // our plugin registered, return success
		}
		else {
			test_pcm_source(1);
			start_or_stop_main_thread_executor(true);
			return 0;
		}
	}
};
