case $TRAVIS_OS_NAME in
linux)
  BUILD=$CXX-$VER
  EXTRA=-latomic
  COV=gcov-$VER
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
  sudo apt-get update -qq
  sudo apt-get install $CXX-$VER -yq
  ;;
osx)
  BUILD=$CXX-mp-$VER
  EXTRA=
  COV="llvm-cov-mp-$VER gcov"
  export COLUMNS=80
  curl -LO https://raw.githubusercontent.com/GiovanniBussi/macports-ci/master/macports-ci
  chmod +x ./macports-ci
  ./macports-ci install
  PATH="/opt/local/bin:$PATH"
  sudo port install $CC-$VER
  ;;
esac
UNIT_TEST=$(pwd)/test/unit_test
mkdir bin
cd bin
