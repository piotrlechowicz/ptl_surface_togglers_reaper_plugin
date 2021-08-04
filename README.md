# Surface Togglers Reaper plugin

Plugin that creates dummy toggle actions with user specified names that can be mapped to surface control buttons in CSI plugin.

Developed for REAPER v6.32-beta version.

## Installation

Copy `releases/ptl_surface_togglers_reaper_plugin.dylib` to `UserPlugins` in Reaper resource path.
Copy `PTL/` folder to Reaper resource path.

## Description

Buttons are lit based on the state of the action (on/off) in Reaper to which they are assigned. However, if button is assigned to action without state/it is used to change CSI zone, its light does not reflect that. To reflect changes in surface, dummy toggles can be used provided by this plugin.
 
Name of toggles are specified in `PTL/SurfTog/config.txt` file, each name in new line. For each `<name>` in the file, the plugin creates 3 actions:

* `_ptl_surface_toggler_<name>_toggle` - action that only has on/off state without doing anything
* `_ptl_surface_toggler_<name>_on` - this action turns the toggle on
* `_ptl_surface_toggler_<name>_off` - this action turns the toggle off


Let us consider `zoom` button that should change surface buttons (e.g. arrows) between scroll mode and zoom mode. In particular, if zoom is unlit, the scroll mode is enabled, while if it is lit, the zoom mode is active.

In CSI zone, we can assign created toggle action to the `zoom` button

```
Zone "ViewLayerScrollMode"
    /====== Navigation
    Zoom Reaper "_ptl_surface_toggler_zoom_toggle"  // assign toggle to reflect state changes on surface with the button light
    Zoom Reaper "_ptl_surface_toggler_zoom_on"      // turn the light on
    Zoom GoZone "ViewLayerZoomMode"                 // go to zone that specifies zoom actions
    /====== Implementation
    /...
ZoneEnd
```

If we want to go back to scroll mode we can reverse the procedure

```
Zone "ViewLayerZoomMode"
    /====== Navigation
    Zoom Reaper "_ptl_surface_toggler_zoom_toggle"  // assign toggle to reflect state changes on surface with the button light
    Zoom Reaper "_ptl_surface_toggler_zoom_off"     // turn the light off
    Zoom GoZone "ViewLayerScrollMode"               // go to zone that specifies scroll actions
    /====== Implementation
    /...
```

In any moment, if we want to turn off all created dummy actions, an action `_ptl_surface_toggler_clear_all_toggles` is specified. Which can be useful to combine with `GoZone Home` directive to reset surface light to initial state.

In particular, these toggles can be useful when navigating between various fader modes (e.g., normal, sends, receives, plugins) and we have defined corresponding buttons on the surface for that. Consider the following buttons `pan`, `sends`, `receives`, `inserts` and the following implementation which turns light on of the button that is pressed and turns the light off of other buttons.

```
Zone "LayerSelection"
    Pan Reaper "_ptl_surface_toggler_pan_toggle"
    Pan Reaper "_ptl_surface_toggler_pan_on"		
    Pan Reaper "_ptl_surface_toggler_sends_off"
    Pan Reaper "_ptl_surface_toggler_receives_off"
    Pan Reaper "_ptl_surface_toggler_inserts_off"

    Sends Reaper "_ptl_surface_toggler_sends_toggle"
	Sends Reaper "_ptl_surface_toggler_pan_off"
	Sends Reaper "_ptl_surface_toggler_sends_on"
	Sends Reaper "_ptl_surface_toggler_receives_off"
	Sends Reaper "_ptl_surface_toggler_inserts_off"

    Receives Reaper "_ptl_surface_toggler_receives_toggle"
	Receives Reaper "_ptl_surface_toggler_pan_off"
	Receives Reaper "_ptl_surface_toggler_sends_off"
	Receives Reaper "_ptl_surface_toggler_receives_on"
	Receives Reaper "_ptl_surface_toggler_inserts_off"

    Inserts Reaper "_ptl_surface_toggler_inserts_toggle"
	Inserts Reaper "_ptl_surface_toggler_pan_off"
	Inserts Reaper "_ptl_surface_toggler_sends_off"
	Inserts Reaper "_ptl_surface_toggler_receives_off"
	Inserts Reaper "_ptl_surface_toggler_inserts_on"
ZoneEnd
```

## Specify toggle names

Toggles can be specified in `PTL/SurfTog/config.txt` file, where each toggle name should be put in separate lines.

There is a special toggle `ClearAll` that unlit all toggles.

## Credits

The plugin is created based on the MyReaperPlugin
https://github.com/elanhickler/MyReaperPlugin
https://forum.cockos.com/archive/index.php/t-169032.html

Code added by me is placed in `PTL_code/source` and `PTL_code/header` folders.
