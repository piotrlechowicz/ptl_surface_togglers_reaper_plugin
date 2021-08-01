#include "utilfuncs.h"
#include "reaper_action_helper.h"
#include "reaper_plugin/reaper_plugin_functions.h"

#include <iostream>
#include <fstream>
#include <filesystem>

#include "surface_togglers.h"
#include "surface_togglers_utils.h"

namespace surface_togglers
{
//---------------------------------------
// parsing config file
//---------------------------------------

std::vector<std::string> ConfigParser::getTogglerNames()
{
	std::filesystem::path resourceDir (GetResourcePath());
	std::filesystem::path configPath = resourceDir / configPathSuffix;
	
	std::ifstream file;
	file.open(configPath.c_str());

	std::vector<std::string> togglerNames;
	
	if (file.is_open())
	{
		std::string line;
		while(getline(file, line))
		{
			if (!std::all_of(line.begin(), line.end(), isspace))
			{
				togglerNames.push_back(line);
			}
		}
		file.close();
	} else {
		ShowConsoleMsg(("PTL_SurfTog: cannot open config file. Check if \"{Reaper resource path}/" + configPathSuffix.string() + "\" file is provided.\n").c_str());
	}
	return togglerNames;
}

//---------------------------------------
// Actions
//---------------------------------------

void setReaperToggleState(action_entry& act, bool state, std::string reaperStateSection, std::string reaperStateKey)
{
	if (state == true)
	{
		act.m_togglestate = ToggleOn;
	}
	else
	{
		act.m_togglestate = ToggleOff;
	}
	char buf[8];
	// the reaper api for ini-files only deals with strings, so form a string from the action toggle state number;
	int toggletemp = 0;
	if (act.m_togglestate == ToggleOn)
	{
		toggletemp = 1;
	}
	sprintf(buf, "%d", toggletemp);
	SetExtState(reaperStateSection.c_str(), reaperStateKey.c_str(), buf, true);
}

std::string Action::namePrefix = "Surface Toggler: ";
std::string Action::idPrefix = "surface_toggler_";

Action::Action(std::string name)
{
	this->id = name;
	this->name = this->namePrefix + name;
	std::replace(this->id.begin(), this->id.end(), ' ', '_');
	this->id = this->idPrefix + this->id;
}

Action::Action(std::string name, std::string id) : name(name), id(id)
{
	// intentionally left blank
}

std::vector<Toggler> Toggler::togglers {};
std::string Toggler::nameSuffix = " toggle";
std::string Toggler::idSuffix = "_toggle";
std::string Toggler::reaperStateSection = "PTL_surface_toggler";

void Toggler::registerActions(std::string name)
{
	Toggler toggler(name);
	
	togglers.push_back(toggler);
	
	add_action(toggler.name, toggler.id, ToggleOff, [=](action_entry& act) {
		toggler.toggleState(act);
	});
	add_action(toggler.actionSetOnName, toggler.actionSetOnId, CannotToggle, [=](action_entry& act) {
		// toggle action with this name is already defined, thus we should fetch it and toggle its state
		// not the passed action that is associated with the on/off command
		action_entry* actOverride = get_action_by_id(toggler.id);
		toggler.setStateOn(*actOverride);
	});
	add_action(toggler.actionSetOffName, toggler.actionSetOffId, CannotToggle, [=](action_entry& act) {
		action_entry* actOverride = get_action_by_id(toggler.id);
		toggler.setStateOff(*actOverride);
	});
}

Toggler::Toggler(std::string name)
: Action(name)
{
	this->name = this->name + this->nameSuffix;
	this->id = this->id + this->idSuffix;
	this->reaperStateKey = this->id;
	
	this->actionSetOnName = eraseSubString(this->name, " toggle", namePrefix.size()) + " on";
	this->actionSetOnId = eraseSubString(this->id, "_toggle", idPrefix.size()) + "_on";
	this->actionSetOffName = eraseSubString(this->name, " toggle", namePrefix.size()) + " off";
	this->actionSetOffId = eraseSubString(this->id, "_toggle", idPrefix.size()) + "_off";
}

void Toggler::registerClearAllTogglersAction()
{
	add_action(Toggler::namePrefix + "clear all toggles", Toggler::idPrefix + "clear_all_toggles", CannotToggle, [=](action_entry& act) {
		for(auto& toggler : Toggler::togglers)
		{
			action_entry* act = get_action_by_id(toggler.id);
			toggler.setStateOff(*act);
		}
	});
}

}
