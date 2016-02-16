#!/bin/sh

# Modules with dependency files.
modules="allocore allonect alloutil examples/mysql alloaudio"

# Override if module are passed in as arguments.
if [ "$#" -gt 0 ]; then
  modules="$*"
fi


ROOT=`pwd`
PLATFORM=`uname`
ARCH=`uname -m`
echo "Installing for ${PLATFORM} ${ARCH} from ${ROOT}"


# Callbacks.
binary_exists(){
  command -v "$1" >/dev/null 2>&1;
}

files_exist(){
  ls -u "$@" >/dev/null 2>&1;
}

# Prepend redirection to apply to entire piped command.
is_callback(){
  type "$1" 2>/dev/null | grep -i 'function' >/dev/null 2>&1;
}

# package_manager: used to pick correct dependency lists and callbacks.

if binary_exists 'apt-get'; then
  package_manager='apt'
  sudo apt-get update
  installer="sudo apt-get install"

# Homebrew is the preferred Mac package manager of the AlloTeam.
elif binary_exists 'brew'; then
  package_manager='brew'
  brew update
  installer="brew install"
elif binary_exists 'port'; then
  package_manager='port'
  sudo port selfupdate
  installer="sudo port install"

# Only MSYS2 is supported on Windows due to the presence of a package manager.
elif uname | grep 'MINGW' > /dev/null 2>&1; then
  if binary_exists 'pacman'; then
    package_manager='pacman'
    arch="$(uname -m)"
    pacman -Syy
    installer='pacman -S'
  fi
else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install Apt, Homebrew, MacPorts, or MSYS2 and try again.'
fi


# Path to this script.
appended_relative=$(dirname "$0" ; echo x)
sanitized_relative=${appended_relative%??}
appended_absolute="$( cd "$sanitized_relative" && pwd; echo x)"
sanitized_absolute=${appended_absolute%??}

for module in $modules; do
  # Source the variables and callbacks of each module.
  eval ". $sanitized_absolute/${module}/dependencies"

  # Workaround for embedded scripts like examples/mysql.
  module="$(basename "$module")"

  # Append dependencies of each module to install_list.
  module_packages="${module}_${package_manager}"
  eval "install_list=\"${install_list} \$${module_packages}\""

  # Run callback if it exists.
  if is_callback "${module_packages}_callback"; then
    eval "${module_packages}_callback"
  fi
done

# Install contatenated dependency list.
exec $installer $install_list
