#!/bin/sh

echo @@@@
env
echo @@@@

export PREFIX="$HOME"/xorg
mkdir -p "$PREFIX"

mkdir -p $HOME/source
pushd $HOME/source

if [ ! -d util-modular ]; then
    git clone git://anongit.freedesktop.org/git/xorg/util/modular util-modular
fi
cd util-modular
git pull
# OSX doesn't have -i
sed 's/uname -i/uname -p/' < build.sh > foo ; mv foo build.sh ; chmod u+x build.sh
ls -l

for i in `cat "$1"` ; do
    pushd $i
    if ! git pull | grep -q "Already up to date." ; then
        popd
        MAKEFLAGS="V=1" ./build.sh -o "$i" --clone -b -c
    else
        popd
    fi
done

# ./build.sh --modfile "$1" --clone -b -c
popd
