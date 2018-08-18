#!/bin/bash

if [ -e /var/run/imud.pid ]; then
  sudo kill `cat /var/run/imud.pid`
else
  echo 'imud not running.'
  exit;
fi
  
if [ -e /var/run/imud.pid ]; then
  sudo rm /var/run/imud.pid
fi
