#!/bin/bash
# Copyright (C) 2013 by Pedro Mendes, Virginia Tech Intellectual 
# Properties, Inc., University of Heidelberg, and The University 
# of Manchester. 
# All rights reserved. 


PATH=$PATH:/bin:/usr/bin:/usr/local/bin

SCP=${COPASI_SCP:-scp}
AWK=${COPASI_AWK:-gawk}
SORT=${COPASI_SORT:-sort}
PACKAGE=${COPASI_PACKAGE:-Windows}

COMMON_ENVIRONMENT=${COPASI_COMMON_ENVIRONMENT:-"/home/shoops/environment"}
SOURCE=${COPASI_SOURCE:-"${COMMON_ENVIRONMENT}/COPASI"}
BUILD_32=${COPASI_BUILD_32:-"${COMMON_ENVIRONMENT}/win32-icc-32/package-32"}
BUILD_64=${COPASI_BUILD_32:-"${COMMON_ENVIRONMENT}/win32-icc-64/package-64"}
SETUP_DIR=${COPASI_SETUP_DIR:-"${COMMON_ENVIRONMENT}/setup"}

INNO_SETUP=${COPASI_INNO_SETUP:-"/cygdrive/c/Program Files (x86)/Inno Setup 5/ISCC.exe"}
INNO_FILE=${COPASI_INNO_FILE:-"${SOURCE}/InnoSetup/copasi-universal.iss"}

major=`${AWK} -- '$2 ~ "COPASI_VERSION_MAJOR" {print $3}' "${SOURCE}/copasi/CopasiVersion.h"`
minor=`${AWK} -- '$2 ~ "COPASI_VERSION_MINOR" {print $3}' "${SOURCE}/copasi/CopasiVersion.h"`
build=`${AWK} -- '$2 ~ "COPASI_VERSION_BUILD" {print $3}' "${SOURCE}/copasi/CopasiVersion.h"`
modified=`${AWK} -- '$2 ~ "COPASI_VERSION_MODIFIED" {print $3}' "${SOURCE}/copasi/CopasiVersion.h"`
comment=`${AWK} -- '$2 ~ "COPASI_VERSION_COMMENT" {print $3}' "${SOURCE}/copasi/CopasiVersion.h"`
buildname=${build}

if [ $modified == true ]; then
  buildname=${buildname}+
fi

MyAppVersion=${major}.${minor}.${build}

if [ x"${comment}" = x\"Snapshot\" ]; then
  MyAppVersion=${major}.
  [ ${#minor} = 1 ] && MyAppVersion=${MyAppVersion}0
  MyAppVersion=${MyAppVersion}${minor}.
  [ ${#build} = 1 ] && MyAppVersion=${MyAppVersion}0
  MyAppVersion=${MyAppVersion}${build}
fi

# Create the unique product code based on version and application name
GUID=`md5sum << EOF
#define MyAppName "COPASI"
#define MyAppVersion "${MyAppVersion}"
#define MyAppPublisher "copasi.org"
#define MyAppURL "http://www.copasi.org/"
#define MyAppExeName "bin\CopasiUI.exe"
EOF` 
GUID=`echo $GUID | sed 'y/abcdef/ABCDEF/'`
productcode=${GUID:0:8}-${GUID:8:4}-${GUID:12:4}-${GUID:16:4}-${GUID:20:12}

[ -e ${SETUP_DIR}/package ] && rm -rf ${SETUP_DIR}/package
mkdir ${SETUP_DIR}/package
pushd ${SETUP_DIR}/package

# Create directory structure
cp -r "${SETUP_DIR}/src/"* .

# Copy README
cp ${SOURCE}/README.Win32 README.txt
chmod 644 README.txt

# Copy license
cp ${SOURCE}/copasi/ArtisticLicense.txt LICENSE.txt
chmod 644 LICENSE.txt

# Copy configuration resources    
cp ${SOURCE}/copasi/MIRIAM/MIRIAMResources.xml share/copasi/config
chmod 444 share/copasi/config/*

# Copy examples
cp ${SOURCE}/TestSuite/distribution/* share/copasi/examples
chmod 444 share/copasi/examples/*
chmod 777 share/copasi/examples

# Copy icons
cp ${SOURCE}/copasi/CopasiUI/icons/Copasi.ico share/copasi/icons
cp ${SOURCE}/copasi/CopasiUI/icons/CopasiDoc.ico share/copasi/icons
chmod 644 share/copasi/icons/*

# Copy wizard resource
cp ${SOURCE}/copasi/wizard/help_html/*.html share/copasi/doc/html
chmod 644 share/copasi/doc/html/*.html

cp ${SOURCE}/copasi/wizard/help_html/figures/*.png \
    share/copasi/doc/html/figures
chmod 644 share/copasi/doc/html/figures/*.png

# 32 bit files
cp "${BUILD_32}/copasi/CopasiUI/CopasiUI.exe"  bin/32
chmod 755 bin/32/CopasiUI.exe
cp "${BUILD_32}/copasi/CopasiSE/CopasiSE.exe"  bin/32
chmod 755 bin/32/CopasiSE.exe

# 32 bit files
cp "${BUILD_64}/copasi/CopasiUI/CopasiUI.exe"  bin/64
chmod 755 bin/64/CopasiUI.exe
cp "${BUILD_64}/copasi/CopasiSE/CopasiSE.exe"  bin/64
chmod 755 bin/64/CopasiSE.exe


# Execute InnoSetup to create Installation package
cd ${SOURCE}/InnoSetup

workdir=`cygpath -wa .`
workdir=${workdir//\\/\\\\}

stagedir=`cygpath -wa "${SETUP_DIR}/package"`
stagedir=${stagedir//\\/\\\\}

#   modify product code, product version, and package name
sed -e '/#define MyAppVersion/s/".*"/"'${MyAppVersion}'"/' \
    -e '/#define MyBuild/s/".*"/"'${buildname}'"/' \
    -e '/#define MyAppId/s/".*"/"{{'${productcode}'}"/' \
    -e '/#define MyWorkDir/s/".*"/"'${workdir}'"/' \
    -e '/#define MyWorkDir/s/".*"/"'${workdir}'"/' \
    -e '/#define MyStageDir/s/".*"/"'${stagedir}'"/' \
    ${INNO_FILE} > tmp.iss

# Run Inno Setup to create package
"${INNO_SETUP}" tmp.iss && rm tmp.iss

# Move the package to its final location
cp COPASI-${buildname}-${PACKAGE}.exe "${SETUP_DIR}/package"
chmod 755 "${SETUP_DIR}"/package/COPASI-${buildname}-${PACKAGE}.exe

popd
