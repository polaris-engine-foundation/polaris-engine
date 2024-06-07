#!/bin/sh

SAVE_DIR=`pwd`
PROJECT_DIR=`dirname $0`
cd "$PROJECT_DIR"

xcodebuild archive -archivePath app.xcarchive -scheme engine-ios -allowProvisioningUpdates
xcodebuild -exportArchive -archivePath app.xcarchive -exportOptionsPlist ExportOptions.plist -exportPath app  -allowProvisioningUpdates

cd "$SAVE_DIR"
open "$PROJECT_DIR/app"

exit
