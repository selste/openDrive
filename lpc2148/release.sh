#!/bin/sh

#
#  $Id: release.sh 366 2009-01-12 22:07:12Z jcw $
#  $Revision: 366 $
#  $Author: jcw $
#  $Date: 2009-01-12 17:07:12 -0500 (Mon, 12 Jan 2009) $
#  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/release.sh $
#

if [ ! -f main.h ]; then
  echo "*"
  echo "* Are we in the root directory of the project?"
  echo "*"
  exit
fi

VERSION=`grep -E "define __VERSION" main.h | cut -f 3,3 -d " " | sed -e "s/\"//g" -e "s/[\r\n]//g"`

if [ ${#VERSION} != 4 ]; then
  echo "*"
  echo "* __VERSION must be 4 charaters, in x.yy format (1.44, for example)"
  echo "*"
  exit
fi

NODOTS_VERSION=`echo ${VERSION} | sed -e "s/\.//g"`

#
#  First, make sure everything is checked into Subversion
#
svn -q status | grep -i "^M" >/dev/null ; retval=$?

if [ $retval -eq 0 ] ; then
  echo "*"
  echo "*  Eeek!  There are files not yet committed to Subversion. Fix with 'svn commit'"
  echo "*"
  exit
fi

#
#  Now make sure there's a Subversion tag copy.  If it's not copied, it shouldn't be released
#
svn ls http://tinymicros.com/svn_public/arm/lpc2148_demo/tags | grep -i "${VERSION}" >/dev/null ; retval=$?

if [ $retval -ne 0 ] ; then
  echo "*"
  echo "*  Eeek!  It appears the Subversion tree isn't tagged yet.  "
  echo "*  Fix with 'svn cp http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk http://tinymicros.com/svn_public/arm/lpc2148_demo/tags/${VERSION} -m \"Release ${VERSION}\"'"
  echo "*"
  exit
fi

#
#  Clean & rebuild the project
#
make clean
make

#
#  Move up one directory
#
cd ..

#
#  Delete any old source
#
rm -rf lpc2148_demo
svn co -q http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk lpc2148_demo

#
#  Copy the built hex file
#
cp trunk/lpc2148.hex lpc2148_demo

#
#  Remove any old copies of this version so we don't append into them
#
rm -f temp/LPC2148_Demo_v${NODOTS_VERSION}.tgz temp/LPC2148_Demo_v${NODOTS_VERSION}.zip

#
#  Build the exclude list so we don't get the .svn directories
#
find lpc2148_demo -name .svn -exec echo {}/* \; > /tmp/exclude.txt

#
#  Build the .tgz and .zip files, copy the README file in
#
tar -X /tmp/exclude.txt -zcf temp/LPC2148_Demo_v${NODOTS_VERSION}.tgz lpc2148_demo
zip -q9r temp/LPC2148_Demo_v${NODOTS_VERSION}.zip lpc2148_demo -x@/tmp/exclude.txt
cp trunk/README temp/xREADME_v${NODOTS_VERSION}

#
#  Cleanup
#
#rm -rf lpc2148_demo /tmp/exclude.txt
