case $TRAVIS_OS_NAME in
linux)
  BUILD=$CXX-$VER
  COV=gcov-$VER
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
  sudo apt-get update -qq
  sudo apt-get install $CXX-$VER -yq
  ;;
osx)
  BUILD=$CXX-mp-$VER
  COV="llvm-cov-mp-$VER gcov"
  export COLUMNS=80
  curl -LO https://raw.githubusercontent.com/GiovanniBussi/macports-ci/master/macports-ci
  chmod +x ./macports-ci
  ./macports-ci install
  PATH="/opt/local/bin:$PATH"
  sudo port install $CC-$VER
  ;;
esac

ROOT=$(pwd)
UNIT_TEST=$ROOT/test/unit_test
PERF_TEST=$ROOT/test/perf_test

curl -LO https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz
tar --gzip -xf boost*
cp -R boost*/boost $PERF_TEST/boost

ESSENTIAL="-std=c++17 -Werror -pedantic -pedantic-errors -Wall -Wextra"
DEBUG="$ESSENTIAL -O0 -g3 -coverage"
PERF="$ESSENTIAL -I$ROOT -I$PERF_TEST -O3"

mkdir bin
cd bin
