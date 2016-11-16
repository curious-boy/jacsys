#!/bin/bash

# start many server 

nohup ./jacServer 2008 1>/dev/null 2>/dev/null &
nohup ./jacServer 2009 1>/dev/null 2>/dev/null &
nohup ./jacServer 2010 1>/dev/null 2>/dev/null &
nohup ./jacServer 2011 1>/dev/null 2>/dev/null &
nohup ./jacServer 2012 1>/dev/null 2>/dev/null &

echo "start finished!"
