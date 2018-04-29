case $1 in
linux)
  case $2 in
  conf)
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
    sudo apt-get update -qq
    sudo apt-get install $3 -yq
    ;;
  build)
    $3 -o bin/unit_test test/unit_test/*.cpp -std=c++17 -mcx16 -latomic -Werror -pedantic -pedantic-errors -Wall -Wextra
    ;;
  esac
  ;;
osx)
  case $2 in
  conf)
    export COLUMNS=80
    curl -LO https://raw.githubusercontent.com/GiovanniBussi/macports-ci/master/macports-ci
    chmod +x ./macports-ci
    ./macports-ci install
    PATH="/opt/local/bin:$PATH"
    sudo port install $3
    ;;
  build)
    $3 -o bin/unit_test test/unit_test/*.cpp -std=c++17 -mcx16 -Werror -pedantic -pedantic-errors -Wall -Wextra
    ;;
  esac
  ;;
esac
