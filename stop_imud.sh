#!/bin/bash

if [ -e /var/run/imud.pid ]; then
  sudo kill `cat /var/run/imud.pid`
else
  echo 'imud not running.'
fi
  
