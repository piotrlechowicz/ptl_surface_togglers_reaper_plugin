#pragma once

#include <string>
#include <filesystem>

namespace surface_togglers
{
//---------------------------------------
// parsing config file
//---------------------------------------

class ConfigParser
{
private:
	std::filesystem::path configPathSuffix {"./PTL/SurfTog/config.txt"};
public:
	std::vector<std::string> getTogglerNames();
};

//---------------------------------------
// Actions
//---------------------------------------

/**
 * Set state of toggle in reaper defined in reaperStateSection with reaperStateKey
 */
void setReaperToggleState(action_entry& act, bool state, std::string reaperStateSection, std::string reaperStateKey);

/**
 * Action parent interface that has reaper's action name and id
 */
class Action
{
public:
	Action(std::string name);

protected:
	std::string name;
	std::string id;
	static std::string namePrefix;
	static std::string idPrefix;
};

/**
 * Simple action which only toggles internal state (wihout actually doing anything).
 * It can be used to light buttons on control surface if no reaper state is associated with it
 * (e.g., when navigating through zones). The action should start with 'surface_toggle_' name and end with '_toggle'
 */
class Toggler : public Action
{
public:
	static std::vector<Toggler> togglers;
	
	static void registerActions(std::string name);
	static void registerClearAllTogglersAction();
	
	Toggler(std::string name);
	
	/* change state of the toggle action */
	inline void setState(action_entry& act, bool state) const
	{
		setReaperToggleState(act, state, this->reaperStateSection, this->reaperStateKey);
	}
	inline void toggleState(action_entry& act) const
	{
		act.m_togglestate == ToggleOff ? this->setState(act, true) : this->setState(act, false);
	}
	inline void setStateOff(action_entry& act) const
	{
		this->setState(act, false);
	}
	inline void setStateOn(action_entry& act) const
	{
		this->setState(act, true);
	}
	
protected:
	/**
	 * Action names for explicitly setting toggle on or off.
	 * Appends to action name _on/_off, respectively.
	 */
	std::string actionSetOnName;
	std::string actionSetOnId;
	std::string actionSetOffName;
	std::string actionSetOffId;
	
	/* section and key under which state is stored (in reaper) */
	std::string reaperStateKey;
	static std::string reaperStateSection;
	static std::string nameSuffix;
	static std::string idSuffix;
};
}
