#!/bin/bash
# Flare Shell Script
# Written by Ethan "flibitijibibo" Lee

# Move to script's directory
cd "`dirname "$0"`"

# Get the kernel/architecture information
UNAME=`uname`
ARCH=`uname -m`

# Set the libpath and pick the proper binary
if [ "$UNAME" == "Darwin" ]; then
    export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:./lib/
    ./flare
elif [ "$UNAME" == "Linux" ]; then
    if [ "$ARCH" == "x86_64" ]; then
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./x86_64/
        ./x86_64/flare.x86_64 $@
    #else
    #    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./x86/
    #    ./x86/flare.x86 $@
    fi
fi
