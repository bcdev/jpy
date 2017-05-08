#!/bin/bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

    # Install some custom requirements on OS X
    # e.g. brew install pyenv-virtualenv
    # See https://gist.github.com/Bouke/11261620

    brew update
    brew install pyenv
    brew install pyenv-virtualenv
    eval "$(pyenv init -)"
    eval "$(pyenv virtualenv-init -)"

    if [[ $TOXENV == '2.7' ]]; then
        pyenv virtualenv 2.7 jpy-venv
    elif [[ $TOXENV == '3.4' ]]; then
        pyenv virtualenv 3.4 jpy-venv
    elif [[ $TOXENV == '3.5' ]]; then
        pyenv virtualenv 3.5 jpy-venv
    elif [[ $TOXENV == '3.6' ]]; then
        pyenv virtualenv 3.6 jpy-venv
    fi

else
    # Install pyenv
    # See https://github.com/pyenv/pyenv
    git clone https://github.com/pyenv/pyenv.git $HOME/.pyenv
    export PYENV_ROOT="$HOME/.pyenv"
    export PATH="$PYENV_ROOT/bin:$PATH"
    eval "$(pyenv init -)"

    # Install pyenv virtualenv plugin
    # See https://github.com/pyenv/pyenv-virtualenv
    git clone https://github.com/pyenv/pyenv-virtualenv.git $(pyenv root)/plugins/pyenv-virtualenv
    eval "$(pyenv virtualenv-init -)"

    # Create virtualenv from current Python
    pyenv virtualenv jpy-venv
fi

pyenv activate jpy-venv
pip install wheel



