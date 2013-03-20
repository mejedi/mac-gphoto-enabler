mac-gphoto-enabler
==================

Mac OS X comes with its own framework for managing digital cameras.
This native framework must be disabled in order for gphoto to claim
and drive a camera.

We provide a script to do this in a reversible way on a per user basis.

Additionally we provide a daemon program that may be used by an application
built on top of gphoto to subscribe to camera discovered/camera gone
notifications.  While there is an open connection between the deamon
and an app, Mac OS X native digital camera framework is disabled automatically. **[NOT READY YET]**

### How does it work?

We patch `/Library/Caches/com.apple.ImageCaptureNotifications.DeviceDiscoveryDatabase.${USERID}`.

This sqlite database contains the rules describing devices that are known to
image capture frawework. When a known device is attached the framework launches
the apropriate backend (ex: PTPCamera).

Each user has his own copy of DeviceDiscoveryDatabase. If the database is discarded
MacOS X rebuilds it from `/System/Library/Image Capture/Devices/*.app/Contents/Resources/DeviceMatchingInfo.plist`
files.
