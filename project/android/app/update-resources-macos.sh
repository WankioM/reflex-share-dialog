#!/bin/sh
if [ -e "$2" ]
then
	MOUNT_POINT=$(hdiutil attach "$1" | grep "/Volumes/" | awk '{print $3}')
	"$MOUNT_POINT/ReflexResourceBuilder.app/Contents/MacOS/ReflexResourceBuilder" "$2"
	hdiutil detach "$MOUNT_POINT"
else
	>&2 echo "You need to pass the path of a resources.xml file (does not exist: $2)"
fi
