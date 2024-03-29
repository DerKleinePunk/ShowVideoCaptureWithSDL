#!/bin/bash
# wget -O build.sh https://raw.githubusercontent.com/DerKleinePunk/ShowVideoCaptureWithSDL/master/build.sh
# chmod +x build.sh

source ./logger.sh
SCRIPTENTRY

echo try to download and build VideoCamSDL2
INFO "try to download and build VideoCamSDL2"
reproBuild="false"
if [ -d .git ]; then
	echo "called inside repro"
	reproBuild="true"
fi

echo "reproBuild $reproBuild"
INFO "reproBuild $reproBuild"

if [ "$reproBuild" = "false" ] ; then
	wget -O DebianPackages.txt https://raw.githubusercontent.com/DerKleinePunk/ShowVideoCaptureWithSDL/master/DebianPackages.txt
	exitCode=$?
	if [ $exitCode -ne 0 ] ; then
		echo "wget give an error"
		exit $exitCode
	fi
else
	echo "Don't forget git pull bevor building"
	INFO "Don't forget git pull bevor building"
fi

InstallPackage(){
	packageName="$1"
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $packageName|grep "install ok installed")
	DEBUG "Checking for $packageName: $PKG_OK"
	if [ "" = "$PKG_OK" ]; then
		echo "No $packageName. Setting up $packageName."
		DEBUG "No $packageName. Setting up $packageName."
		sudo apt-get --yes install $packageName
	fi
}

InstallPackage git
InstallPackage git-lfs

DEPENSFILE="DebianPackages.txt"

while read LINE; do
     InstallPackage $LINE
done < $DEPENSFILE

sudo apt-get --yes update
sudo apt-get --yes upgrade
sudo apt-get autoremove -y

rpiversion=$(cat /sys/firmware/devicetree/base/compatible |cut -c1-13)
echo "PI $rpiversion"
DEBUG "PI $rpiversion"

InstallSDLComponent(){
	packageName="$1"
	packageVersion="$2"
	if [ ! -d "$packageVersion" ]; then
		wget -N https://www.libsdl.org/projects/$packageName/release/$packageVersion.tar.gz
		exitCode=$?
		if [ $exitCode -ne 0 ] ; then
		   echo "wget give an Error"
		   exit $exitCode
		fi
		tar -xzf $packageVersion.tar.gz
		exitCode=$?
		if [ $exitCode -ne 0 ] ; then
		   echo "tar give an Error"
		   exit $exitCode
		fi
		cd $packageVersion
		./autogen.sh
		./configure
		make -j$(nproc)
		sudo make install
		cd ..
	else
		cd $packageVersion
		./autogen.sh
		./configure
		make -j$(nproc)
		sudo make install
		cd ..
	fi
}

if [ "$reproBuild" = "true" ] ; then
  cd ..
fi

echo "we are here"
pwd

DIRECTORY="SDL2-2.26.4"
if [ ! -d "$DIRECTORY" ]; then
	wget -N https://www.libsdl.org/release/$DIRECTORY.tar.gz
	exitCode=$?
	if [ $exitCode -ne 0 ] ; then
	   echo "wget give an Error"
	   ERROR "wget give an Error"
	   exit $exitCode
	fi
	tar -xzf $DIRECTORY.tar.gz
	exitCode=$?
	if [ $exitCode -ne 0 ] ; then
	   echo "tar give an Error"
	   ERROR "wget give an Error"
	   exit $exitCode
	fi
fi

cd $DIRECTORY
./autogen.sh
if [ "$rpiversion" == "raspberrypi,4" ] ; then
	./configure --enable-video-kmsdrm --disable-esd --disable-video-wayland --disable-video-opengl --disable-video-rpi --disable-kmsdrm-shared --enable-arm-neon
elif [ "$rpiversion" == "raspberrypi,3" ] ; then
	./configure --disable-esd --disable-video-wayland --disable-video-opengl --enable-arm-neon
else
	./configure
fi
exitCode=$?
if [ $exitCode -ne 0 ] ; then
   echo "configure give an Error"
   exit $exitCode
fi
make -j$(nproc)
sudo make install
cd ..

InstallSDLComponent SDL_image SDL2_image-2.0.5
InstallSDLComponent SDL_mixer SDL2_mixer-2.0.4
InstallSDLComponent SDL_net SDL2_net-2.0.1
InstallSDLComponent SDL_ttf SDL2_ttf-2.0.18

echo building VideoCamSDL2
echo "we are here"
pwd

DIRECTORY="ShowVideoCaptureWithSDL"
if [ ! -d "$DIRECTORY" ]; then
	git clone https://github.com/DerKleinePunk/ShowVideoCaptureWithSDL.git
	exitCode=$?
	if [ $exitCode -ne 0 ] ; then
	   echo "git give an Error"
	   exit $exitCode
	fi
	cd $DIRECTORY
else
	cd $DIRECTORY
	if [ "$reproBuild" = "false" ] ; then
		git pull
		exitCode=$?
		if [ $exitCode -ne 0 ] ; then
			echo "git give an Error"
			exit $exitCode
		fi
	fi
fi

DIRECTORY="buildrelease"
if [ ! -d "$DIRECTORY" ]; then
	mkdir $DIRECTORY
fi
cd $DIRECTORY

cmake .. -DCMAKE_BUILD_TYPE=Release -DTARGET=Linux -DENABLE_CPPCHECK=OFF
exitCode=$?
if [ $exitCode -ne 0 ] ; then
	echo "cmake give an Error"
	exit $exitCode
fi

cmake --build . -j $(nproc)
exitCode=$?
if [ $exitCode -ne 0 ] ; then
	echo "cmake give an Error"
	exit $exitCode
fi
cd ..

echo to run call ./bin/Linux/VideoCamSDL2
