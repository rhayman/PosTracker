## Synopsis

A plugin for the openephys plugin-GUI electrophysiology software that tracks the largest
bright spot captured from a webcam and saves the x,y and time of capture.

## Motivation

The openephys plugin-GUI has no built-in position tracking capability which is needed by
those studying spatial behaviour / neurophysiology

## Installation

Prerequisites include the following:

* opencv (tested with v3.1.0)

Download the files and extract to a folder and make sure it's in the plugin folder of
your openephys installation and then re-compile with

```
make -f Makefile.plugins
```

Once installed the PosTracker plugin should appear as a Source in openephys

## Usage

It should be reasonably self-evident what the controls and so on do, but briefly:

Drag the PosTracker down to the Signal Chain bar at the bottom

The Devices drop down menu should show a list of available devices ('/dev/video0' etc)

Select the one you want to use and then the Resolution drop down will fill out with the
available resolutions / framerates. Select the one you want; I go for one about
800 x 600 @ 30FPS

The Brightness, Contrast and Exposure slider controls are self-explanatory except that
in order to use the Exposure one you need to de-select (no tick mark) the Auto-exposure
check-box. You'll want to have a low-ish Exposure value if you want a high as possible
frame-rate.

Click the Show Video check-box and (hopefully) a window will appear with the image from
the webcam. You can adjust the Brightness etc during live playback, and also adjust a
bounding box that surrounds the captured video so that the tracking is only done within
that bounding box. The bounding box controls are the sliders labelled left-right and
top-bottom

At the bottom right is a check-box called Path Overlay. Selecting this will superimpose
a green line that shows you the cumulative tracked position of the LED - de-selecting
will clear / remove the green line.

The tracked x,y position is also displayed to the left of the Brightness control as well
as the FPS (frames per second). If you've asked for a resolution that has 30 FPS then
usually you'll want to actually be capturing at around this frequency; the parameter that
affects this the most is Exposure - it determines how long the camera 'exposes'
its CCD to capture light; lower values mean this duration is shorter and so the FPS can go
up.

## Notes

The frame-rate is actually variable and not a fixed thing - if you look at the data
(see below) you'll see that even though you've asked for 30FPS it might be 30.56 or
something. It's important to note that there are three variables that are saved; x and y
position and the timestamp *that the first byte of data was captured* - this data is
from the camera and so is different to the openephys timestamp i.e. it serves as an
offset. You should subtract this from timestamp openephys says is the timestamp for the
binary event. So far I've only tested the PosTracker with the .nwb format so if the
path to the position data is as follows in the nwb file:

```
mydata['acquisition']['timeseries']['recording_name']['events']['binary1']
```

Then this part of the nwb file has fields called

```
'data'
```

and

```
'timestamps'
```

'data' is a n_samples x 3 array. If there are n_samples rows, the the first two
columns are the x and y position and the last column is the timestamp the first
byte was captured in camera time. The 'timestamps' field is when OE captured the
binary data so to get the 'real' timestamp subtract the third column of 'data'
(divided by 1e6; PosTracker timestamps are in ms, OE's in seconds) from the 'timestamps' field.